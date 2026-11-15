-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
-- PE32 / PE32+: file offsets, never loaded-image offsets.
return function(p)
    local function u16(o) return p:uint(o, 2) end
    local function u32(o) return p:uint(o, 4) end
    assert(p:bytes(0, 2) == "MZ", "Missing DOS signature")
    p:struct(0, "e_magic:2 e_cblp:2 e_cp:2 e_crlc:2 e_cparhdr:2 e_minalloc:2 e_maxalloc:2 e_ss:2 e_sp:2 e_csum:2 e_ip:2 e_cs:2 e_lfarlc:2 e_ovno:2 e_res:8 e_oemid:2 e_oeminfo:2 e_res2:20 e_lfanew:4", "DOS", 1)
    local nt = u32(60)
    assert(nt >= 64 and p:bytes(nt, 4) == "PE\0\0", "Invalid PE signature/offset")
    p:mark(64, nt - 64, "DOS stub / Rich header", 2)
    p:mark(nt, 4, "PE.Signature", 3)
    local coff = nt + 4
    p:struct(coff, "Machine:2 NumberOfSections:2 TimeDateStamp:4 PointerToSymbolTable:4 NumberOfSymbols:4 SizeOfOptionalHeader:2 Characteristics:2", "COFF", 2)
    local opt, optSize = coff + 20, u16(coff + 16)
    p:range(opt, optSize)
    local magic = u16(opt)
    assert(magic == 0x10B or magic == 0x20B, "Unsupported optional header magic")
    local wide = magic == 0x20B
    local word = if wide then 8 else 4
    local baseSize = if wide then 112 else 96
    assert(optSize >= baseSize, "Truncated optional header")
    local standard = "Magic:2 MajorLinkerVersion:1 MinorLinkerVersion:1 SizeOfCode:4 SizeOfInitializedData:4 SizeOfUninitializedData:4 AddressOfEntryPoint:4 BaseOfCode:4 "
    if not wide then standard ..= "BaseOfData:4 " end
    standard ..= "ImageBase:" .. word .. " SectionAlignment:4 FileAlignment:4 MajorOperatingSystemVersion:2 MinorOperatingSystemVersion:2 MajorImageVersion:2 MinorImageVersion:2 MajorSubsystemVersion:2 MinorSubsystemVersion:2 Win32VersionValue:4 SizeOfImage:4 SizeOfHeaders:4 CheckSum:4 Subsystem:2 DllCharacteristics:2 "
    for _, name in { "SizeOfStackReserve", "SizeOfStackCommit", "SizeOfHeapReserve", "SizeOfHeapCommit" } do
        standard ..= name .. ":" .. word .. " "
    end
    standard ..= "LoaderFlags:4 NumberOfRvaAndSizes:4"
    p:struct(opt, standard, if wide then "Optional64" else "Optional32", 3)
    local sections = {}
    local sectionCount = u16(coff + 2)
    p:array(opt + optSize, sectionCount, 40, 40, function(o, i)
        local rawName = p:bytes(o, 8)
        local name = string.match(rawName, "^[^%z]*")
        local label = "Section[" .. i .. "] " .. name
        p:struct(o, "Name:8 VirtualSize:4 VirtualAddress:4 SizeOfRawData:4 PointerToRawData:4 PointerToRelocations:4 PointerToLinenumbers:4 NumberOfRelocations:2 NumberOfLinenumbers:2 Characteristics:4", label, 4)
        local section = { rva = u32(o + 12), offset = u32(o + 20), size = u32(o + 16), virtualSize = u32(o + 8) }
        table.insert(sections, section)
        p:mark(section.offset, section.size, label .. ".data", 5 + i)
        local relocCount = u16(o + 32)
        local relocOffset = u32(o + 24)
        local overflow = bit32.band(u32(o + 36), 0x1000000) ~= 0
        if overflow then
            assert(relocCount == 65535, "Invalid relocation overflow flag")
            relocCount = u32(relocOffset)
        end
        p:array(relocOffset, relocCount, 10, 10, function(r, n)
            p:struct(r, "VirtualAddress:4 SymbolTableIndex:4 Type:2", label .. ".Relocation[" .. n .. "]", 6)
        end)
        p:array(u32(o + 28), u16(o + 34), 6, 6, function(r, n)
            p:struct(r, "TypeOrSymbol:4 Linenumber:2", label .. ".Line[" .. n .. "]", 6)
        end)
    end)
    local headers = u32(opt + 60)
    local function rva(address, size)
        size = size or 1
        if address < headers and size <= headers - address then p:range(address, size); return address end
        for _, s in sections do
            local delta = address - s.rva
            if delta >= 0 and delta < s.size and size <= s.size - delta then
                p:range(s.offset + delta, size)
                return s.offset + delta
            end
        end
        error(string.format("Unmapped file-backed RVA 0x%X + 0x%X", address, size))
    end
    local function rstring(address, name)
        -- Find the NUL only inside the mapped file-backed range.
        local o = rva(address)
        local available = if address < headers then headers - address else 0
        for _, s in sections do
            if address >= s.rva and address - s.rva < s.size then available = s.size - (address - s.rva); break end
        end
        return p:cstring(o, math.min(available, 65536), name, 5)
    end
    local names = { "Export", "Import", "Resource", "Exception", "Security", "BaseRelocation", "Debug", "Architecture", "GlobalPtr", "TLS", "LoadConfig", "BoundImport", "IAT", "DelayImport", "CLR", "Reserved" }
    local directories = {}
    local dirCount = u32(opt + baseSize - 4)
    assert(dirCount <= math.floor((optSize - baseSize) / 8), "Data directories exceed optional header")
    for i = 0, dirCount - 1 do
        local o = opt + baseSize + i * 8
        local name = names[i + 1] or ("Directory" .. i)
        p:struct(o, "VirtualAddress:4 Size:4", name .. ".Directory", 4)
        local address, size = u32(o), u32(o + 4)
        if address ~= 0 and size ~= 0 then
            local offset = if i == 4 then address else rva(address, size)
            p:mark(offset, size, name .. ".data", 5)
            directories[i] = { offset = offset, size = size, address = address }
        end
    end
    p:mark(opt + baseSize + dirCount * 8, optSize - baseSize - dirCount * 8, "OptionalHeader.extension", 3)
    local function entries(index, stride, callback)
        local d = directories[index]
        if not d then return end
        assert(d.size % stride == 0, "Misaligned " .. names[index + 1] .. " table")
        p:array(d.offset, d.size / stride, stride, stride, callback)
    end
    local function thunks(address, label)
        if address == 0 then return end
        for i = 0, 199999 do
            local o = rva(address + i * word, word)
            local lo = u32(o)
            local hi = if wide then u32(o + 4) else lo
            if lo == 0 and (not wide or hi == 0) then p:mark(o, word, label .. ".Terminator", 6); return end
            p:mark(o, word, label .. "[" .. i .. "]", 6)
            if bit32.band(hi, 0x80000000) == 0 then
                assert(not wide or hi == 0, "Invalid import name RVA")
                p:mark(rva(lo, 2), 2, label .. ".Hint", 4)
                rstring(lo + 2, label .. ".Name")
            end
        end
        error("Unterminated import thunk table")
    end
    local d = directories[0]
    if d then
        assert(d.size >= 40, "Truncated export directory")
        local o = d.offset
        p:struct(o, "Characteristics:4 TimeDateStamp:4 MajorVersion:2 MinorVersion:2 Name:4 Base:4 NumberOfFunctions:4 NumberOfNames:4 AddressOfFunctions:4 AddressOfNames:4 AddressOfNameOrdinals:4", "Export", 6)
        rstring(u32(o + 12), "Export.DLL")
        local funcs, count = u32(o + 20), u32(o + 24)
        local f = rva(u32(o + 28), funcs * 4)
        p:array(f, funcs, 4, 4, function(a, i)
            p:mark(a, 4, "Export.Address[" .. i .. "]", 4)
            local target = u32(a)
            if target >= d.address and target - d.address < d.size then rstring(target, "Export.Forwarder") end
        end)
        local n, ord = rva(u32(o + 32), count * 4), rva(u32(o + 36), count * 2)
        p:array(n, count, 4, 4, function(a, i)
            p:mark(a, 4, "Export.NameRVA[" .. i .. "]", 4)
            assert(u16(ord + i * 2) < funcs, "Invalid export ordinal")
            p:mark(ord + i * 2, 2, "Export.Ordinal[" .. i .. "]", 4)
            rstring(u32(a), "Export.Name[" .. i .. "]")
        end)
    end
    local function imports(index, delayed)
        local dir = directories[index]
        if not dir then return end
        local stride = if delayed then 32 else 20
        local label = names[index + 1]
        local terminated = false
        for i = 0, math.floor(dir.size / stride) - 1 do
            local o = dir.offset + i * stride
            if p:bytes(o, stride) == string.rep("\0", stride) then
                p:mark(o, stride, label .. ".Terminator", 6); terminated = true; break
            end
            if delayed then
                p:struct(o, "Attributes:4 Name:4 ModuleHandle:4 IAT:4 INT:4 BoundIAT:4 UnloadIAT:4 TimeDateStamp:4", label .. "[" .. i .. "]", 6)
            else
                p:struct(o, "OriginalFirstThunk:4 TimeDateStamp:4 ForwarderChain:4 Name:4 FirstThunk:4", label .. "[" .. i .. "]", 6)
            end
            local function address(at)
                local value = u32(at)
                if delayed and bit32.band(u32(o), 1) == 0 and value ~= 0 then
                    local imageBase = p:uint(opt + (if wide then 24 else 28), word)
                    value -= imageBase
                end
                return value
            end
            rstring(address(o + (if delayed then 4 else 12)), label .. ".DLL")
            local lookup = address(o + (if delayed then 16 else 0))
            local iat = address(o + (if delayed then 12 else 16))
            if lookup ~= 0 or (not delayed and u32(o + 4) == 0) then
                thunks(if lookup == 0 then iat else lookup, label .. ".INT")
            end
            -- Bound IAT values are VAs; color them as opaque entries rather than name RVAs.
            if iat ~= 0 then
                for j = 0, 199999 do
                    local a = rva(iat + j * word, word)
                    p:mark(a, word, label .. ".IAT[" .. j .. "]", 5)
                    if p:bytes(a, word) == string.rep("\0", word) then break end
                    assert(j < 199999, "Unterminated IAT")
                end
            end
        end
        assert(terminated, "Unterminated " .. label .. " descriptors")
    end
    imports(1, false)
    imports(13, true)
    d = directories[2]
    if d then
        local visited = {}
        local function resource(relative, depth)
            assert(depth < 32 and not visited[relative], "Cyclic/deep resource directory")
            visited[relative] = true
            local function at(rel, size)
                assert(rel >= 0 and rel <= d.size and size <= d.size - rel, "Resource exceeds directory")
                return d.offset + rel
            end
            local o = at(relative, 16)
            p:struct(o, "Characteristics:4 TimeDateStamp:4 MajorVersion:2 MinorVersion:2 NumberOfNamedEntries:2 NumberOfIdEntries:2", "Resource.Header", 6)
            local count = u16(o + 12) + u16(o + 14)
            at(relative + 16, count * 8)
            p:array(o + 16, count, 8, 8, function(a, i)
                p:struct(a, "NameOrId:4 OffsetToData:4", "Resource.Entry[" .. i .. "]", 4)
                local name, target = u32(a), u32(a + 4)
                if bit32.band(name, 0x80000000) ~= 0 then
                    local rel = bit32.band(name, 0x7FFFFFFF)
                    local text = at(rel, 2)
                    local length = u16(text)
                    at(rel + 2, length * 2)
                    p:mark(text, 2 + length * 2, "Resource.Name.UTF16", 5)
                end
                local rel = bit32.band(target, 0x7FFFFFFF)
                if bit32.band(target, 0x80000000) ~= 0 then resource(rel, depth + 1)
                else
                    local data = at(rel, 16)
                    p:struct(data, "OffsetToData:4 Size:4 CodePage:4 Reserved:4", "Resource.DataEntry", 6)
                    local size = u32(data + 4)
                    if size > 0 then p:mark(rva(u32(data), size), size, "Resource.Payload", 5) end
                end
            end)
            visited[relative] = nil
        end
        resource(0, 0)
    end
    d = directories[5]
    if d then
        local o, ending = d.offset, d.offset + d.size
        while o < ending do
            assert(ending - o >= 8, "Truncated base relocation block")
            local size = u32(o + 4)
            assert(size >= 8 and size % 2 == 0 and size <= ending - o, "Invalid relocation block size")
            p:struct(o, "VirtualAddress:4 SizeOfBlock:4", "BaseRelocation.Block", 6)
            p:array(o + 8, (size - 8) / 2, 2, 2, function(a, i) p:mark(a, 2, "BaseRelocation.TypeOffset[" .. i .. "]", 4) end)
            o += size
        end
    end
    entries(6, 28, function(o, i)
        p:struct(o, "Characteristics:4 TimeDateStamp:4 MajorVersion:2 MinorVersion:2 Type:4 SizeOfData:4 AddressOfRawData:4 PointerToRawData:4", "Debug[" .. i .. "]", 6)
        p:mark(u32(o + 24), u32(o + 16), "Debug.Payload", 5)
    end)
    d = directories[4]
    if d then
        local o, ending = d.offset, d.offset + d.size
        while o < ending do
            assert(ending - o >= 8, "Truncated certificate")
            local size = u32(o)
            assert(size >= 8 and size <= ending - o, "Invalid certificate length")
            p:struct(o, "dwLength:4 wRevision:2 wCertificateType:2", "Certificate", 6)
            p:mark(o + 8, size - 8, "Certificate.Payload", 5)
            local aligned = math.ceil(size / 8) * 8
            assert(aligned <= ending - o, "Truncated certificate padding")
            p:mark(o + size, aligned - size, "Certificate.Padding", 4)
            o += aligned
        end
    end
    d = directories[9]
    if d then
        local size = word * 4 + 8
        assert(d.size >= size, "Truncated TLS directory")
        p:struct(d.offset, "StartAddressOfRawData:" .. word .. " EndAddressOfRawData:" .. word .. " AddressOfIndex:" .. word .. " AddressOfCallBacks:" .. word .. " SizeOfZeroFill:4 Characteristics:4", "TLS", 6)
        local baseOffset = opt + (if wide then 24 else 28)
        local templateSize = p:delta(d.offset + word, d.offset, word)
        if templateSize > 0 then
            local start = p:delta(d.offset, baseOffset, word)
            p:mark(rva(start, templateSize), templateSize, "TLS.Template", 5)
        end
        local indexOffset = d.offset + word * 2
        if p:bytes(indexOffset, word) ~= string.rep("\0", word) then
            local address = p:delta(indexOffset, baseOffset, word)
            local memoryOnly = false
            for _, section in sections do
                local delta = address - section.rva
                if delta >= section.size and delta >= 0 and delta + 4 <= section.virtualSize then memoryOnly = true; break end
            end
            -- The loader often allocates the TLS index in .bss (no file bytes).
            if not memoryOnly then p:mark(rva(address, 4), 4, "TLS.Index", 5) end
        end
        local callbackOffset = d.offset + word * 3
        if p:bytes(callbackOffset, word) ~= string.rep("\0", word) then
            local callbacks = p:delta(callbackOffset, baseOffset, word)
            for i = 0, 199999 do
                local a = rva(callbacks + i * word, word)
                p:mark(a, word, "TLS.Callback[" .. i .. "]", 4)
                if p:bytes(a, word) == string.rep("\0", word) then break end
                assert(i < 199999, "Unterminated TLS callback array")
            end
        end
    end
    -- Architecture-specific exception records remain opaque except the standard AMD64 layout.
    if u16(coff) == 0x8664 then
        entries(3, 12, function(o, i) p:struct(o, "BeginAddress:4 EndAddress:4 UnwindInfoAddress:4", "RuntimeFunction[" .. i .. "]", 6) end)
    end
    d = directories[10]
    if d then
        local size = u32(d.offset)
        assert(size >= 4 and size <= d.size, "Invalid load configuration size")
        local layout = "Size:4 TimeDateStamp:4 MajorVersion:2 MinorVersion:2 GlobalFlagsClear:4 GlobalFlagsSet:4 CriticalSectionDefaultTimeout:4 "
        for _, n in { "DeCommitFreeBlockThreshold", "DeCommitTotalFreeThreshold", "LockPrefixTable", "MaximumAllocationSize", "VirtualMemoryThreshold" } do layout ..= n .. ":" .. word .. " " end
        layout ..= if wide then "ProcessAffinityMask:8 ProcessHeapFlags:4 " else "ProcessHeapFlags:4 ProcessAffinityMask:4 "
        layout ..= "CSDVersion:2 DependentLoadFlags:2 "
        for _, n in { "EditList", "SecurityCookie", "SEHandlerTable", "SEHandlerCount", "GuardCFCheckFunctionPointer", "GuardCFDispatchFunctionPointer", "GuardCFFunctionTable", "GuardCFFunctionCount" } do layout ..= n .. ":" .. word .. " " end
        layout ..= "GuardFlags:4 CodeIntegrity:12 "
        for _, n in { "GuardAddressTakenIatEntryTable", "GuardAddressTakenIatEntryCount", "GuardLongJumpTargetTable", "GuardLongJumpTargetCount", "DynamicValueRelocTable", "CHPEMetadataPointer", "GuardRFFailureRoutine", "GuardRFFailureRoutineFunctionPointer" } do layout ..= n .. ":" .. word .. " " end
        layout ..= "DynamicValueRelocTableOffset:4 DynamicValueRelocTableSection:2 Reserved2:2 GuardRFVerifyStackPointerFunctionPointer:" .. word .. " HotPatchTableOffset:4 Reserved3:4 EnclaveConfigurationPointer:" .. word .. " VolatileMetadataPointer:" .. word .. " GuardEHContinuationTable:" .. word .. " GuardEHContinuationCount:" .. word .. " GuardXFGCheckFunctionPointer:" .. word .. " GuardXFGDispatchFunctionPointer:" .. word .. " GuardXFGTableDispatchFunctionPointer:" .. word .. " CastGuardOsDeterminedFailureMode:" .. word .. " GuardMemcpyFunctionPointer:" .. word
        local cursor = 0
        for field, width in string.gmatch(layout, "([%w_]+):(%d+)") do
            local length = tonumber(width)
            if cursor + length > size then break end
            p:mark(d.offset + cursor, length, "LoadConfig." .. field, 6)
            cursor += length
        end
        p:mark(d.offset + cursor, size - cursor, "LoadConfig.Extension", 6)
    end
    d = directories[11]
    if d then
        local o, ending, terminated = d.offset, d.offset + d.size, false
        while o + 8 <= ending do
            local stamp, name, refs = u32(o), u16(o + 4), u16(o + 6)
            p:struct(o, "TimeDateStamp:4 OffsetModuleName:2 NumberOfModuleForwarderRefs:2", "BoundImport", 6)
            o += 8
            if stamp == 0 and name == 0 and refs == 0 then terminated = true; break end
            assert(name < d.size, "Invalid bound import name")
            p:cstring(d.offset + name, d.size - name, "BoundImport.DLL", 5)
            assert(refs * 8 <= ending - o, "Truncated bound forwarders")
            p:array(o, refs, 8, 8, function(a)
                p:struct(a, "TimeDateStamp:4 OffsetModuleName:2 Reserved:2", "BoundImport.Forwarder", 4)
                local n = u16(a + 4)
                assert(n < d.size, "Invalid bound forwarder name")
                p:cstring(d.offset + n, d.size - n, "BoundImport.Forwarder.DLL", 5)
            end)
            o += refs * 8
        end
        assert(terminated, "Unterminated bound imports")
    end
    d = directories[14]
    if d then
        assert(d.size >= 72 and u32(d.offset) >= 72, "Truncated CLR header")
        p:struct(d.offset, "cb:4 MajorRuntimeVersion:2 MinorRuntimeVersion:2 MetaDataRVA:4 MetaDataSize:4 Flags:4 EntryPointTokenOrRVA:4 ResourcesRVA:4 ResourcesSize:4 StrongNameSignatureRVA:4 StrongNameSignatureSize:4 CodeManagerTableRVA:4 CodeManagerTableSize:4 VTableFixupsRVA:4 VTableFixupsSize:4 ExportAddressTableJumpsRVA:4 ExportAddressTableJumpsSize:4 ManagedNativeHeaderRVA:4 ManagedNativeHeaderSize:4", "CLR", 6)
        for _, delta in { 8, 24, 32, 40, 48, 56, 64 } do
            local address, size = u32(d.offset + delta), u32(d.offset + delta + 4)
            if address ~= 0 and size > 0 then p:mark(rva(address, size), size, "CLR.Subdirectory[" .. delta .. "]", 5) end
        end
    end
    local symbols, symbolCount = u32(coff + 8), u32(coff + 12)
    if symbols ~= 0 then
        local strings = symbols + symbolCount * 18
        local stringSize = u32(strings)
        assert(stringSize >= 4, "Invalid COFF string table")
        p:mark(strings, stringSize, "COFF.StringTable", 5)
        p:range(symbols, symbolCount * 18)
        local i = 0
        while i < symbolCount do
            local o = symbols + i * 18
            p:struct(o, "Name:8 Value:4 SectionNumber:2 Type:2 StorageClass:1 NumberOfAuxSymbols:1", "COFF.Symbol[" .. i .. "]", 6)
            local aux = p:uint(o + 17, 1)
            assert(aux < symbolCount - i, "Invalid auxiliary symbol count")
            p:mark(o + 18, aux * 18, "COFF.AuxSymbols", 4)
            i += 1 + aux
        end
    end
    p:finish()
end
