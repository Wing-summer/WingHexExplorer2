#ifndef FRAMELESSWINDOWBASE_H
#define FRAMELESSWINDOWBASE_H

#include "src/control/titlebar.h"
#include "src/framelesswindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class FramelessWindowBase : public FramelessWindow<QDialog> {
    Q_OBJECT

    Q_PROPERTY(QColor titlecolor READ titleColor WRITE setTitleColor NOTIFY
                   titleColorChanged FINAL)
public:
    explicit FramelessWindowBase(QWidget *parent = nullptr);

    virtual ~FramelessWindowBase();

    QColor titleColor() const;
    void setTitleColor(const QColor &newTitleColor);

    int titleHeight() const;

signals:
    void titleColorChanged();

private:
    void buildUp();

protected:
    void buildUpContent(TitleBar::TitleItems title, QWidget *content);

    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;

    bool onTitleMovingBegin(const QPoint &pt, bool &needProcess) override;
    void onTitleMoving(const QPoint &pt, bool needProcess) override;

    virtual void onDealedWithBorder(bool needHide);

#ifdef Q_OS_WIN
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    virtual bool nativeEvent(const QByteArray &eventType, void *message,
                             qintptr *result) override {
#else
    virtual bool nativeEvent(const QByteArray &eventType, void *message,
                             long *result) override {
#endif
        MSG *msg = static_cast<MSG *>(message);

        // TODO: ADD Add Aero Snap Support ?
        // but I don't want to implement such thing...
        return QDialog::nativeEvent(eventType, message, result);
    }
#endif

protected:
    TitleBar *m_titlebar = nullptr;
    QWidget *WIN_WIDGET = nullptr;
    QColor m_titleColor = Qt::white;

private:
#ifndef Q_OS_WIN
    bool m_btmlock = false;
#endif

    bool m_isBuilt = false;

    // FramelessWindow interface
};

#endif // FRAMELESSMAINWINDOW_H
