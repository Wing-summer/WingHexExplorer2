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

#ifndef TOAST_H
#define TOAST_H

#include <QBasicTimer>
#include <QColor>
#include <QDialog>
#include <QElapsedTimer>
#include <QFont>
#include <QPixmap>
#include <QPointer>
#include <QSize>
#include <QString>
#include <QTextLayout>

class Toast : public QDialog {
    Q_OBJECT

    Q_PROPERTY(QColor backcolor READ backColor WRITE setBackColor)
    Q_PROPERTY(QColor textcolor READ textColor WRITE setTextColor)

public:
    enum ToastPos { Top, Center, Bottom };

    static void toast(QWidget *parent, const QPixmap &icon,
                      const QString &strContent);

    virtual ~Toast() override;

    void setToastPos(ToastPos pos);
    ToastPos lastToastPos() const;

    QPixmap icon() const;
    void setIcon(const QPixmap &icon);

    QColor backColor() const;
    void setBackColor(const QColor &backColor);

    QColor textColor() const;
    void setTextColor(const QColor &textColor);

protected:
    QFont displayFont() const;
    QSize calculateTextSize();
    void init();

    virtual void changeEvent(QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString m_strContent;
    QTextLayout m_textLayout;
    QBasicTimer m_timer;
    QElapsedTimer m_elapsed;
    qint64 m_activeTime = 0;
    qreal m_opacity = 1.0;
    QColor m_textColor = Qt::white;
    QColor m_backColor = Qt::black;
    QPixmap m_icon;

    QPointer<QWidget> m_parent;

    ToastPos m_pos = ToastPos::Bottom;
    bool m_pauseTimer = false;
    bool m_dismissing = false;

    void dismiss();

    Toast(const QString &strContent, const QPixmap &icon, QWidget *parent);
};

#endif // TOAST_H
