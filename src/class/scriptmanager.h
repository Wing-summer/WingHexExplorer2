#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include <QObject>

#include <iostream>

class ScriptManager : public QObject {
    Q_OBJECT

private:
    class callback_streambuf : public std::streambuf {
    public:
        callback_streambuf(
            std::function<void(char const *, std::streamsize)> callback)
            : callback(callback) {}

    protected:
        std::streamsize xsputn(char_type const *s,
                               std::streamsize count) override {
            callback(s, count);
            return count;
        }

    private:
        std::function<void(char const *, std::streamsize)> callback;
    };

public:
    explicit ScriptManager(QObject *parent = nullptr);

signals:
};

#endif // SCRIPTMANAGER_H
