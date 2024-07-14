#include "titlebar.h"
#include <QApplication>
#include <QStyle>

TitleBar::TitleBar(const QString &title, TitleItems type, QWidget *parent)
    : QWidget(parent) {
    auto TITLE_HEIGHT = std::max(
        30, QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight));

    auto layout = new QHBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 1, 10, 1);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    layout->setSpacing(1);

    m_icon = new QToolButton(this);
    m_icon->setIconSize(QSize(20, 20));
    m_icon->setObjectName(QStringLiteral("WIN_TITLE_ICON"));
    m_icon->setVisible(type.testFlag(TITLE_ICON));

    m_btnMin = new QPushButton(this);
    m_btnMin->setObjectName(QStringLiteral("WIN_BTN_MIN"));
    m_btnMin->setFixedSize(25, TITLE_HEIGHT);
    m_btnMin->setIconSize(QSize(10, 10));
    m_btnMin->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnMin->setVisible(type.testFlag(TITLE_MINBUTTON));

    m_btnMax = new QPushButton(this);
    m_btnMax->setObjectName(QStringLiteral("WIN_BTN_MAX"));
    m_btnMax->setFixedSize(25, TITLE_HEIGHT);
    m_btnMax->setIconSize(QSize(10, 10));
    m_btnMax->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnMax->setVisible(type.testFlag(TITLE_MAXBUTTON));
    setIsMaximizedFlag(false);

    m_btnClose = new QPushButton(this);
    m_btnClose->setObjectName(QStringLiteral("WIN_BTN_CLOSE"));
    m_btnClose->setFixedSize(25, TITLE_HEIGHT);
    m_btnClose->setIconSize(QSize(10, 10));
    m_btnClose->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnClose->setVisible(type.testFlag(TITLE_CLOSEBUTTON));

    m_lblTitle = new QLabel(title, this);
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setObjectName(QStringLiteral("WIN_TITLE_NAME"));
    m_lblTitle->setFixedHeight(TITLE_HEIGHT);
    m_lblTitle->setStyleSheet(QStringLiteral("background:transparent;"));

    layout->addWidget(m_icon);
    layout->addWidget(m_lblTitle);
    layout->addWidget(m_btnMin);
    layout->addWidget(m_btnMax);
    layout->addWidget(m_btnClose);

    connect(m_btnClose, &QPushButton::clicked, this,
            &TitleBar::titleCloseButtonClicked);
    connect(m_btnMax, &QPushButton::clicked, this, [this] {
        auto ismax = m_btnMax->property("titlemax").toBool();
        this->setIsMaximizedFlag(!ismax);
        emit this->titleMaxButtonClicked(!ismax);
    });
    connect(m_btnMin, &QPushButton::clicked, this,
            &TitleBar::titleMinButtonClicked);
}

TitleBar::~TitleBar() {}

QString TitleBar::title() const { return m_lblTitle->text(); }

void TitleBar::setTitle(const QString &title) const {
    m_lblTitle->setText(title);
}

int TitleBar::titleHeight() const { return m_lblTitle->height(); }

void TitleBar::setWindowIcon(const QIcon &icon) { m_icon->setIcon(icon); }

void TitleBar::setTitleMenu(QMenu *menu) { m_icon->setMenu(menu); }

void TitleBar::setIsMaximizedFlag(bool isMaximized) {
    m_btnMax->setProperty("titlemax", isMaximized);
}

QIcon TitleBar::windowIcon() const { return m_icon->icon(); }

QMenu *TitleBar::titleMenu() const { return m_icon->menu(); }

bool TitleBar::testIsMaximized() const {
    return m_btnMax->property("titlemax").toBool();
}

QLabel *TitleBar::titleHolder() const { return m_lblTitle; }
