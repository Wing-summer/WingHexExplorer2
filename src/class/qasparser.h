#ifndef _QAS_PARSER_H_
#define _QAS_PARSER_H_

#include <QObject>

class QAsParser : public QObject {
    Q_OBJECT
public:
    explicit QAsParser(QObject *parent = nullptr);
    virtual ~QAsParser();
};

#endif // !_QCPP_PARSER_H_
