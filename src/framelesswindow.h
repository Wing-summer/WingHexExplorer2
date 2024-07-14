/*******************************************************************************
 * Copyright (C) 2020 - 2023, winsoft666, <winsoft666@outlook.com>.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
 * EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Expect bugs
 *
 * Please use and enjoy. Please let me know of any bugs/improvements
 * that you have found/implemented and I will fix/incorporate them into this
 * file.
 *******************************************************************************/

#ifndef FRAMELESS_WINDOW_HPP_
#define FRAMELESS_WINDOW_HPP_

#include <QApplication>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QMainWindow>
#include <QMouseEvent>
#include <QScreen>
#include <QString>
#include <QWidget>
#include <QWindow>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#endif

// modified by wingsummer for more feature
// and bug fix with QT6 support

template <typename T>
class FramelessWindow : public T {
public:
    enum class Direction {
        UP = 0,
        DOWN = 1,
        LEFT,
        RIGHT,
        LEFTTOP,
        LEFTBOTTOM,
        RIGHTBOTTOM,
        RIGHTTOP,
        NONE
    };

    enum class WindowState { Normal, Maximized, Minimized, FullScreen };

    explicit FramelessWindow(bool translucentBackground,
                             QWidget *parent = Q_NULLPTR,
                             bool ignoreReturn = false)
        : T(parent), m_ignoreReturn(ignoreReturn), m_bLeftPressed(false),
          m_bResizeable(false), m_bUseSystemMove(true),
          m_bCurUseSystemMove(false), m_bNearToScreenEdge(false),
          m_bDisableTitlebarDoubleClick(false), m_Direction(Direction::NONE),
          m_iResizeRegionPadding(4) {
        m_iResizeRegionPadding =
            m_iResizeRegionPadding * this->devicePixelRatioF();
        this->installEventFilter(this);

        Qt::WindowFlags flags = this->windowFlags();
        this->setWindowFlags(flags | Qt::FramelessWindowHint |
                             Qt::WindowSystemMenuHint);

        if (translucentBackground) {
            this->setAttribute(Qt::WA_TranslucentBackground);
        }
    }

    ~FramelessWindow() = default;

public:
    void setTitlebar(QVector<QWidget *> titleBar) {
        m_titlebarWidget = titleBar;
    }

    void setResizeable(bool b, const QMargins &transparentMargins) {
        m_bResizeable = b;
        m_transparentMargsins = transparentMargins;
    }

    bool resizeable() const { return m_bResizeable; }

    void setResizeRegionPadding(int padding) {
        m_iResizeRegionPadding = padding * this->devicePixelRatioF();
    }

    int resizeRegionPadding() const {
        return m_iResizeRegionPadding / this->devicePixelRatioF();
    }

    void setNearToScreenEdge(bool yes) { m_bNearToScreenEdge = yes; }

    bool nearToScreenEdge() const { return m_bNearToScreenEdge; }

    QMargins transparentMargins() const { return m_transparentMargsins; }

    bool useSystemMove() const { return m_bUseSystemMove; }

    void setUseSystemMove(bool b) { m_bUseSystemMove = b; }

    bool titlebarDoubleClickDisabled() const {
        return m_bDisableTitlebarDoubleClick;
    }

    void setDisableTitlebarDoubleClicked(bool b) {
        m_bDisableTitlebarDoubleClick = b;
    }

    void initMouseTracking() { setAllWidgetMouseTracking(this); }

    void setAllWidgetMouseTracking(QWidget *widget) {
        if (!widget)
            return;

        widget->setMouseTracking(true);
        QObjectList list = widget->children();
        foreach (QObject *obj, list) {
            QWidget *w = qobject_cast<QWidget *>(obj);
            if (w) {
                setAllWidgetMouseTracking(w);
            }
        }
    }

public slots:
    void show(WindowState state = WindowState::Normal) {
        switch (state) {
        case WindowState::Normal:
            this->setContentsMargins(m_transparentMargsins);
            this->showNormal();
            break;
        case WindowState::Maximized:
            this->setContentsMargins(0, 0, 0, 0);
            this->showMaximized();
            break;
        case WindowState::Minimized:
            this->setContentsMargins(m_transparentMargsins);
            this->showMinimized();
            break;
        case WindowState::FullScreen:
            this->setContentsMargins(0, 0, 0, 0);
            this->showFullScreen();
            break;
        }
    }

protected:
    virtual bool eventFilter(QObject *target, QEvent *event) {
        if (event->type() == QEvent::KeyPress) {
            if (m_ignoreReturn) {
                QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
                if (keyEvent->key() == Qt::Key_Enter ||
                    keyEvent->key() == Qt::Key_Return)
                    return true; // mark the event as handled
            }
        } else if (event->type() == QEvent::Move) {
            if (m_bLeftPressed) {
                onManualMoving(this->geometry().topLeft());
            }

            if (m_bNearToScreenEdge) {
                const int NEAR_SPACE = 20;
                QRect curGeo = this->geometry();

                bool isNearEdge = false;
                QPoint nearToPoint;
                QList<QScreen *> screens = QGuiApplication::screens();
                for (int i = 0; i < screens.size(); i++) {
                    QRect geo = screens[i]->geometry();
                    if (qAbs(geo.left() - curGeo.left()) <= NEAR_SPACE) {
                        isNearEdge = true;
                        nearToPoint = QPoint(geo.left(), curGeo.top());
                        break;
                    } else if (qAbs(geo.top() - curGeo.top()) <= NEAR_SPACE) {
                        isNearEdge = true;
                        nearToPoint = QPoint(curGeo.left(), geo.top());
                        break;
                    } else if (qAbs(geo.right() - curGeo.right()) <=
                               NEAR_SPACE) {
                        isNearEdge = true;
                        nearToPoint =
                            QPoint(geo.right() - curGeo.width(), curGeo.top());
                        break;
                    } else if (qAbs(geo.bottom() - curGeo.bottom()) <=
                               NEAR_SPACE) {
                        isNearEdge = true;
                        nearToPoint = QPoint(curGeo.left(),
                                             geo.bottom() - curGeo.height());
                        break;
                    }
                }

                if (isNearEdge) {
                    this->move(nearToPoint);
                    return true;
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            handleMouseMoveEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            handleMousePressEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            handleMouseReleaseEvent(mouseEvent);
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            handleMouseDoubleClickEvent(mouseEvent);
        }
        return T::eventFilter(target, event);
    }

    void handleMouseDoubleClickEvent(QMouseEvent *event) {
        if (m_bResizeable && !m_bDisableTitlebarDoubleClick) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QPoint globalPoint = event->globalPosition().toPoint();
#else
            QPoint globalPoint = event->globalPos();
#endif
            QWidget *actionWidget = QApplication::widgetAt(globalPoint);
            if (actionWidget) {
                bool inTitlebar = false;
                for (auto &item : m_titlebarWidget) {
                    if (actionWidget == item) {
                        inTitlebar = true;
                        break;
                    }
                }
                if (inTitlebar) {
                    if (this->isMaximized() || this->isFullScreen()) {
                        this->show(WindowState::Normal);
                    } else {
                        this->show(WindowState::Maximized);
                    }
                }
            }
        }
    }

    void handleMousePressEvent(QMouseEvent *event) {
        m_MousePressedGeo = this->geometry();
        if (event->button() == Qt::LeftButton) {
            if (m_Direction != Direction::NONE) {
                m_bLeftPressed = true;
                this->grabMouse();
            } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QPoint globalPoint = event->globalPosition().toPoint();
#else
                QPoint globalPoint = event->globalPos();
#endif
                QWidget *action = QApplication::widgetAt(globalPoint);
                if (action) {
                    bool inTitlebar = false;
                    for (auto &item : m_titlebarWidget) {
                        if (action == item) {
                            inTitlebar = true;
                            break;
                        }
                    }
                    if (inTitlebar) {
                        m_DragPos =
                            globalPoint - this->frameGeometry().topLeft();
                        auto useSystem = onTitleMovingBegin(
                            m_DragPos, m_needProcessTitleMoving);
                        m_bLeftPressed = true;
                        if (m_bUseSystemMove && useSystem) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                            m_bCurUseSystemMove =
                                this->windowHandle()->startSystemMove();

                            Q_ASSERT_X(m_bCurUseSystemMove, "mousePressEvent()",
                                       "this->windowHandle()->startSystemMove()"
                                       " failed");
#endif
                        }
                    }
                }
            }
        }
    }

    void handleMouseMoveEvent(QMouseEvent *event) {
#ifdef Q_OS_WIN
        // Windows 对鼠标单击时无法及时触发 Release
        auto isLBtnDown = GetKeyState(VK_LBUTTON) & 128;
        if (!isLBtnDown && m_bLeftPressed) {
            qApp->sendPostedEvents(this, QMouseEvent::MouseButtonRelease);
            event->ignore();
            return;
        }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QPoint globalPoint = event->globalPosition().toPoint();
#else
        QPoint globalPoint = event->globalPos();
#endif
        if (m_bLeftPressed) {
            bool bIgnore = true;
            QList<QScreen *> screens = QGuiApplication::screens();
            for (int i = 0; i < screens.size(); i++) {
                QScreen *pScreen = screens[i];
                QRect geometryRect = pScreen->availableGeometry();
                if (geometryRect.contains(globalPoint)) {
                    bIgnore = false;
                    break;
                }
            }

            if (bIgnore) {
                event->ignore();
                return;
            }

            if (m_Direction != Direction::NONE) {
                QRect rect = this->rect();
                QPoint tl = this->mapToGlobal(rect.topLeft());
                QPoint rb = this->mapToGlobal(rect.bottomRight());

                QRect rMove(tl, rb);

                switch (m_Direction) {
                case Direction::LEFT: {
                    if (rb.x() - globalPoint.x() <= this->minimumWidth())
                        rMove.setX(tl.x());
                    else
                        rMove.setX(globalPoint.x() -
                                   m_transparentMargsins.left());
                } break;
                case Direction::RIGHT:
                    rMove.setWidth(globalPoint.x() - tl.x() +
                                   m_transparentMargsins.right());
                    break;
                case Direction::UP:
                    if (rb.y() - globalPoint.y() <= this->minimumHeight())
                        rMove.setY(tl.y());
                    else
                        rMove.setY(globalPoint.y() -
                                   m_transparentMargsins.top());
                    break;
                case Direction::DOWN:
                    rMove.setHeight(globalPoint.y() - tl.y() +
                                    m_transparentMargsins.bottom());
                    break;
                case Direction::LEFTTOP:
                    if (rb.x() - globalPoint.x() <= this->minimumWidth())
                        rMove.setX(tl.x());
                    else
                        rMove.setX(globalPoint.x() -
                                   m_transparentMargsins.left());
                    if (rb.y() - globalPoint.y() <= this->minimumHeight())
                        rMove.setY(tl.y());
                    else
                        rMove.setY(globalPoint.y() -
                                   m_transparentMargsins.top());
                    break;
                case Direction::RIGHTTOP:
                    if (rb.y() - globalPoint.y() <= this->minimumHeight())
                        rMove.setY(tl.y());
                    else
                        rMove.setY(globalPoint.y() -
                                   m_transparentMargsins.top());
                    rMove.setWidth(globalPoint.x() - tl.x() +
                                   m_transparentMargsins.right());
                    break;
                case Direction::LEFTBOTTOM:
                    if (rb.x() - globalPoint.x() <= this->minimumWidth())
                        rMove.setX(tl.x());
                    else
                        rMove.setX(globalPoint.x() -
                                   m_transparentMargsins.left());
                    rMove.setHeight(globalPoint.y() - tl.y() +
                                    m_transparentMargsins.bottom());
                    break;
                case Direction::RIGHTBOTTOM:
                    rMove.setWidth(globalPoint.x() - tl.x() +
                                   m_transparentMargsins.right());
                    rMove.setHeight(globalPoint.y() - tl.y() +
                                    m_transparentMargsins.bottom());
                    break;
                default:
                    break;
                }

                this->setGeometry(rMove);
                onManualResizing(rMove);

                this->update();
                for (auto &w : m_titlebarWidget) {
                    w->update();
                }
            } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                QPoint globalPoint = event->globalPosition().toPoint();
#else
                QPoint globalPoint = event->globalPos();
#endif
                QPoint newPoint = globalPoint - m_DragPos;

                onTitleMoving(m_DragPos, m_needProcessTitleMoving);

                if (!m_bCurUseSystemMove) {
                    this->move(newPoint);
                }
                event->accept();
            }
        } else {
            region(globalPoint);
        }
    }

    void region(const QPoint &cursorGlobalPoint) {
        if (!m_bResizeable || this->isMaximized() || this->isFullScreen())
            return;

        QRect rect = this->contentsRect();

        QPoint tl = this->mapToGlobal(rect.topLeft());
        QPoint rb = this->mapToGlobal(rect.bottomRight());
        int x = cursorGlobalPoint.x();
        int y = cursorGlobalPoint.y();

        if (tl.x() + m_iResizeRegionPadding >= x && tl.x() <= x &&
            tl.y() + m_iResizeRegionPadding >= y && tl.y() <= y) {
            m_Direction = Direction::LEFTTOP;
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
        } else if (x >= rb.x() - m_iResizeRegionPadding && x <= rb.x() &&
                   y >= rb.y() - m_iResizeRegionPadding && y <= rb.y()) {
            m_Direction = Direction::RIGHTBOTTOM;
            this->setCursor(QCursor(Qt::SizeFDiagCursor));
        } else if (x <= tl.x() + m_iResizeRegionPadding && x >= tl.x() &&
                   y >= rb.y() - m_iResizeRegionPadding && y <= rb.y()) {
            m_Direction = Direction::LEFTBOTTOM;
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
        } else if (x <= rb.x() && x >= rb.x() - m_iResizeRegionPadding &&
                   y >= tl.y() && y <= tl.y() + m_iResizeRegionPadding) {
            m_Direction = Direction::RIGHTTOP;
            this->setCursor(QCursor(Qt::SizeBDiagCursor));
        } else if (x <= tl.x() + m_iResizeRegionPadding && x >= tl.x()) {
            m_Direction = Direction::LEFT;
            this->setCursor(QCursor(Qt::SizeHorCursor));
        } else if (x <= rb.x() && x >= rb.x() - m_iResizeRegionPadding) {
            m_Direction = Direction::RIGHT;
            this->setCursor(QCursor(Qt::SizeHorCursor));
        } else if (y >= tl.y() && y <= tl.y() + m_iResizeRegionPadding) {
            m_Direction = Direction::UP;
            this->setCursor(QCursor(Qt::SizeVerCursor));
        } else if (y <= rb.y() && y >= rb.y() - m_iResizeRegionPadding) {
            m_Direction = Direction::DOWN;
            this->setCursor(QCursor(Qt::SizeVerCursor));
        } else {
            m_Direction = Direction::NONE;
            this->setCursor(QCursor(Qt::ArrowCursor));
        }
    }

    void handleMouseReleaseEvent(QMouseEvent *event) {
        Q_UNUSED(event);
        m_bLeftPressed = false;
        if (m_Direction != Direction::NONE) {
            m_Direction = Direction::NONE;
            this->releaseMouse();
            this->setCursor(QCursor(Qt::ArrowCursor));
        }

        QRect curGeo = this->geometry();
        if (m_MousePressedGeo != curGeo) {
            onManualMoved(curGeo.topLeft());
        }
        if (m_MousePressedGeo.size() != curGeo.size()) {
            onManualResized(this->rect());
        }
    }

    virtual bool onTitleMovingBegin(const QPoint &pt, bool &needProcess) {
        Q_UNUSED(pt);
        needProcess = true;
        return true;
    }

    virtual void onTitleMoving(const QPoint &pt, bool needProcess) {
        Q_UNUSED(pt);
        Q_UNUSED(needProcess);
    }

    virtual void onManualResizing(const QRect &rc) { Q_UNUSED(rc); }
    virtual void onManualResized(const QRect &rc) { Q_UNUSED(rc); }
    virtual void onManualMoving(const QPoint &pt) { Q_UNUSED(pt); }
    virtual void onManualMoved(const QPoint &pt) { Q_UNUSED(pt); }

protected:
    const bool m_ignoreReturn;
    bool m_needProcessTitleMoving;
    bool m_bLeftPressed;
    bool m_bResizeable;
    bool m_bUseSystemMove;
    bool m_bCurUseSystemMove;
    bool m_bNearToScreenEdge;
    bool m_bDisableTitlebarDoubleClick;
    Direction m_Direction;
    int m_iResizeRegionPadding;
    QPoint m_DragPos;
    QRect m_MousePressedGeo;
    QMargins m_transparentMargsins;
    QVector<QWidget *> m_titlebarWidget;
};

#endif
