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

// This file is part of QTextPad.

#include "settingspopup.h"

#include <QApplication>
#include <QHeaderView>
#include <QLineEdit>
#include <QPainter>
#include <QScrollBar>
#include <QTreeWidget>
#include <QVBoxLayout>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include <KSyntaxHighlighting/Repository>

#include "3rdparty/WingCodeEdit/wingcodeedit.h"
#include "utilities.h"

TreeFilterEdit::TreeFilterEdit(QWidget *parent) : QLineEdit(parent) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    const bool darkTheme =
        (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#else
    const bool darkTheme =
        (palette().color(QPalette::Window).lightness() < 128);
#endif

    // TODO
    // m_searchIcon =
    //(QStringLiteral("search-filter"),
    //                                             darkTheme);

    setClearButtonEnabled(true);
    recomputeIconPos();
}

QSize TreeFilterEdit::sizeHint() const {
    QSize hint = QLineEdit::sizeHint();
    hint.setHeight(qMin(hint.height(), 18));
    return hint;
}

void TreeFilterEdit::paintEvent(QPaintEvent *event) {
    QLineEdit::paintEvent(event);

    QPainter painter(this);
    QRect iconRect(m_iconPosition.x(), m_iconPosition.y(), 16, 16);
    if (event->region().intersects(iconRect))
        m_searchIcon.paint(&painter, iconRect);
}

void TreeFilterEdit::resizeEvent(QResizeEvent *event) {
    QLineEdit::resizeEvent(event);
    recomputeIconPos();
}

void TreeFilterEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Down) {
        emit navigateDown();
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void TreeFilterEdit::recomputeIconPos() {
    const int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QStringLiteral("QLineEdit { padding-left: %1px; }")
                      .arg(frameWidth + 18));

    m_iconPosition =
        QPoint(rect().left() + frameWidth + 1, (rect().bottom() - 16) / 2);
}

static QTreeWidgetItem *firstVisibleItem(QTreeWidgetItem *parent) {
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        if (!item->isHidden())
            return item;
    }
    return nullptr;
}

FilteredTreePopup::FilteredTreePopup(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_filter = new TreeFilterEdit(this);
    connect(m_filter, &QLineEdit::textChanged, this,
            &FilteredTreePopup::filterItems);
    layout->addWidget(m_filter);

    m_tree = new QTreeWidget(this);
    m_tree->header()->hide();
    layout->addWidget(m_tree);

    connect(m_filter, &TreeFilterEdit::navigateDown, this, [this] {
        m_tree->setFocus();
        auto topItem = firstVisibleItem(m_tree->invisibleRootItem());
        if (topItem)
            m_tree->setCurrentItem(topItem);
    });
}

QSize FilteredTreePopup::sizeHint() const {
    QSize hint;
    const int viewWidth =
        5 + m_tree->columnWidth(0) +
        m_tree->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) +
        m_tree->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    hint.setWidth(qMax(viewWidth, m_tree->sizeHint().width()));
    const QFontMetrics fm = m_tree->fontMetrics();
    hint.setHeight(m_filter->sizeHint().height() + (fm.height() * 16));
    return hint;
}

TreeFilterEdit *FilteredTreePopup::filter() { return m_filter; }

QTreeWidget *FilteredTreePopup::tree() { return m_tree; }

template <typename FilterProc>
bool applyFilter(QTreeWidgetItem *parent, const FilterProc &proc) {
    bool showParent = false;
    for (int i = 0; i < parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        if (applyFilter(item, proc)) {
            item->setHidden(false);
            showParent = true;
            continue;
        }

        if (proc(item)) {
            item->setHidden(false);
            showParent = true;
        } else {
            item->setHidden(true);
        }
    }
    return showParent;
}

void FilteredTreePopup::filterItems(const QString &text) {
    if (text.isEmpty()) {
        applyFilter(m_tree->invisibleRootItem(),
                    [](QTreeWidgetItem *) { return true; });
    } else {
        applyFilter(m_tree->invisibleRootItem(), [text](QTreeWidgetItem *item) {
            return item->text(0).contains(text, Qt::CaseInsensitive);
        });
    }
}

void FilteredTreePopup::showEvent(QShowEvent *e) {
    m_filter->setFocus();
    QWidget::showEvent(e);
}

bool FilteredTreePopup::focusNextPrevChild(bool) {
    // The default Qt tab navigation doesn't seem to work correctly
    // in a popup widget
    if (m_filter->hasFocus())
        m_tree->setFocus();
    else
        m_filter->setFocus();
    return true;
}

SyntaxPopup::SyntaxPopup(QWidget *parent) : FilteredTreePopup(parent) {
    connect(tree(), &QTreeWidget::itemActivated, this,
            &SyntaxPopup::syntaxItemChosen);
    connect(tree(), &QTreeWidget::itemClicked, this,
            &SyntaxPopup::syntaxItemChosen);

    // Default unformatted option
    m_plainTextItem =
        new QTreeWidgetItem(tree(), QStringList{tr("Plain Text")});

    // Load the syntax definitions (these are already sorted)
    auto &syntaxRepo = WingCodeEdit::syntaxRepo();
    const auto syntaxDefs = syntaxRepo.definitions();
    QMap<QString, QTreeWidgetItem *> groupItems;
    for (const auto &def : syntaxDefs) {
        if (def.isHidden() || def == WingCodeEdit::nullSyntax())
            continue;

        QTreeWidgetItem *parentItem =
            groupItems.value(def.translatedSection(), Q_NULLPTR);
        if (!parentItem) {
            parentItem = new QTreeWidgetItem(
                tree(), QStringList{def.translatedSection()});
            groupItems[def.translatedSection()] = parentItem;
        }
        auto item =
            new QTreeWidgetItem(parentItem, QStringList{def.translatedName()});
        item->setData(0, Qt::UserRole, QVariant::fromValue(def));
    }
    tree()->expandAll();
    tree()->resizeColumnToContents(0);
}

void SyntaxPopup::syntaxItemChosen(QTreeWidgetItem *current, int) {
    if (!current)
        return;

    if (current == m_plainTextItem) {
        emit syntaxSelected(WingCodeEdit::nullSyntax());
        return;
    }

    QVariant itemData = current->data(0, Qt::UserRole);
    if (itemData.canConvert<KSyntaxHighlighting::Definition>())
        emit syntaxSelected(itemData.value<KSyntaxHighlighting::Definition>());
}

EncodingPopup::EncodingPopup(QWidget *parent) : FilteredTreePopup(parent) {
    connect(tree(), &QTreeWidget::itemActivated, this,
            &EncodingPopup::encodingItemChosen);
    connect(tree(), &QTreeWidget::itemClicked, this,
            &EncodingPopup::encodingItemChosen);

    auto encodings = Utilities::getEncodings();

    for (auto &e : encodings) {
        auto item = new QTreeWidgetItem(tree(), QStringList{e});
        item->setData(0, Qt::UserRole, e);
    }

    tree()->resizeColumnToContents(0);
}

void EncodingPopup::encodingItemChosen(QTreeWidgetItem *current, int) {
    if (!current)
        return;

    QVariant codecName = current->data(0, Qt::UserRole);
    if (codecName.isValid())
        emit encodingSelected(codecName.toString());
}
