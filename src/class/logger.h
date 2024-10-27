#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QObject>
#include <QSharedPointer>
#include <QTextStream>

class Logger : public QObject {
    Q_OBJECT
public:
    // http://en.wikipedia.org/wiki/Java_logging_framework
    // http://commons.apache.org/logging/guide.html#Message%20Priorities/Levels
    //!
    enum Level : int {
        q0FATAL,
        q1ERROR,
        q2WARN,
        q3INFO,
        q4DEBUG,
        q5TRACE,
        LEVEL_BEGIN = q0FATAL,
        LEVEL_LAST = q5TRACE,
        LEVEL_END = LEVEL_LAST + 1
    };

public:
    static Logger &instance();

    Level logLevel() const;

    static QString getString(Level level);

    static Level defaultLevel();

signals:
    void log(const QString &message);

public slots:
    // internal use only
    static void _log(const QString &message);

    // external use
    static void trace(const QString &message);
    static void warning(const QString &message);
    static void info(const QString &message);
    static void debug(const QString &message);
    static void critical(const QString &message);

    void setLogLevel(Level level);

private:
    explicit Logger(QObject *parent = nullptr);

    virtual ~Logger();

    Q_DISABLE_COPY_MOVE(Logger)

private:
    Level _level = Level::q4DEBUG;
    QSharedPointer<QFile> _file = nullptr;
    QSharedPointer<QTextStream> _stream = nullptr;
};

#endif // LOGGER_H
