// This script is a example for reading import table and extracting infos with WingCStruct
// @author wingsummer
// @License BSD
// @date 2026/03/02

enum IMAGE_DIRECTORY_ENTRY_INDEX {
    ENTRY_EXPORT = 0, // Export Directory
    ENTRY_IMPORT = 1, // Import Directory
    ENTRY_RESOURCE = 2, // Resource Directory
    ENTRY_EXCEPTION = 3, // Exception Directory
    ENTRY_SECURITY = 4, // Security Directory
    ENTRY_BASERELOC = 5, // Base Relocation Table
    ENTRY_DEBUG = 6, // Debug Directory
    ENTRY_COPYRIGHT = 7, // (X86 usage)
    ENTRY_ARCHITECTURE = 7, // Architecture Specific Data
    ENTRY_GLOBALPTR = 8, // RVA of GP
    ENTRY_TLS = 9, // TLS Directory
    ENTRY_LOAD_CONFIG = 10, // Load Configuration Directory
    ENTRY_BOUND_IMPORT = 11, // Bound Import Directory in headers
    ENTRY_IAT = 12, // Import Address Table
    ENTRY_DELAY_IMPORT = 13, // Delay Load Import Descriptors
    ENTRY_COM_DESCRIPTOR = 14 // COM Runtime descriptor
}

class IMAGE_DIRECTORY_ENTRY {
    uint32 VirtualAddress;
    uint32 Size;
}

class IMAGE_SECTION_HEADER {
    uint32 VirtualAddress;
    uint32 Size;
    uint32 PointerToRawData;
}

int64 rva2foa(IMAGE_SECTION_HEADER[] headers, uint rva) {
    if (rva == 0) {
        return 0;
    }
    foreach(auto item : headers){
        if(rva >= item.VirtualAddress && rva < item.VirtualAddress + item.Size){
            auto delta = rva - item.VirtualAddress;
            return int64(item.PointerToRawData + delta);
        }
    }
    return 0;
}

int main() {
    int NOT_FOUND_HEADER = 1;
    int CRASH_STRUCT = 2;
    
    if (!reader::isCurrentDocEditing()) {
        print("No file is editing!");
        return 0;
    }

    WingCStruct::parser parser("winnt.h");

    auto e = parser.containsStruct("IMAGE_DOS_HEADER");
    if (!e) {
        print("IMAGE_DOS_HEADER not found!");
        return NOT_FOUND_HEADER;
    }

    auto idh = parser.readMembers(0, "IMAGE_DOS_HEADER", { "e_magic", "e_lfanew" });
    if (idh.isEmpty()) {
        print("IMAGE_DOS_HEADER crash");
        return NOT_FOUND_HEADER;
    }

    auto dv = parser.constVarValueInt("IMAGE_DOS_SIGNATURE");
    if (int64(idh["e_magic"]) != dv) {
        print("Invalid PE file!");
        return CRASH_STRUCT;
    }

    auto peoff = int64(idh["e_lfanew"]);
    e = parser.containsConstVar("IMAGE_NT_SIGNATURE");
    if (!e) {
        print("IMAGE_NT_SIGNATURE not found");
        return NOT_FOUND_HEADER;
    }

    auto pesig = int32(reader::readInt32(peoff));
    auto psig = parser.constVarValueInt("IMAGE_NT_SIGNATURE");
    if (pesig != psig) {
        print("Invalid PE file!");
        return CRASH_STRUCT;
    }

    println("Start parsing...");

    e = parser.containsStruct("IMAGE_FILE_HEADER");
    if (!e) {
        print("IMAGE_FILE_HEADER not found!");
        return NOT_FOUND_HEADER;
    }

    auto ifhoff = peoff + parser.sizeOf("DWORD");
    auto ifh = parser.readMembers(ifhoff, "IMAGE_FILE_HEADER", { "NumberOfSections", "Machine" });

    if (ifh.isEmpty()) {
        print("IMAGE_FILE_HEADER crash");
        return CRASH_STRUCT;
    }

    auto numberOfSect = uint(ifh["NumberOfSections"]);
    e = parser.containsConstVar("IMAGE_FILE_MACHINE_I386");
    if (!e) {
        print("IMAGE_FILE_MACHINE_I386 not found");
        return NOT_FOUND_HEADER;
    }

    e = parser.containsConstVar("IMAGE_FILE_MACHINE_AMD64");
    if (!e) {
        print("IMAGE_FILE_MACHINE_AMD64 not found");
        return NOT_FOUND_HEADER;
    }

    auto x64sig = parser.constVarValue("IMAGE_FILE_MACHINE_AMD64").toInt();

    auto iohoff = ifhoff + parser.sizeOf("IMAGE_FILE_HEADER");
    auto mcsig = int64(ifh["Machine"]);
    int64 ishoff = 0;
    if (x64sig == mcsig) {
        println("> X64 PE file\n");
        e = parser.containsStruct("IMAGE_OPTIONAL_HEADER64");
        if (!e) {
            print("IMAGE_OPTIONAL_HEADER64 not found!");
            return NOT_FOUND_HEADER;
        }
        ishoff = iohoff + parser.sizeOf("IMAGE_OPTIONAL_HEADER64");
    } else {
        println("Unsuported PE file");
        return CRASH_STRUCT;
    }

    e = parser.containsStruct("IMAGE_SECTION_HEADER");
    if (!e) {
        print("IMAGE_SECTION_HEADER not found!");
        return NOT_FOUND_HEADER;
    }

    auto SIZE_OF_SECTION = parser.sizeOf("IMAGE_SECTION_HEADER");
    IMAGE_SECTION_HEADER[] sections;
    sections.reserve(numberOfSect);
    for (uint i = 0; i < numberOfSect; i++) {
        auto sh = parser.readMembers(ishoff + i * SIZE_OF_SECTION, "IMAGE_SECTION_HEADER", { "Misc", "VirtualAddress", "SizeOfRawData", "PointerToRawData" });
        IMAGE_SECTION_HEADER header;
        header.PointerToRawData = uint(sh["PointerToRawData"]);
        header.VirtualAddress = uint(sh["VirtualAddress"]);
        auto rawSize = uint(sh["SizeOfRawData"]);
        auto misc = cast<dictionary>(sh["Misc"]);
        auto vSize = uint(misc["VirtualSize"]);
        header.Size = vSize > rawSize ? vSize : rawSize;
        sections.insertLast(header);
    }

    auto SIZE_OF_DD = parser.sizeOf("IMAGE_DATA_DIRECTORY");
    auto ENTRY_COUNT = parser.constVarValue("IMAGE_NUMBEROF_DIRECTORY_ENTRIES").toInt();

    auto iddoff = ishoff - ENTRY_COUNT * SIZE_OF_DD;

    auto dd = parser.read(iddoff + IMAGE_DIRECTORY_ENTRY_INDEX::ENTRY_IMPORT * SIZE_OF_DD, "IMAGE_DATA_DIRECTORY");
    IMAGE_DIRECTORY_ENTRY import_dd;
    import_dd.VirtualAddress = uint32(dd["VirtualAddress"]);
    import_dd.Size = uint(dd["Size"]);
        
    auto desc_off = rva2foa(sections, import_dd.VirtualAddress);
    assert(desc_off != 0);

    auto IMPORT_DESCRIPTOR_SIZE = parser.sizeOf("IMAGE_IMPORT_DESCRIPTOR");
    auto ORDINAL_FLAG64 = parser.constVarValue("IMAGE_ORDINAL_FLAG64").toUInt();
    println("--- IMPORT_TABLE ---\n");
    while (true) {
        auto im = parser.readMembers(desc_off, "IMAGE_IMPORT_DESCRIPTOR", { "DUMMYUNIONNAME", "Name" });
        auto DUMMYUNIONNAME = cast<dictionary>(im["DUMMYUNIONNAME"]);
        auto OriginalFirstThunk = uint32(DUMMYUNIONNAME["OriginalFirstThunk"]);
        if (OriginalFirstThunk == 0) {
            break;
        }

        auto name_rva = uint32(im["Name"]);
        auto name_ofa = rva2foa(sections, name_rva);
        assert(name_ofa != 0);
        printf("<{}>\n", reader::readString(name_ofa));

        auto thunk_foa = rva2foa(sections, OriginalFirstThunk);
        assert(thunk_foa != 0);

        auto thunk = reader::readUInt64(thunk_foa);
        if (thunk & ORDINAL_FLAG64 != 0) {
            printf("#ORD {}\n", thunk & 0xFFFF);
        } else {
            // parsing IMAGE_IMPORT_BY_NAME
            auto fn_foa = rva2foa(sections, uint32(thunk)) + 2;
            printf("- {}\n", reader::readString(fn_foa));
        }

        print('\n');
        desc_off += IMPORT_DESCRIPTOR_SIZE;
    }

    return 0;
}
