/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#include "searchbar.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QIconEngine>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QSignalBlocker>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <algorithm>

namespace {
enum class SearchIcon { Options, Previous, Next, Close, Busy, Warning };

// One logical grid and stroke weight for every search control, independent of
// the differing canvas sizes and padding of theme SVGs.
class SearchIconEngine final : public QIconEngine {
public:
    SearchIconEngine(SearchIcon icon, const QPalette &palette)
        : m_icon(icon), m_palette(palette) {}
    QIconEngine *clone() const override {
        return new SearchIconEngine(m_icon, m_palette);
    }
    QPixmap pixmap(const QSize &size, QIcon::Mode mode,
                   QIcon::State state) override {
        QPixmap result(size);
        result.fill(Qt::transparent);
        QPainter painter(&result);
        paint(&painter, QRect(QPoint(), size), mode, state);
        return result;
    }
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode,
                         QIcon::State state, qreal scale) override {
        auto result = pixmap(size * scale, mode, state);
        result.setDevicePixelRatio(scale);
        return result;
    }
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
               QIcon::State) override {
        painter->save();
        const qreal side = std::min(rect.width(), rect.height());
        painter->translate(rect.x() + (rect.width() - side) / 2,
                           rect.y() + (rect.height() - side) / 2);
        painter->scale(side / 16, side / 16);
        painter->setRenderHint(QPainter::Antialiasing);
        const QColor color = m_palette.color(
            mode == QIcon::Disabled ? QPalette::Disabled : QPalette::Active,
            QPalette::Text);
        painter->setPen(
            QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        QPainterPath path;
        switch (m_icon) {
        case SearchIcon::Options:
            for (qreal y : {3., 8., 13.}) {
                path.moveTo(3, y);
                path.lineTo(13, y);
            }
            break;
        case SearchIcon::Previous:
            path.moveTo(8, 13);
            path.lineTo(8, 3);
            path.moveTo(3, 8);
            path.lineTo(8, 3);
            path.lineTo(13, 8);
            break;
        case SearchIcon::Next:
            path.moveTo(8, 3);
            path.lineTo(8, 13);
            path.moveTo(3, 8);
            path.lineTo(8, 13);
            path.lineTo(13, 8);
            break;
        case SearchIcon::Close:
            path.moveTo(3, 3);
            path.lineTo(13, 13);
            path.moveTo(13, 3);
            path.lineTo(3, 13);
            break;
        case SearchIcon::Busy:
            painter->drawArc(QRectF(3, 3, 10, 10), 30 * 16, 300 * 16);
            path.moveTo(10, 3);
            path.lineTo(13, 3);
            path.lineTo(13, 6);
            break;
        case SearchIcon::Warning:
            path.moveTo(8, 2.5);
            path.lineTo(13.5, 13);
            path.lineTo(2.5, 13);
            path.closeSubpath();
            path.moveTo(8, 6);
            path.lineTo(8, 9);
            painter->drawPoint(QPointF(8, 11));
            break;
        }
        painter->drawPath(path);
        painter->restore();
    }

private:
    SearchIcon m_icon;
    QPalette m_palette;
};

QIcon searchIcon(const QWidget *widget, SearchIcon icon) {
    return QIcon(new SearchIconEngine(icon, widget->palette()));
}
} // namespace

SearchBar::SearchBar(QWidget *parent, const QString &prefix) : QWidget(parent) {
    setObjectName(prefix + QStringLiteral("Bar"));
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(5);
    m_controls.input = new QLineEdit(this);
    m_controls.input->setObjectName(prefix + QStringLiteral("Input"));
    m_controls.input->setPlaceholderText(tr("Find..."));
    m_controls.input->setAccessibleName(tr("Search"));
    m_controls.input->setTextMargins(6, 3, 6, 3);
    m_controls.input->setClearButtonEnabled(true);
    m_controls.input->setMinimumWidth(40);
    m_controls.input->setStyleSheet(
        QStringLiteral("QLineEdit { border: 0px; padding: 0px; }"));
    m_controls.input->installEventFilter(this);
    setFocusProxy(m_controls.input);
    layout->addWidget(m_controls.input, 1);
    m_controls.status = new QLabel(this);
    m_controls.status->setObjectName(prefix + QStringLiteral("Status"));
    m_controls.status->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(m_controls.status, 0, Qt::AlignVCenter);
    m_controls.status->hide();
    const auto button = [this, layout, &prefix](const QString &name,
                                                const QString &text) {
        auto b = new QToolButton(this);
        b->setObjectName(prefix + name);
        b->setAutoRaise(true);
        b->installEventFilter(this);
        b->setToolButtonStyle(Qt::ToolButtonIconOnly);
        b->setToolTip(text);
        b->setAccessibleName(text);
        layout->addWidget(b, 0, Qt::AlignVCenter);
        m_controls.buttons.append(b);
        return b;
    };
    auto settings = button(QStringLiteral("Options"), tr("Options"));
    settings->setPopupMode(QToolButton::InstantPopup);
    settings->setStyleSheet(QStringLiteral(
        "QToolButton::menu-indicator { image: none; width: 0px; }"));
    m_controls.menu = new QMenu(settings);
    settings->setMenu(m_controls.menu);
    const auto option = [this](const QString &text, const QString &name,
                               bool checked) {
        auto action = m_controls.menu->addAction(text);
        action->setObjectName(name);
        action->setCheckable(true);
        action->setChecked(checked);
        connect(action, &QAction::toggled, this, &SearchBar::scheduleSearch);
        return action;
    };
    m_options.caseSensitive =
        option(tr("Match case"), QStringLiteral("matchCase"), false);
    m_options.wholeWord =
        option(tr("Whole words"), QStringLiteral("wholeWords"), false);
    m_options.regex = option(tr("Regular expressions"),
                             QStringLiteral("regularExpressions"), false);
    m_options.wrap =
        option(tr("Wrap around"), QStringLiteral("wrapAround"), true);
    connect(button(QStringLiteral("Previous"), tr("Find previous")),
            &QToolButton::clicked, this, [this] {
                cancelPendingSearch();
                Q_EMIT searchRequested(true);
            });
    connect(button(QStringLiteral("Next"), tr("Find next")),
            &QToolButton::clicked, this, [this] {
                cancelPendingSearch();
                Q_EMIT searchRequested(false);
            });
    connect(button(QStringLiteral("Close"), tr("Close")), &QToolButton::clicked,
            this, &SearchBar::closeRequested);
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(150);
    connect(&m_debounce, &QTimer::timeout, this, [this] {
        if (isVisible()) {
            Q_EMIT searchRequested(false);
        }
    });
    connect(m_controls.input, &QLineEdit::textChanged, this,
            &SearchBar::scheduleSearch);
    connect(m_controls.input, &QLineEdit::returnPressed, this, [this] {
        cancelPendingSearch();
        Q_EMIT searchRequested(
            QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier));
    });
    installEventFilter(this);
    refreshAppearance();
    hide();
}
QString SearchBar::searchText() const { return m_controls.input->text(); }
void SearchBar::setSearchText(const QString &text) {
    m_controls.input->setText(text);
}
QMenu *SearchBar::optionsMenu() const { return m_controls.menu; }
QLineEdit *SearchBar::lineEdit() const { return m_controls.input; }
SearchBar::Options SearchBar::options() const {
    return {m_options.caseSensitive->isChecked() ? Qt::CaseSensitive
                                                 : Qt::CaseInsensitive,
            m_options.wholeWord->isChecked(), m_options.regex->isChecked(),
            m_options.wrap->isChecked()};
}
void SearchBar::setOptions(const Options &options) {
    const QSignalBlocker blockCase(m_options.caseSensitive),
        blockWord(m_options.wholeWord), blockRegex(m_options.regex),
        blockWrap(m_options.wrap);
    m_options.caseSensitive->setChecked(options.caseSensitivity ==
                                        Qt::CaseSensitive);
    m_options.wholeWord->setChecked(options.wholeWords);
    m_options.regex->setChecked(options.regularExpression);
    m_options.wrap->setChecked(options.wrap);
    scheduleSearch();
}
SearchBar::State SearchBar::state() const { return m_state; }
QRegularExpression SearchBar::expression() const {
    const auto settings = options();
    auto text = settings.regularExpression
                    ? searchText()
                    : QRegularExpression::escape(searchText());
    if (settings.wholeWords) {
        text = QStringLiteral("\\b(?:") + text + QStringLiteral(")\\b");
    }
    QRegularExpression::PatternOptions flags =
        QRegularExpression::UseUnicodePropertiesOption;
    if (settings.caseSensitivity == Qt::CaseInsensitive) {
        flags |= QRegularExpression::CaseInsensitiveOption;
    }
    return QRegularExpression(text, flags);
}
SearchBar::Query SearchBar::query() const {
    return {searchText(), options(), expression()};
}
bool SearchBar::Query::matches(const QString &candidate) const {
    return options.regularExpression || options.wholeWords
               ? pattern.match(candidate).hasMatch()
               : candidate.contains(text, options.caseSensitivity);
}
void SearchBar::activate() {
    show();
    m_controls.input->setFocus();
    m_controls.input->selectAll();
}
void SearchBar::scheduleSearch() {
    Q_EMIT queryChanged();
    setState(State::Idle);
    m_debounce.start();
}
void SearchBar::cancelPendingSearch() { m_debounce.stop(); }
void SearchBar::setState(State state, const QString &detail) {
    m_state = state;
    m_detail = detail;
    updateStatus();
}
QIcon SearchBar::busyIcon(const QPalette &palette) {
    return QIcon(new SearchIconEngine(SearchIcon::Busy, palette));
}
void SearchBar::refreshAppearance() {
    const int extent =
        style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
    m_controls.status->setFixedSize(extent, extent);
    // Apply input metrics before measuring buttons and the overall bar.
    updateStatus();
    const int side =
        std::max(m_controls.input->sizeHint().height(), extent + 6);
    const SearchIcon icons[] = {SearchIcon::Options, SearchIcon::Previous,
                                SearchIcon::Next, SearchIcon::Close};
    Q_ASSERT(m_controls.buttons.size() == 4);
    for (int i = 0; i < m_controls.buttons.size(); ++i) {
        m_controls.buttons[i]->setIconSize(QSize(extent, extent));
        m_controls.buttons[i]->setFixedSize(side, side);
        m_controls.buttons[i]->setIcon(searchIcon(this, icons[i]));
    }
    Q_EMIT geometryChanged();
}
void SearchBar::updateStatus() {
    QString message;
    switch (m_state) {
    case State::Idle:
        break;
    case State::Searching:
        message = tr("Searching...");
        break;
    case State::NoMatch:
        message = tr("No matches");
        break;
    case State::InvalidPattern:
        message = tr("Invalid regular expression: %1").arg(m_detail);
        break;
    }
    if (m_state != State::Idle) {
        const auto icon = m_state == State::Searching ? SearchIcon::Busy
                                                      : SearchIcon::Warning;
        m_controls.status->setPixmap(
            searchIcon(m_controls.status, icon)
                .pixmap(m_controls.status->size(), devicePixelRatioF()));
    }
    m_controls.status->setToolTip(message);
    m_controls.status->setAccessibleName(message);
    m_controls.status->setVisible(m_state != State::Idle);
    m_controls.input->setToolTip(message);
    m_controls.input->setAccessibleDescription(message);
    const bool error =
        m_state == State::NoMatch || m_state == State::InvalidPattern;
    if (m_controls.input->property("searchError").toBool() != error) {
        m_controls.input->setProperty("searchError", error);
        // Dynamic selector properties require repolishing the affected widget.
        m_controls.input->style()->unpolish(m_controls.input);
        m_controls.input->style()->polish(m_controls.input);
        m_controls.input->update();
    }
}
bool SearchBar::handleKeyEvent(QEvent *event) {
    if (event->type() != QEvent::ShortcutOverride &&
        event->type() != QEvent::KeyPress) {
        return false;
    }
    const auto key = static_cast<QKeyEvent *>(event);
    const bool find = key->matches(QKeySequence::Find);
    const bool next = key->keyCombination() == QKeyCombination(Qt::Key_F3);
    const bool previous =
        key->keyCombination() == QKeyCombination(Qt::ShiftModifier, Qt::Key_F3);
    const bool close =
        key->keyCombination() == QKeyCombination(Qt::Key_Escape) && isVisible();
    if (!find && !next && !previous && !close) {
        return false;
    }
    event->accept();
    if (event->type() == QEvent::KeyPress) {
        cancelPendingSearch();
        if (close) {
            Q_EMIT closeRequested();
        } else {
            Q_EMIT showRequested();
            if (next || previous) {
                Q_EMIT searchRequested(previous);
            }
        }
    }
    return true;
}
void SearchBar::paintEvent(QPaintEvent *) {
    QStyleOption option;
    option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}
bool SearchBar::eventFilter(QObject *object, QEvent *event) {
    return handleKeyEvent(event) || QWidget::eventFilter(object, event);
}
void SearchBar::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::PaletteChange) {
        // Child styles are polished after the parent receives StyleChange.
        QTimer::singleShot(0, this, &SearchBar::refreshAppearance);
    }
}
void SearchBar::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    refreshAppearance();
}
void SearchBar::hideEvent(QHideEvent *event) {
    QWidget::hideEvent(event);
    cancelPendingSearch();
    Q_EMIT geometryChanged();
}
