/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

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
    bool setClangStyle(const QString &newClangStyle);
    int clangCurrentStyleIndex() const;

    quint32 identWidth() const;
    void setIdentWidth(quint32 newIdentWidth);

public:
    void save();
    void reset();

    bool autoFormat() const;
    void setAutoFormat(bool newAutoFmt);

private:
    explicit ClangFormatManager();

    bool checkClangCustomStyle(const QString &styles);

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

private:
    Q_DISABLE_COPY_MOVE(ClangFormatManager)
};

#endif // CLANGFORMATMANAGER_H
