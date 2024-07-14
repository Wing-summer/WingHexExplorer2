#ifndef DEFINE_H
#define DEFINE_H

#include <QObject>

#define APP_ORG "WingCloudStudio"
#define APP_NAME "WingHexExplorer2"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef qsizetype qindextype;
#else
typedef int qindextype;
#endif

#endif // DEFINE_H
