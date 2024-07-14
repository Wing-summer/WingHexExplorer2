#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    static Logger *getInstance();

signals:
    void log(const QString &message);

public slots:
    // internal use only
    static void _log(const QString &message);

    // external use
    static void warning(const QString &message);
    static void info(const QString &message);
    static void debug(const QString &message);
    static void critical(const QString &message);

private:
    static Logger *instance;
};

#endif // LOGGER_H
