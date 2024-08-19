/*
        Transition header file.
*/

#ifndef _QFORMAT_FACTORY_H_

#include "qformatscheme.h"

class QFormatFactory : public QFormatScheme {
public:
    inline QFormatFactory(QObject *p = nullptr) : QFormatScheme(p) {}
    inline QFormatFactory(const QString &f, QObject *p = nullptr)
        : QFormatScheme(f, p) {}
};

#endif
