#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QtGlobal>

class CrashHandler {
public:
    static CrashHandler &instance();

    void init();

    static void reportCrashAndExit();

private:
    CrashHandler();
};

#endif // CRASHHANDLER_H
