-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- Generic System V ELF structures. Machine-specific payloads keep region labels.
return function(p)
    assert(p:bytes(0, 4) == "\127ELF", "Missing ELF signature")
    local class, encoding = p:uint(4, 1), p:uint(5, 1)
    assert(class == 1 or class == 2, "Unsupported ELF class")
    assert(encoding == 1 or encoding == 2, "Unsupported ELF byte order")
    p.little = encoding == 1
    local wide = class == 2
    local word = if wide then 8 else 4
    local function u16(o) return p:uint(o, 2) end
    local function u32(o) return p:uint(o, 4) end
    local function ux(o) return p:uint(o, word) end
    assert(p:uint(6, 1) == 1 and u32(20) == 1, "Unsupported ELF version")
    p:struct(0, "EI_MAG:4 EI_CLASS:1 EI_DATA:1 EI_VERSION:1 EI_OSABI:1 EI_ABIVERSION:1 EI_PAD:7 e_type:2 e_machine:2 e_version:4 e_entry:" .. word .. " e_phoff:" .. word .. " e_shoff:" .. word .. " e_flags:4 e_ehsize:2 e_phentsize:2 e_phnum:2 e_shentsize:2 e_shnum:2 e_shstrndx:2", "ELF", 1)
    local tail = if wide then 48 else 36
    local headerSize = if wide then 64 else 52
    local ehsize = u16(tail + 4)
    assert(ehsize >= headerSize, "ELF header size is too small")
    p:mark(headerSize, ehsize - headerSize, "ELF.HeaderExtension", 1)
    local phoff, shoff = ux(24 + word), ux(24 + word * 2)
    local phsize, phnum = u16(tail + 6), u16(tail + 8)
    local shsize, shnum, namesIndex = u16(tail + 10), u16(tail + 12), u16(tail + 14)
    local minimumSh = if wide then 64 else 40
    local sections = {}
    local sectionLayout = "sh_name:4 sh_type:4 sh_flags:" .. word .. " sh_addr:" .. word .. " sh_offset:" .. word .. " sh_size:" .. word .. " sh_link:4 sh_info:4 sh_addralign:" .. word .. " sh_entsize:" .. word
    local function section(o)
        return { nameIndex = u32(o), kind = u32(o + 4), flags = u32(o + 8),
            offset = ux(o + 8 + word * 2), size = ux(o + 8 + word * 3),
            link = u32(o + 8 + word * 4), info = u32(o + 12 + word * 4),
            stride = ux(o + 16 + word * 5), alignment = ux(o + 16 + word * 4) }
    end
    if shoff ~= 0 then
        assert(shsize >= minimumSh, "Invalid section header size")
        p:range(shoff, shsize)
        local zero = section(shoff)
        assert(zero.kind == 0, "ELF section zero is not SHT_NULL")
        if shnum == 0 then shnum = zero.size end
        if phnum == 65535 then phnum = zero.info end
        if namesIndex == 65535 then namesIndex = zero.link end
        p:array(shoff, shnum, shsize, minimumSh, function(o, i)
            sections[i] = section(o)
            p:struct(o, sectionLayout, "SectionHeader[" .. i .. "]", 3)
            p:mark(o + minimumSh, shsize - minimumSh, "SectionHeader.Extension", 3)
        end)
    else
        assert(shnum == 0 and namesIndex == 0 and phnum ~= 65535, "Missing extended section numbering")
    end
    assert(namesIndex == 0 or (namesIndex < shnum and sections[namesIndex].kind == 3), "Invalid section name table")
    local function table_string(s, index, label)
        assert(s and s.kind == 3 and index >= 0 and index < s.size, "Invalid string table reference: " .. label)
        return p:cstring(s.offset + index, math.min(s.size - index, 65536), label, 5)
    end
    for i = 1, shnum - 1 do
        local s = sections[i]
        s.name = if namesIndex == 0 then "" else table_string(sections[namesIndex], s.nameIndex, "Section.Name[" .. i .. "]")
        s.label = "Section[" .. i .. "] " .. s.name
        -- SHT_NOBITS occupies memory only, including arbitrarily large .bss.
        if s.kind ~= 8 and s.kind ~= 0 then p:mark(s.offset, s.size, s.label, 4 + i) end
    end
    local notesSeen = {}
    local function notes(offset, size, label, alignment)
        local key = offset .. ":" .. size
        if notesSeen[key] then return end
        notesSeen[key] = true
        local o, ending = offset, offset + size
        p:range(offset, size)
        while o < ending do
            assert(ending - o >= 12, "Truncated ELF note header")
            local namesz, descsz = u32(o), u32(o + 4)
            local nameEnd = o + 12 + namesz
            local desc = math.ceil(nameEnd / alignment) * alignment
            local nextNote = math.ceil((desc + descsz) / alignment) * alignment
            assert(nameEnd <= ending and desc <= ending and descsz <= ending - desc and nextNote <= ending, "Truncated ELF note")
            p:struct(o, "n_namesz:4 n_descsz:4 n_type:4", label .. ".Note", 6)
            p:mark(o + 12, namesz, label .. ".Note.Name", 4)
            p:mark(nameEnd, desc - nameEnd, label .. ".Note.NamePadding", 3)
            p:mark(desc, descsz, label .. ".Note.Descriptor", 5)
            p:mark(desc + descsz, nextNote - desc - descsz, label .. ".Note.Padding", 3)
            o = nextNote
        end
    end
    local dynamicSeen = {}
    local function dynamic(offset, size, stride, label, strings)
        local key = offset .. ":" .. size
        if dynamicSeen[key] then return end
        dynamicSeen[key] = true
        assert(size % stride == 0, "Misaligned dynamic table")
        local terminated = false
        p:array(offset, size / stride, stride, word * 2, function(o, i)
            p:struct(o, "d_tag:" .. word .. " d_val_or_ptr:" .. word, label .. ".Dynamic[" .. i .. "]", 6)
            p:mark(o + word * 2, stride - word * 2, label .. ".Dynamic.Extension", 6)
            -- Only read low tag values, leaving signed OS/processor tags exact as bytes.
            if not terminated then
                local bytes = p:bytes(o, word)
                if bytes == string.rep("\0", word) then terminated = true
                elseif strings then
                    local tag = u32(o + (if p.little then 0 else word - 4))
                    local high = if not wide then 0 else u32(o + (if p.little then 4 else 0))
                    if high == 0 and (tag == 1 or tag == 14 or tag == 15 or tag == 29 or tag == 0x7FFFFFFD or tag == 0x7FFFFFFF) then
                        table_string(strings, ux(o + word), label .. ".Dynamic.String")
                    end
                end
            end
        end)
        assert(terminated, "Missing DT_NULL")
    end
    local segments = {}
    if phnum > 0 then
        assert(phoff ~= 0, "Missing program header table")
        local minimum = if wide then 56 else 32
        p:array(phoff, phnum, phsize, minimum, function(o, i)
            local layout = if wide then "p_type:4 p_flags:4 p_offset:8 p_vaddr:8 p_paddr:8 p_filesz:8 p_memsz:8 p_align:8" else "p_type:4 p_offset:4 p_vaddr:4 p_paddr:4 p_filesz:4 p_memsz:4 p_flags:4 p_align:4"
            local kind = u32(o)
            local offset = ux(o + (if wide then 8 else 4))
            local size = ux(o + (if wide then 32 else 16))
            local label = "Segment[" .. i .. "] type=" .. kind
            p:struct(o, layout, label, 2)
            p:mark(o + minimum, phsize - minimum, "ProgramHeader.Extension", 2)
            p:mark(offset, size, label .. ".data", 4)
            table.insert(segments, { kind = kind, offset = offset, size = size, header = o, label = label, alignment = ux(o + (if wide then 48 else 28)) })
        end)
    end
    local function linked(s)
        assert(s.link < shnum and sections[s.link], "Invalid sh_link")
        return sections[s.link]
    end
    for i = 1, shnum - 1 do
        local s = sections[i]
        if s.kind == 0 or s.kind == 8 or s.size == 0 then continue end
        local label, o, size = s.label, s.offset, s.size
        if bit32.band(s.flags, 0x800) ~= 0 then
            local minimum = if wide then 24 else 12
            assert(size >= minimum, "Truncated compressed section")
            p:struct(o, if wide then "ch_type:4 ch_reserved:4 ch_size:8 ch_addralign:8" else "ch_type:4 ch_size:4 ch_addralign:4", label .. ".CompressionHeader", 6)
            p:mark(o + minimum, size - minimum, label .. ".CompressedPayload", 5)
            continue
        end
        local function records(minimum, callback)
            assert(s.stride >= minimum and size % s.stride == 0, "Invalid section entry size: " .. label)
            p:array(o, size / s.stride, s.stride, minimum, function(a, n)
                callback(a, n)
                p:mark(a + minimum, s.stride - minimum, label .. ".EntryExtension", 6)
            end)
        end
        if s.kind == 2 or s.kind == 11 then
            local strings = linked(s)
            records(if wide then 24 else 16, function(a, n)
                local layout = if wide then "st_name:4 st_info:1 st_other:1 st_shndx:2 st_value:8 st_size:8" else "st_name:4 st_value:4 st_size:4 st_info:1 st_other:1 st_shndx:2"
                p:struct(a, layout, label .. ".Symbol[" .. n .. "]", 6)
                table_string(strings, u32(a), label .. ".Symbol.Name")
            end)
        elseif s.kind == 4 or s.kind == 9 then
            local symbols = linked(s)
            assert(symbols.kind == 2 or symbols.kind == 11, "Relocations do not link to symbols")
            local rela = s.kind == 4
            records(word * (if rela then 3 else 2), function(a, n)
                p:struct(a, "r_offset:" .. word .. " r_info:" .. word .. (if rela then " r_addend:" .. word else ""), label .. ".Relocation[" .. n .. "]", 6)
            end)
        elseif s.kind == 6 then
            dynamic(o, size, s.stride, label, linked(s))
        elseif s.kind == 7 then
            notes(o, size, label, if s.alignment == 8 then 8 else 4)
        elseif s.kind == 5 then
            assert(size >= 8, "Truncated SysV hash header")
            local buckets, chains = u32(o), u32(o + 4)
            assert(8 + (buckets + chains) * 4 <= size, "Truncated SysV hash")
            p:struct(o, "nbucket:4 nchain:4", label .. ".Hash", 6)
            p:array(o + 8, buckets + chains, 4, 4, function(a, n)
                p:mark(a, 4, label .. (if n < buckets then ".Bucket[" .. n else ".Chain[" .. (n - buckets)) .. "]", 4)
            end)
        elseif s.kind == 0x6FFFFFF6 then
            assert(size >= 16, "Truncated GNU hash header")
            local buckets, bloom = u32(o), u32(o + 8)
            local prefix = 16 + bloom * word + buckets * 4
            assert(prefix <= size and (size - prefix) % 4 == 0, "Truncated GNU hash")
            p:struct(o, "nbuckets:4 symoffset:4 bloom_size:4 bloom_shift:4", label .. ".GnuHash", 6)
            p:array(o + 16, bloom, word, word, function(a, n) p:mark(a, word, label .. ".Bloom[" .. n .. "]", 4) end)
            p:array(o + 16 + bloom * word, buckets, 4, 4, function(a, n) p:mark(a, 4, label .. ".Bucket[" .. n .. "]", 5) end)
            p:array(o + prefix, (size - prefix) / 4, 4, 4, function(a, n) p:mark(a, 4, label .. ".Chain[" .. n .. "]", 6) end)
        elseif s.kind == 14 or s.kind == 15 or s.kind == 16 or s.kind == 19 or s.kind == 0x6FFFFFFF or s.kind == 17 then
            local width = if s.kind == 19 or s.kind == 17 then 4 elseif s.kind == 0x6FFFFFFF then 2 else word
            records(width, function(a, n) p:mark(a, width, label .. ".Entry[" .. n .. "]", 6) end)
        elseif s.kind == 0x6FFFFFFD or s.kind == 0x6FFFFFFE then
            local definition = s.kind == 0x6FFFFFFD
            local strings = linked(s)
            local relative, count = 0, 0
            local function at(rel, length)
                assert(rel >= 0 and rel <= size and length <= size - rel, "Version record outside section")
                return o + rel
            end
            repeat
                count += 1
                assert(count <= 200000, "Too many version records")
                local a = at(relative, if definition then 20 else 16)
                local n, aux, nextOffset
                if definition then
                    p:struct(a, "vd_version:2 vd_flags:2 vd_ndx:2 vd_cnt:2 vd_hash:4 vd_aux:4 vd_next:4", label .. ".Verdef", 6)
                    n, aux, nextOffset = u16(a + 6), u32(a + 12), u32(a + 16)
                else
                    p:struct(a, "vn_version:2 vn_cnt:2 vn_file:4 vn_aux:4 vn_next:4", label .. ".Verneed", 6)
                    n, aux, nextOffset = u16(a + 2), u32(a + 8), u32(a + 12)
                    table_string(strings, u32(a + 4), label .. ".Version.File")
                end
                local auxRelative = relative + aux
                assert(n == 0 or aux >= (if definition then 20 else 16), "Invalid version auxiliary offset")
                for j = 1, n do
                    local b = at(auxRelative, if definition then 8 else 16)
                    p:struct(b, if definition then "vda_name:4 vda_next:4" else "vna_hash:4 vna_flags:2 vna_other:2 vna_name:4 vna_next:4", label .. ".Version.Aux", 4)
                    table_string(strings, u32(b + (if definition then 0 else 8)), label .. ".Version.Name")
                    local nextAux = u32(b + (if definition then 4 else 12))
                    if j < n then assert(nextAux >= (if definition then 8 else 16), "Invalid auxiliary chain") end
                    auxRelative += nextAux
                end
                if nextOffset == 0 then break end
                assert(nextOffset >= (if definition then 20 else 16), "Invalid version chain")
                relative += nextOffset
            until false
            assert(s.info == 0 or count == s.info, "Version record count mismatch")
        elseif s.kind == 3 then
            -- String tables are regions; referenced names get separate annotations.
            assert(p:bytes(o, 1) == "\0" and p:bytes(o + size - 1, 1) == "\0", "Invalid string table boundaries")
        end
    end
    for _, s in segments do
        if s.kind == 3 and s.size > 0 then p:cstring(s.offset, s.size, s.label .. ".Interpreter", 5)
        elseif s.kind == 4 or s.kind == 0x6474E553 then notes(s.offset, s.size, s.label, if s.alignment == 8 then 8 else 4)
        elseif s.kind == 2 then dynamic(s.offset, s.size, word * 2, s.label, nil)
        end
    end
    p:finish()
end
