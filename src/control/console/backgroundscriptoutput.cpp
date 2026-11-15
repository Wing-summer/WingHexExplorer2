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

#include "backgroundscriptoutput.h"

#include <algorithm>

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <QScrollBar>
#include <QShowEvent>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include "class/settings/thememanager.h"
#include "dialog/common/wingmessagebox.h"
#include "dialog/framelessdialogbase.h"

namespace {
class ElidedLabel final : public QLabel {
public:
    explicit ElidedLabel(QWidget *parent) : QLabel(parent) {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }

    virtual QSize minimumSizeHint() const override {
        return {0, QLabel::minimumSizeHint().height()};
    }

    virtual void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        style()->drawItemText(&painter, contentsRect(), alignment(), palette(),
                              isEnabled(),
                              fontMetrics().elidedText(text(), Qt::ElideRight,
                                                       contentsRect().width()),
                              QPalette::WindowText);
    }
};

class ScriptListDelegate final : public QStyledItemDelegate {
public:
    explicit ScriptListDelegate(QObject *parent)
        : QStyledItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override {
        QStyleOptionViewItem styled(option);
        initStyleOption(&styled, index);
        const auto name = styled.text;
        styled.text.clear();
        auto *style = styled.widget->style();
        style->drawControl(QStyle::CE_ItemViewItem, &styled, painter,
                           styled.widget);
        const auto rect = style
                              ->subElementRect(QStyle::SE_ItemViewItemText,
                                               &styled, styled.widget)
                              .adjusted(4, 5, -6, -5);
        painter->save();
        const auto color = option.state & QStyle::State_Selected
                               ? QPalette::HighlightedText
                               : QPalette::Text;
        painter->setPen(option.palette.color(color));
        auto font = option.font;
        font.setBold(true);
        painter->setFont(font);
        const QFontMetrics titleMetrics(font);
        painter->drawText(
            rect, Qt::AlignTop | Qt::AlignLeft,
            titleMetrics.elidedText(name, Qt::ElideRight, rect.width()));
        font = option.font;
        font.setPointSizeF(std::max(8.0, font.pointSizeF() - 1.0));
        painter->setFont(font);
        const QFontMetrics detailsMetrics(font);
        const auto detailRect =
            rect.adjusted(0, titleMetrics.height() + 3, 0, 0);
        painter->drawText(
            detailRect, Qt::AlignTop | Qt::AlignLeft,
            detailsMetrics.elidedText(index.data(Qt::UserRole).toString(),
                                      Qt::ElideRight, detailRect.width()));
        painter->restore();
    }

    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &) const override {
        return {140, option.fontMetrics.height() * 2 + 16};
    }
};
} // namespace

BackgroundScriptOutput::BackgroundScriptOutput(QWidget *parent)
    : QWidget(parent) {
    setObjectName(QStringLiteral("backgroundScriptOutput"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout->addWidget(m_splitter);
    m_sidebar = new QWidget(m_splitter);
    m_sidebar->setMinimumWidth(100);
    auto *sideLayout = new QVBoxLayout(m_sidebar);
    sideLayout->setContentsMargins(6, 4, 6, 4);
    auto *heading = new QHBoxLayout;
    auto *label = new ElidedLabel(m_sidebar);
    label->setText(tr("Background scripts"));
    auto font = label->font();
    font.setBold(true);
    label->setFont(font);
    label->setMinimumWidth(0);
    heading->addWidget(label);
    m_count = new QLabel(m_sidebar);
    heading->addWidget(m_count);
    sideLayout->addLayout(heading);
    m_list = new QListWidget(m_sidebar);
    m_list->setObjectName(QStringLiteral("backgroundScriptList"));
    m_list->setAccessibleName(tr("Background scripts"));
    m_list->setItemDelegate(new ScriptListDelegate(m_list));
    m_list->setIconSize(QSize(16, 16));
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    sideLayout->addWidget(m_list);
    connect(m_list, &QListWidget::currentRowChanged, this,
            &BackgroundScriptOutput::selectPage);

    auto *content = new QWidget(m_splitter);
    content->setMinimumWidth(100);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    auto *toolbar = new QToolBar(content);
    toolbar->setIconSize(QSize(16, 16));
    m_title = new ElidedLabel(toolbar);
    m_title->setMinimumWidth(0);
    toolbar->addWidget(m_title);
    m_status = new QLabel(toolbar);
    toolbar->addWidget(m_status);
    toolbar->addSeparator();
    auto &theme = ThemeManager::instance();
    m_clear = toolbar->addAction(
        theme.standardIcon(ThemeManager::StandardIcon::Delete), tr("Clear"),
        this, &BackgroundScriptOutput::clearCurrent);
    m_stop = toolbar->addAction(
        theme.standardIcon(ThemeManager::StandardIcon::DebugStop),
        tr("Terminate script"), this, &BackgroundScriptOutput::stopCurrent);
    m_close = toolbar->addAction(
        theme.standardIcon(ThemeManager::StandardIcon::Close),
        tr("Close output"), this, &BackgroundScriptOutput::closeCurrent);
    m_stop->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    m_stop->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(m_stop);
    contentLayout->addWidget(toolbar);
    m_stack = new QStackedWidget(content);
    auto *empty = new QLabel(tr("Run a script to view its output"), m_stack);
    empty->setAlignment(Qt::AlignCenter);
    m_stack->addWidget(empty);
    contentLayout->addWidget(m_stack);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({200, 600});
    auto &machine = ScriptMachine::instance();
    connect(&machine, &ScriptMachine::sigOnBackgroundTaskStateChanged, this,
            &BackgroundScriptOutput::taskStateChanged);
    connect(&machine, &ScriptMachine::sigOnBackgroundTasksChanged, this,
            &BackgroundScriptOutput::refreshCount);
    refreshCount();
    refreshActions();
}

BackgroundScriptOutput::~BackgroundScriptOutput() = default;

QString BackgroundScriptOutput::stateText(ScriptExecution::State state) {
    switch (state) {
    case ScriptExecution::State::Pending:
        return tr("Starting");
    case ScriptExecution::State::Running:
        return tr("Running");
    case ScriptExecution::State::WaitingInput:
        return tr("Waiting for input");
    case ScriptExecution::State::Resident:
        return tr("Resident");
    case ScriptExecution::State::Stopping:
        return tr("Stopping");
    case ScriptExecution::State::Success:
        return tr("Succeeded");
    case ScriptExecution::State::Failed:
        return tr("Failed");
    case ScriptExecution::State::Aborted:
        return tr("Terminated");
    }
    Q_UNREACHABLE();
}

BackgroundScriptOutput::Page *BackgroundScriptOutput::currentPage() const {
    for (const auto &page : m_pages) {
        if (page->item == m_list->currentItem()) {
            return page.get();
        }
    }
    return nullptr;
}

BackgroundScriptOutput::Page *
BackgroundScriptOutput::findPage(quint64 taskId) const {
    for (const auto &page : m_pages) {
        if (page->taskId == taskId) {
            return page.get();
        }
    }
    return nullptr;
}

void BackgroundScriptOutput::runScript(const QString &path) {
    const QFileInfo info(path);
    const auto canonical = info.canonicalFilePath();
    const auto identity =
        canonical.isEmpty() ? info.absoluteFilePath() : canonical;
    Page *target = nullptr;
    for (const auto &page : m_pages) {
        if (page->path == identity) {
            target = page.get();
            break;
        }
    }
    if (target && target->active()) {
        m_list->setCurrentItem(target->item);
        selectPage();
        return;
    }
    auto &machine = ScriptMachine::instance();
    const auto created = machine.createBackgroundExecution(
        identity, ScriptExecution::Lifetime::OneShot);
    if (!created.execution) {
        WingMessageBox::warning(
            this, tr("Run script"),
            created.result == ScriptMachine::ExecutionResult::Capacity
                ? tr("All 16 background script environments are in use. Stop a "
                     "script before starting another.")
                : tr("The script engine is unavailable."));
        return;
    }
    if (!target) {
        if (m_pages.size() >= ScriptMachine::MaxBackgroundTasks) {
            Page *oldest = nullptr;
            for (const auto &page : m_pages) {
                if (!page->active() &&
                    (!oldest || page->lastUsed < oldest->lastUsed)) {
                    oldest = page.get();
                }
            }
            Q_ASSERT(oldest);
            removePage(oldest);
        }
        auto page = std::make_unique<Page>();
        target = page.get();
        target->path = identity;
        target->item = new QListWidgetItem(m_list);
        target->output = new QPlainTextEdit(m_stack);
        target->output->setReadOnly(true);
        target->output->setAccessibleName(tr("Script output"));
        target->output->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(target->output, &QPlainTextEdit::customContextMenuRequested,
                this, &BackgroundScriptOutput::showOutputMenu);
        m_stack->addWidget(target->output);
        m_pages.push_back(std::move(page));
    }
    target->taskId = created.execution->taskId();
    target->state = ScriptExecution::State::Pending;
    target->lastOutputType = ScriptMachine::MessageType::Unknown;
    target->output->clear();
    target->unread = false;
    target->lastUsed = ++m_useCounter;
    refreshPage(*target);
    m_list->setCurrentItem(target->item);
    selectPage();
    auto *execution = created.execution;
    connect(execution, &ScriptExecution::sigOnExecutionCompleted, execution,
            &QObject::deleteLater);
    connect(execution, &ScriptExecution::sigOnExecutionStopped, execution,
            &QObject::deleteLater);
    connect(execution, &ScriptExecution::sigOnExecutionFailed, execution,
            &QObject::deleteLater);
    QTimer::singleShot(0, execution, &ScriptExecution::start);
}

void BackgroundScriptOutput::refreshPage(Page &page) {
    const QFileInfo info(page.path);
    page.item->setText((page.unread ? QStringLiteral("• ") : QString()) +
                       info.fileName());
    const auto state = stateText(page.state);
    const QString details =
        state + QStringLiteral(" · ") + info.dir().dirName();
    page.item->setData(Qt::UserRole, details);
    page.item->setToolTip(page.path + QLatin1Char('\n') + state);
    const QString accessible = info.fileName() + QStringLiteral(", ") + state;
    page.item->setData(Qt::AccessibleTextRole, accessible);
    const auto icon = page.state == ScriptExecution::State::Success
                          ? QStyle::SP_DialogApplyButton
                      : page.state == ScriptExecution::State::Failed
                          ? QStyle::SP_MessageBoxCritical
                      : page.state == ScriptExecution::State::WaitingInput
                          ? QStyle::SP_MessageBoxQuestion
                      : page.active() ? QStyle::SP_MediaPlay
                                      : QStyle::SP_MediaStop;
    page.item->setIcon(style()->standardIcon(icon));
}

void BackgroundScriptOutput::selectPage() {
    if (auto *page = currentPage()) {
        page->lastUsed = ++m_useCounter;
        if (isVisible()) {
            page->unread = false;
        }
        m_stack->setCurrentWidget(page->output);
        refreshPage(*page);
    } else {
        m_stack->setCurrentIndex(0);
    }
    refreshActions();
}

void BackgroundScriptOutput::refreshActions() {
    const auto *page = currentPage();
    m_clear->setEnabled(page);
    m_stop->setEnabled(page && page->active() &&
                       page->state != ScriptExecution::State::Stopping);
    m_close->setEnabled(page && !page->active());
    m_title->setText(page ? QFileInfo(page->path).fileName() : QString());
    m_title->setToolTip(page ? page->path : QString());
    m_status->setText(page ? stateText(page->state) : QString());
}

void BackgroundScriptOutput::refreshCount() {
    m_count->setText(QStringLiteral("%1/%2")
                         .arg(ScriptMachine::instance().activeBackgroundTasks())
                         .arg(ScriptMachine::MaxBackgroundTasks));
    m_count->setAccessibleName(tr("Active script environments"));
}

void BackgroundScriptOutput::clearCurrent() {
    if (auto *page = currentPage()) {
        page->output->clear();
        page->lastOutputType = ScriptMachine::MessageType::Unknown;
    }
}

void BackgroundScriptOutput::stopCurrent() {
    if (auto *page = currentPage()) {
        ScriptMachine::instance().abortBackgroundTask(page->taskId);
    }
}

void BackgroundScriptOutput::closeCurrent() {
    if (auto *page = currentPage(); page && !page->active()) {
        removePage(page);
    }
}

void BackgroundScriptOutput::removePage(Page *page) {
    if (!page) {
        return;
    }
    const auto found =
        std::find_if(m_pages.begin(), m_pages.end(),
                     [page](const auto &entry) { return entry.get() == page; });
    auto *item = page->item;
    auto *output = page->output;
    m_pages.erase(found);
    delete item;
    m_stack->removeWidget(output);
    delete output;
    selectPage();
}

void BackgroundScriptOutput::taskStateChanged(quint64 taskId,
                                              ScriptExecution::State state) {
    if (auto *page = findPage(taskId)) {
        page->state = state;
        refreshPage(*page);
        refreshActions();
    }
}

void BackgroundScriptOutput::showOutputMenu(const QPoint &position) {
    auto *page = currentPage();
    if (!page) {
        return;
    }
    std::unique_ptr<QMenu> menu(page->output->createStandardContextMenu());
    menu->addSeparator();
    menu->addAction(m_clear);
    menu->addAction(m_stop);
    menu->addAction(m_close);
    menu->exec(page->output->mapToGlobal(position));
}

void BackgroundScriptOutput::appendOutput(
    const ScriptMachine::MessageInfo &message) {
    auto *page = findPage(message.taskId);
    if (!page) {
        return;
    }
    auto *output = page->output;
    auto *scroll = output->verticalScrollBar();
    const bool follow = scroll->value() == scroll->maximum();
    const auto oldScroll = scroll->value();
    auto cursor = QTextCursor(output->document());
    cursor.movePosition(QTextCursor::End);
    const bool hasText = cursor.block().length() > 1;
    const auto type = message.type;
    QTextCharFormat format;
    QString text = message.message;
    if (type != ScriptMachine::MessageType::Print && message.row > 0 &&
        message.col > 0) {
        text.prepend(
            QStringLiteral("(%1, %2) ").arg(message.row).arg(message.col));
    }
    if (type == ScriptMachine::MessageType::Unknown) {
        return;
    }
    if (hasText &&
        (type != ScriptMachine::MessageType::Print ||
         (page->lastOutputType != type &&
          page->lastOutputType != ScriptMachine::MessageType::Unknown))) {
        cursor.insertBlock();
    }
    if (type == ScriptMachine::MessageType::Error) {
        format.setForeground(Qt::red);
        text.prepend(tr("[Error]"));
    } else if (type == ScriptMachine::MessageType::Warn) {
        format.setForeground(QColorConstants::Svg::gold);
        text.prepend(tr("[Warn]"));
    } else if (type != ScriptMachine::MessageType::Print) {
        text.prepend(tr("[Info]"));
    }
    cursor.insertText(text, format);
    page->lastOutputType = type;
    scroll->setValue(follow ? scroll->maximum() : oldScroll);
    if (currentPage() != page || !isVisible()) {
        page->unread = true;
        refreshPage(*page);
        Q_EMIT sigOnUnreadOutputReceived();
    }
}

QString BackgroundScriptOutput::requestInput(quint64 taskId) {
    auto *page = findPage(taskId);
    if (!page) {
        return {};
    }
    auto *input = new QInputDialog;
    input->setLabelText(
        tr("Enter a value for %1:").arg(QFileInfo(page->path).fileName()));
    input->setWindowFlag(Qt::Widget);
    FramelessDialogBase dialog(this);
    dialog.buildUpContent(input);
    dialog.setWindowTitle(tr("Input request"));
    connect(input, &QInputDialog::finished, &dialog, &QDialog::done);
    connect(&ScriptMachine::instance(),
            &ScriptMachine::sigOnBackgroundInputCancelled, &dialog,
            [&dialog, taskId](quint64 cancelled) {
                if (cancelled == taskId) {
                    dialog.reject();
                }
            });
    return dialog.exec() == QDialog::Accepted ? input->textValue() : QString();
}

void BackgroundScriptOutput::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    selectPage();
}

void BackgroundScriptOutput::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    m_sidebar->setMaximumWidth(std::max(100, std::min(360, width() / 2)));
}
