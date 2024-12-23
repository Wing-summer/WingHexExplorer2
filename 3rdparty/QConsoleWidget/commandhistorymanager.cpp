#include "commandhistorymanager.h"

#include "utilities.h"

#include <QTextStream>

QStringList CommandHistoryManager::load() {
    QStringList ret;
    QFile f(getHistoryPath());
    if (f.open(QFile::ReadOnly)) {
        QTextStream is(&f);
        while (!is.atEnd()) {
            ret.append(is.readLine());
        }
    }
    return ret;
}

void CommandHistoryManager::save(const QStringList &strings) {
    QFile f(getHistoryPath());
    if (f.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream os(&f);
        int n = strings.size();
        while (n > 0)
            os << strings.at(--n) << Qt::endl;
    }
}

QString CommandHistoryManager::getHistoryPath() {
    QDir dir(Utilities::getAppDataPath());
    return dir.absoluteFilePath(QStringLiteral(".command_history.lst"));
}
