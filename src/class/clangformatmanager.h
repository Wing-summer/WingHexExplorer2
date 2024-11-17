#ifndef CLANGFORMATMANAGER_H
#define CLANGFORMATMANAGER_H

#include <QString>

class ClangFormatManager {
public:
    static ClangFormatManager &instance();

public:
    bool exists();

    bool refind();

    static QString getProgramName();

    static QStringList supportedStyles();

    QString formatCode(const QString &codes, bool &ok);

    int runningTimeout() const;

    void setRunningTimeout(int msecs);

    QString version() const;

    QString customStyleString() const;

    void setCustomStyleString(const QString &newCustomStyleString);

    QString path() const;

    bool enabled() const;
    void setEnabled(bool newEnabled);

    QString clangStyle() const;
    void setClangStyle(const QString &newClangStyle);

    quint32 identWidth() const;
    void setIdentWidth(quint32 newIdentWidth);

private:
    explicit ClangFormatManager();

    QString runClangFormat(const QStringList &params, const QString &stdinput,
                           bool &ok);

private:
    bool m_enabled = true;
    bool m_autoFmt = true;

    quint32 m_identWidth = 4;

    QString m_clangPath;
    QString m_clangVersion;
    QString m_clangStyle;

    QString m_customStyleString;

    int m_timeout = 5000;
};

#endif // CLANGFORMATMANAGER_H
