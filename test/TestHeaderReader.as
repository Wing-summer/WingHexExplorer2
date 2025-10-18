#pragma WingCStruct Pak 1
#pragma WingCStruct Inc "TestHeader.h"

int main() {
    auto c = WingCStruct::read(0, "TestHeader");
    println(WingCStruct::sizeOf("TestHeader"));
    println(beautify(stringify(c)));
    return 0;
}
