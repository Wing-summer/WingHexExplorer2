#ifndef SHOWINSHELL_H
#define SHOWINSHELL_H

#include <QString>
#include <QWidget>

struct HostOsInfo {
#ifdef Q_OS_MAC
    static constexpr bool isMacHost() { return true; }
#else
    static constexpr bool isMacHost() { return false; }
#endif
#ifdef Q_OS_WIN
    static constexpr bool isWindowsHost() { return true; }
#else
    static constexpr bool isWindowsHost() { return false; }
#endif
};

class ShowInShell {
public:
    static bool showInGraphicalShell(QWidget *parent, const QString &pathIn,
                                     bool deselect);

private:
    static bool showInWindowsShell(const QString &filePath, bool deselect);
};

#endif // SHOWINSHELL_H
