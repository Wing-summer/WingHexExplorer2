#ifndef DEFINE_H
#define DEFINE_H

#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef qsizetype qindextype;
#else
typedef int qindextype;
#endif

#endif // DEFINE_H
