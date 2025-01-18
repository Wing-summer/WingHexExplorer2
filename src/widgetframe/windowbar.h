// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#ifndef WINDOWBAR_H
#define WINDOWBAR_H

#include <QAbstractButton>
#include <QFrame>
#include <QLabel>
#include <QMenuBar>

namespace QWK {

class WindowBarPrivate;

class WindowBar : public QLabel {
    Q_OBJECT
    Q_DECLARE_PRIVATE(WindowBar)
public:
    explicit WindowBar(QWidget *parent = nullptr);
    ~WindowBar();

public:
    QAbstractButton *iconButton() const;
    QAbstractButton *minButton() const;
    QAbstractButton *maxButton() const;
    QAbstractButton *closeButton() const;

    QWidget *holder() const;

    void setTitleLabel(QLabel *label);
    void setIconButton(QAbstractButton *btn);
    void setMinButton(QAbstractButton *btn);
    void setMaxButton(QAbstractButton *btn);
    void setCloseButton(QAbstractButton *btn);

    QAbstractButton *takeIconButton();
    QAbstractButton *takeMinButton();
    QAbstractButton *takeMaxButton();
    QAbstractButton *takeCloseButton();

    QWidget *hostWidget() const;
    void setHostWidget(QWidget *w);

    bool titleFollowWindow() const;
    void setTitleFollowWindow(bool value);

    bool iconFollowWindow() const;
    void setIconFollowWindow(bool value);

Q_SIGNALS:
    void minimizeRequested();
    void maximizeRequested(bool max = false);
    void closeRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    WindowBar(WindowBarPrivate &d, QWidget *parent = nullptr);

    QScopedPointer<WindowBarPrivate> d_ptr;
};

} // namespace QWK

#endif // WINDOWBAR_H
