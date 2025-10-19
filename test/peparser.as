// Written by wingsummer
// 该脚本仅简单的测试标注高亮

#pragma WingCStruct Inc "WinNTPE.h"

void cleanUp(int handle) {
    ctl::endMarco();
    ctl::closeHandle(handle);
}

int main() {
    int NOT_FOUND_HEADER = 1;
    int CRASH_STRUCT = 2;

    if (!reader::isCurrentDocEditing()) {
        print("No file is editing!");
        return 0;
    }

    auto e = WingCStruct::containsStruct("IMAGE_DOS_HEADER");
    if (!e) {
        print("IMAGE_DOS_HEADER not found!");
        return NOT_FOUND_HEADER;
    }

    auto idh = WingCStruct::read(0, "IMAGE_DOS_HEADER");

    if (idh.isEmpty()) {
        print("IMAGE_DOS_HEADER crash");
        return CRASH_STRUCT;
    }

    e = WingCStruct::containsConstVar("IMAGE_DOS_SIGNATURE");
    if (!e) {
        print("IMAGE_DOS_SIGNATURE not found");
        return NOT_FOUND_HEADER;
    }

    auto dv = WingCStruct::constVarValueInt("IMAGE_DOS_SIGNATURE");
    if (int64(idh["e_magic"]) != dv) {
        print("Invalid PE file!");
        return CRASH_STRUCT;
    }

    auto peoff = int64(idh["e_lfanew"]);
    e = WingCStruct::containsConstVar("IMAGE_NT_SIGNATURE");
    if (!e) {
        print("IMAGE_NT_SIGNATURE not found");
        return NOT_FOUND_HEADER;
    }

    auto pesig = int32(reader::readInt32(peoff));
    auto psig = WingCStruct::constVarValueInt("IMAGE_NT_SIGNATURE");
    if (pesig != psig) {
        print("Invalid PE file!");
        return CRASH_STRUCT;
    }

    print("Start parsing...\n");
    auto handle = ctl::openCurrent();
    ctl::beginMarco("PEParser");

    auto structSize = WingCStruct::sizeOf("IMAGE_DOS_HEADER");
    ctl::metadata(0, structSize, color("#e5c07b"), color(), "IMAGE_DOS_HEADER");
    ctl::comment(structSize, peoff - structSize, "DosStub");
    ctl::foreground(peoff, 4, color("#06d8bd"));

    e = WingCStruct::containsStruct("IMAGE_FILE_HEADER");
    if (!e) {
        print("IMAGE_FILE_HEADER not found!");
        cleanUp(handle);
        return NOT_FOUND_HEADER;
    }

    auto ifhoff = peoff + 4;
    auto ifh = WingCStruct::read(ifhoff, "IMAGE_FILE_HEADER");

    if (ifh.isEmpty()) {
        print("IMAGE_FILE_HEADER crash");
        cleanUp(handle);
        return CRASH_STRUCT;
    }

    auto numberOfSect = int64(ifh["NumberOfSections"]);

    structSize = WingCStruct::sizeOf("IMAGE_FILE_HEADER");
    ctl::metadata(ifhoff, structSize, color("#259f5e"), color(),
        "IMAGE_FILE_HEADER");

    e = WingCStruct::containsConstVar("IMAGE_FILE_MACHINE_I386");
    if (!e) {
        print("IMAGE_FILE_MACHINE_I386 not found");
        cleanUp(handle);
        return NOT_FOUND_HEADER;
    }

    e = WingCStruct::containsConstVar("IMAGE_FILE_MACHINE_AMD64");
    if (!e) {
        print("IMAGE_FILE_MACHINE_AMD64 not found");
        cleanUp(handle);
        return NOT_FOUND_HEADER;
    }

    auto x32sig = WingCStruct::constVarValueInt("IMAGE_FILE_MACHINE_I386");
    auto x64sig = WingCStruct::constVarValueInt("IMAGE_FILE_MACHINE_AMD64");

    auto iohoff = ifhoff + structSize;
    auto mcsig = int64(ifh["Machine"]);
    int64 iddoff = 0;
    if (x32sig == mcsig) {
        print("X86 PE file\n");
        e = WingCStruct::containsStruct("IMAGE_OPTIONAL_HEADER32");
        if (!e) {
            print("IMAGE_OPTIONAL_HEADER32 not found!");
            cleanUp(handle);
            return NOT_FOUND_HEADER;
        }
        auto ioh = WingCStruct::read(iohoff, "IMAGE_OPTIONAL_HEADER32");
        if (ioh.isEmpty()) {
            print("IMAGE_OPTIONAL_HEADER32 crash");
            cleanUp(handle);
            return NOT_FOUND_HEADER;
        }

        structSize = WingCStruct::sizeOf("IMAGE_OPTIONAL_HEADER32");
        ctl::metadata(iohoff, structSize, color(), color("#3e80d3"), "");
        iddoff = iohoff + structSize;
    } else if (x64sig == mcsig) {
        print("X64 PE file\n");
        e = WingCStruct::containsStruct("IMAGE_OPTIONAL_HEADER64");
        if (!e) {
            print("IMAGE_OPTIONAL_HEADER64 not found!");
            cleanUp(handle);
            return NOT_FOUND_HEADER;
        }
        auto ioh = WingCStruct::read(iohoff, "IMAGE_OPTIONAL_HEADER64");
        if (ioh.isEmpty()) {
            print("IMAGE_OPTIONAL_HEADER64 crash");
            cleanUp(handle);
            return NOT_FOUND_HEADER;
        }

        structSize = WingCStruct::sizeOf("IMAGE_OPTIONAL_HEADER64");
        ctl::metadata(iohoff, structSize, color(), color("#3e80d3"), "");
        iddoff = iohoff + structSize;
    } else {
        print("Unsuported PE file\n");
        cleanUp(handle);
        return CRASH_STRUCT;
    }

    e = WingCStruct::containsStruct("IMAGE_SECTION_HEADER");
    if (!e) {
        print("IMAGE_SECTION_HEADER not found!");
        cleanUp(handle);
        return NOT_FOUND_HEADER;
    }

    structSize = WingCStruct::sizeOf("IMAGE_SECTION_HEADER");

    for (auto i = 0; i < numberOfSect; i++) {
        color c;
        if (i % 2 == 1) {
            c = color("#86d585");
        } else {
            c = color("#fffc6c");
        }
        ctl::metadata(iddoff, structSize, color(), c,
            "IMAGE_SECTION_HEADER [" + i + "]");
        iddoff += structSize;
    }

    print("PE Parsing Finshed\n");
    cleanUp(handle);
    return 0;
}
