/* @brief 一个拷贝选中字节为 C/C++ 样式数组的脚本
 * @license BSD 3-Clause License
 * @author 寂静的羽夏 (wingsummer)
 */

string header = "[" + __SECTION_BASE__ + "] ";
string icon = ":/com.wingsummer.winghex/images/copyhex.png";

enum Message { LowVersion, NoFileEdit, NoCopy, CopyClipBoard }

char toHexUpper(byte value) { return "0123456789ABCDEF"[value & 0xF]; }

string trMsg(Message msg) {
#if __LANG__ == "zh_CN"
    if (msg == Message::LowVersion) {
        return "该脚本仅支持在软件 2.2.3 及其之后版本下运行";
    } else if (msg == Message::NoFileEdit) {
        return "无当前编辑的文件";
    } else if (msg == Message::NoCopy) {
        return "无拷贝项目";
    } else if (msg == Message::CopyClipBoard) {
        return "已拷贝到剪切板";
    }
#else
    if (msg == Message::LowVersion) {
        return "Script should be run with verison higher than or equal to " +
            "2.2.3";
    } else if (msg == Message::NoFileEdit) {
        return "There is no editing file";
    } else if (msg == Message::NoCopy) {
        return "Nothing to copy";
    } else if (msg == Message::CopyClipBoard) {
        return "Copy to clipboard";
    }
#endif
    return "";
}

string toHex(byte value) {
    string ret = "0x";
    ret += toHexUpper(value >> 4);
    ret += toHexUpper(value & 0xF);
    return ret;
}

bool check() {
#if __WING_VERSION_MAJOR__ * 10000 + __WING_VERSION_MINOR__  * 100 + __WING_VERSION_PATCH__ >= 20203
    return true;
#else
    return false;
#endif
}

int main() {
    if (!check()) {
        toast(header + trMsg(Message::LowVersion), icon);
        return -2;
    }

    if (!reader::isCurrentDocEditing()) {
        toast(header + trMsg(Message::NoFileEdit), icon);
        return -1;
    }

    int64 total = reader::selectionCount();

    if (total < 1) {
        toast(header + trMsg(Message::NoCopy), icon);
        return 0;
    }

    string buffer = "unsigned char array[] = {";

    string[] strs;

    for (int64 i = 0; i < total; i++) {
        byte[] bytes = reader::selectedBytes(i);
        string str = toHex(bytes[0]);
        for (uint p = 1; p < bytes.length(); p++) {
            str += ", ";
            str += toHex(bytes[p]);
        }
        strs.insertLast(str);
    }

    buffer += string::join(strs, ", ");
    buffer += '};';

    clipboard::setText(buffer);

    toast(header + trMsg(Message::CopyClipBoard), icon);
    return 0;
}
