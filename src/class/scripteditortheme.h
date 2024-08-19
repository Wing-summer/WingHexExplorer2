#ifndef SCRIPTEDITORTHEME_H
#define SCRIPTEDITORTHEME_H

#include <QObject>

#include <QTextCharFormat>
#include <QXmlStreamAttributes>

class ScriptEditorTheme : public QObject {
    Q_OBJECT
public:
    enum class Styles {

    };
    Q_ENUM(Styles)

    struct ColorScheme {
        QColor foreground;
        QColor background;
        QString fontFamily;
        QTextCharFormat::UnderlineStyle underLine =
            QTextCharFormat::NoUnderline;
        bool isBold = false;
        bool isItalic = false;
    };

public:
    explicit ScriptEditorTheme(QObject *parent = nullptr);
    explicit ScriptEditorTheme(const QString &filename = QString(),
                               QObject *parent = nullptr);

    ColorScheme colorScheme(Styles style) const;

    void setColorScheme(Styles style, const ColorScheme &scheme);

    bool loadTheme(const QString &filename);

    bool saveTheme(const QString &filename);

    bool isValid() const;

    QString name() const;

    QString version() const;

    QString author() const;

    QString comment() const;

private:
    bool readAttr(const QXmlStreamAttributes &attr, const QString &name,
                  QString &output);

    bool string2Bool(const QString &str, bool &ok);

    QString bool2String(bool b);

private:
    bool m_isValid = false;
    QString m_name;
    QString m_version;
    QString m_author;
    QString m_comment;
    QMap<Styles, ColorScheme> m_colors;
};

#endif // SCRIPTEDITORTHEME_H
