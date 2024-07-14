#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QToolButton>

class TitleBar : public QWidget {
    Q_OBJECT

public:
    enum TitleItem : uint {
        TITLE_NONE = 0,
        TITLE_ICON = 1,
        TITLE_MINBUTTON = 2,
        TITLE_MAXBUTTON = 4,
        TITLE_CLOSEBUTTON = 8,
        TITLE_ALL =
            TITLE_ICON | TITLE_MINBUTTON | TITLE_MAXBUTTON | TITLE_CLOSEBUTTON,
        TITLE_DIALOG = TITLE_CLOSEBUTTON
    };
    Q_DECLARE_FLAGS(TitleItems, TitleItem)

public:
    explicit TitleBar(const QString &title, TitleItems type,
                      QWidget *parent = nullptr);

    virtual ~TitleBar();

    QLabel *titleHolder() const;

    QString title() const;

    int titleHeight() const;

    QColor titleColor() const;

    QIcon windowIcon() const;

    QMenu *titleMenu() const;

    bool testIsMaximized() const;

public slots:
    void setTitle(const QString &title) const;

    void setWindowIcon(const QIcon &icon);

    void setTitleMenu(QMenu *menu);

    void setIsMaximizedFlag(bool isMaximized);

signals:
    void titleCloseButtonClicked();
    void titleMaxButtonClicked(bool needMax);
    void titleMinButtonClicked();

    void titleColorChanged();

private:
    QToolButton *m_icon;

    QPushButton *m_btnClose;
    QPushButton *m_btnMin;
    QPushButton *m_btnMax;

    QLabel *m_lblTitle;
};

#endif // TITLEBAR_H
