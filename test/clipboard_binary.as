// clipboard_binary.as
// Test clipboard binary set/get (may be platform-dependent)
void main() {
    array<uint8> bin;
    bin.insertLast(1);
    bin.insertLast(2);
    clipboard::setBinary(@bin);
    array<byte>@ rout = clipboard::getBinary();
    if (rout !is null) {
        println("clipboard binary len: " + rout.length());
    } else {
        println("clipboard getBinary returned null");
    }
    print('clipboard content: ', rout, '\n');
    println("clipboard_binary OK");
}
