#include <wingsummer>

class CMyClass {
    int method()       { a++; return a; }
    int method() const {      return a; }
    int a;
}

int main() {
	// This is a script

    for (int i = 0; i < 5; ++i){
        print("hello world!")
    }

    print("AngelScript for WingHexExplorer2 by wingsummer.")

    return 0;
}
