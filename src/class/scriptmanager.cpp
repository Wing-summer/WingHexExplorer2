#include "scriptmanager.h"

#include "../dbghelper.h"

ScriptManager &ScriptManager::instance() {
    static ScriptManager ins;
    return ins;
}

ScriptManager::ScriptManager() : QObject() {
    ASSERT_SINGLETON;

    sout = new callback_streambuf(
        std::bind(&ScriptManager::messageCallBack, this, STD_OUTPUT::STD_OUT,
                  std::placeholders::_1, std::placeholders::_2));
    serr = new callback_streambuf(
        std::bind(&ScriptManager::messageCallBack, this, STD_OUTPUT::STD_ERROR,
                  std::placeholders::_1, std::placeholders::_2));

    std_out = std::cout.rdbuf(sout);
    std_err = std::cerr.rdbuf(serr);
}

ScriptManager::~ScriptManager() {
    std::cout.rdbuf(std_out);
    std::cerr.rdbuf(std_err);
    delete sout;
    delete serr;
}

void ScriptManager::messageCallBack(STD_OUTPUT io, const char *str,
                                    std::streamsize size) {
    switch (io) {
    case STD_OUTPUT::STD_OUT:
        std_out->sputn(str, size);
        break;
    case STD_OUTPUT::STD_ERROR:
        std_err->sputn(str, size);
        break;
    }
    emit messageOut(io, QString::fromStdString(std::string(str, size)));
}
