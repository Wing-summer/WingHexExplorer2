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
    static ScriptManager &instance();

public:
    enum class STD_OUTPUT { STD_OUT, STD_ERROR };
    Q_ENUM(STD_OUTPUT)

signals:
    void messageOut(STD_OUTPUT io, QString message);

private:
    explicit ScriptManager();
    virtual ~ScriptManager();

    Q_DISABLE_COPY(ScriptManager)

    void messageCallBack(ScriptManager::STD_OUTPUT io, char const *str,
                         std::streamsize size);

private:
    callback_streambuf *sout = nullptr;
    callback_streambuf *serr = nullptr;

    std::streambuf *std_out = nullptr;
    std::streambuf *std_err = nullptr;
};

#endif // SCRIPTMANAGER_H
