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

#include "diffdialog.h"

#include <limits>

#include <QButtonGroup>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QProgressBar>
#include <QScrollBar>
#include <QSet>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbontabcontent.h"

#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "control/diffrenderer.h"
#include "control/documentchoice.h"
#include "control/editorview.h"
#include "control/qtableviewext.h"
#include "control/searchbar.h"
#include "model/diffresultmodel.h"

namespace {

using Icon = ThemeManager::StandardIcon;
constexpr int RefreshIntervalMs = 100;
constexpr int RecompareDelayMs = 500;
constexpr int ProgressScale = 1000;
constexpr int DifferenceRowHeight = 24;
constexpr int DefaultViewPaneHeight = 600;
constexpr int DefaultDifferenceListHeight = 220;
constexpr quint64 PreviewByteLimit = 16;

QString errorText(WingDiff::Error error) {
    switch (error) {
    case WingDiff::Error::None:
        return {};
    case WingDiff::Error::FileLimitExceeded:
        return DiffDialog::tr("File size limit exceeded");
    case WingDiff::Error::SourceChanged:
        return DiffDialog::tr("Source changed — compare again");
    case WingDiff::Error::ResultBudgetExceeded:
        return DiffDialog::tr("Result memory limit reached");
    case WingDiff::Error::OpenFailed:
        return DiffDialog::tr("Cannot open comparison source");
    case WingDiff::Error::SeekFailed:
        return DiffDialog::tr("Cannot seek comparison source");
    case WingDiff::Error::ReadFailed:
        return DiffDialog::tr("Cannot read comparison source");
    default:
        return DiffDialog::tr("Invalid comparison source");
    }
}

QString readErrorText(QHexReadError error) {
    return error == QHexReadError::SourceChanged
               ? DiffDialog::tr("Source changed — compare again")
               : DiffDialog::tr("Cannot open comparison source");
}

} // namespace

DiffDialog::DiffDialog(EditorView *initial, QWidget *parent)
    : FramelessMainWindow(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Compare"));
    setWindowIcon(ThemeManager::instance().standardIcon(Icon::Compare));
    resize(1250, 850);

    auto content = new QWidget(this);
    auto layout = new QVBoxLayout(content);
    layout->setContentsMargins(1, 0, 1, 0);
    buildUpRibbonBar();
    buildViews();
    buildDifferenceList();
    buildStatusBar();

    layout->addWidget(m_ribbon);
    layout->addWidget(m_contentSplitter, 1);
    buildUpContent(content);

    m_contentSplitter->setSizes(
        {DefaultViewPaneHeight, DefaultDifferenceListHeight});
    m_viewsSplitter->setSizes({1, 1});
    reloadSettings();
    connect(&SettingManager::instance(),
            &SettingManager::sigOnDiffSettingsChanged, this,
            &DiffDialog::reloadSettings);
    refreshCandidates();
    if (initial) {
        for (int index = 0; index < m_candidates.size(); ++index) {
            if (m_candidates[index]->hexEditor()->document() ==
                initial->hexEditor()->document()) {
                m_choices[Side::Left]->setCurrentIndex(index);
                break;
            }
        }
    }
    if (m_choices[Side::Left]->currentIndex() < 0 && !m_candidates.isEmpty()) {
        m_choices[Side::Left]->setCurrentIndex(0);
    }
    for (int candidate = 0; candidate < m_candidates.size(); ++candidate) {
        if (m_candidates[candidate]->hexEditor()->document() !=
            m_views[Side::Left]->document()) {
            m_choices[Side::Right]->setCurrentIndex(candidate);
            break;
        }
    }
    bindSource(Side::Left);
    bindSource(Side::Right);
    m_finalStatus = usable() ? tr("Ready to compare")
                             : tr("Choose two different open documents");
    m_recompareTimer.setSingleShot(true);
    m_recompareTimer.setInterval(RecompareDelayMs);
    connect(&m_recompareTimer, &QTimer::timeout, this,
            &DiffDialog::tryRecompare);
    connect(&m_timer, &QTimer::timeout, this, &DiffDialog::tick);
    m_timer.start(RefreshIntervalMs);
    connect(&m_worker, &QFutureWatcher<WingDiff::Outcome>::finished, this,
            &DiffDialog::comparisonFinished);
    connect(&ThemeManager::instance(), &ThemeManager::sigOnThemeChanged, this,
            &DiffDialog::applyTheme);
    applyTheme();
    updateControls();
}

void DiffDialog::comparisonFinished() {
    m_running = false;
    m_durationMs = m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
    const auto outcome = m_worker.result();
    if (!m_invalid || m_phase == Phase::Cancelling) {
        switch (outcome.status) {
        case WingDiff::Status::Complete:
            m_phase = Phase::Complete;
            m_finalStatus = tr("Complete");
            break;
        case WingDiff::Status::Cancelled:
            m_phase = Phase::Cancelled;
            m_finalStatus = tr("Cancelled — partial results");
            break;
        case WingDiff::Status::Failed:
            m_phase = Phase::Failed;
            m_finalStatus = errorText(outcome.error);
            break;
        }
    }
    if (m_results) {
        m_model->refresh();
    }
    for (auto view : m_views) {
        view->viewport()->update();
    }
    tick();
    if (m_recomparePending && !m_recompareTimer.isActive()) {
        m_recompareTimer.start(0);
    }
}

void DiffDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    buildComparePage();
    buildViewPage();
}

void DiffDialog::buildComparePage() {
    auto tab = m_ribbon->addTab(tr("Compare"));
    auto run = tab->addGroup(tr("Comparison"));
    m_startButton =
        addPannelAction(run, tr("Compare / refresh"), Icon::Compare,
                        &DiffDialog::start, false, QKeySequence(Qt::Key_F5));
    m_startButton->setObjectName(QStringLiteral("diff.compare"));
    m_cancelButton = addPannelAction(run, tr("Cancel"), Icon::CompareStop,
                                     &DiffDialog::cancel, false,
                                     QKeySequence(Qt::Key_Escape));
    m_autoCompareButton =
        addPannelAction(run, tr("Auto compare"), Icon::Refresh,
                        &DiffDialog::setAutoCompare, true);
    m_autoCompareButton->setChecked(true);
    m_autoCompareButton->setToolTip(
        tr("Compare automatically after source changes"));
    m_cancelButton->setToolTip(
        tr("Cancel comparison and pause automatic comparison"));
    m_swapButton = addPannelAction(run, tr("Swap sides"), Icon::CompareSwap,
                                   &DiffDialog::swapSides);
    auto nav = tab->addGroup(tr("Differences"));
    m_navigation = {addPannelAction(nav, tr("First"), Icon::FirstDifference,
                                    &DiffDialog::firstDifference),
                    addPannelAction(nav, tr("Previous"), Icon::ArrowLeft,
                                    &DiffDialog::previousDifference, false,
                                    QKeySequence(Qt::SHIFT | Qt::Key_F7)),
                    addPannelAction(nav, tr("Next"), Icon::ArrowRight,
                                    &DiffDialog::nextDifference, false,
                                    QKeySequence(Qt::Key_F7)),
                    addPannelAction(nav, tr("Last"), Icon::LastDifference,
                                    &DiffDialog::lastDifference)};
    addPannelAction(nav, tr("Search"), Icon::Search, &DiffDialog::showSearch);

    addPannelAction(tab->addGroup(tr("Resources")), tr("Settings"),
                    Icon::Settings, &DiffDialog::sigOnSettingsRequested);
}

void DiffDialog::buildViewPage() {
    auto tab = m_ribbon->addTab(tr("View"));
    auto arrangement = tab->addGroup(tr("Layout"));
    m_arrangementButtons[PaneLayout::SideBySide] = addPannelAction(
        arrangement, tr("Side by side"), Icon::CompareHorizontal,
        &DiffDialog::showSideBySide, true);
    m_arrangementButtons[PaneLayout::Stacked] =
        addPannelAction(arrangement, tr("Stacked"), Icon::CompareVertical,
                        &DiffDialog::showStacked, true);
    auto layouts = new QButtonGroup(this);
    for (auto button : m_arrangementButtons) {
        layouts->addButton(button);
    }
    m_syncButton =
        addPannelAction(arrangement, tr("Sync scrolling"), Icon::SyncScroll,
                        &DiffDialog::setSyncScrolling, true);
    m_listButton =
        addPannelAction(arrangement, tr("Difference list"), Icon::Structure,
                        &DiffDialog::setListVisible, true);
    m_listButton->setChecked(true);
    addPannelAction(arrangement, tr("Reset layout"), Icon::Layout,
                    &DiffDialog::resetLayout);
    auto display = tab->addGroup(tr("Display"));
    addPannelAction(display, tr("ASCII"), Icon::String,
                    &DiffDialog::setAsciiVisible, true)
        ->setChecked(true);
    addPannelAction(display, tr("Addresses"), Icon::Address,
                    &DiffDialog::setAddressesVisible, true)
        ->setChecked(true);
}

void DiffDialog::swapSides() {
    const auto first = m_choices[Side::Left]->currentIndex();
    const auto second = m_choices[Side::Right]->currentIndex();
    m_choices[Side::Left]->setCurrentIndex(second);
    m_choices[Side::Right]->setCurrentIndex(first);
}

void DiffDialog::firstDifference() { selectDifference(0); }

void DiffDialog::previousDifference() { navigate(-1); }

void DiffDialog::nextDifference() { navigate(1); }

void DiffDialog::lastDifference() {
    if (m_results && m_results->publishedDifferences()) {
        selectDifference(m_results->publishedDifferences() - 1);
    }
}

void DiffDialog::showSideBySide() { setOrientation(false); }

void DiffDialog::showStacked() { setOrientation(true); }

void DiffDialog::setSyncScrolling(bool enabled) {
    SettingManager::instance().setDiffSyncScroll(enabled);
}

void DiffDialog::setListVisible(bool visible) {
    m_listPanel->setVisible(visible);
}

void DiffDialog::resetLayout() {
    showSideBySide();
    m_viewsSplitter->setSizes({1, 1});
    m_contentSplitter->setSizes(
        {DefaultViewPaneHeight, DefaultDifferenceListHeight});
    m_listPanel->show();
    m_listButton->setChecked(true);
}

void DiffDialog::setAsciiVisible(bool visible) {
    for (auto view : m_views) {
        view->setAsciiVisible(visible);
    }
}

void DiffDialog::setAddressesVisible(bool visible) {
    for (auto view : m_views) {
        view->setAddressVisible(visible);
    }
}

void DiffDialog::buildViews() {
    m_contentSplitter = new QSplitter(Qt::Vertical, this);
    m_viewsSplitter = new QSplitter(m_contentSplitter);
    m_viewsSplitter->setObjectName(QStringLiteral("diff.views"));
    for (const auto side : {Side::Left, Side::Right}) {
        auto panel = new QWidget(m_viewsSplitter);
        auto panelLayout = new QVBoxLayout(panel);
        panelLayout->setContentsMargins(0, 0, 0, 0);
        auto row = new QHBoxLayout;
        row->addWidget(
            new QLabel(side == Side::Right ? tr("Right") : tr("Left"), panel));
        auto choice = new DocumentChoice(panel);
        connect(choice, &DocumentChoice::sigOnAboutToShowPopup, this,
                &DiffDialog::refreshCandidates);
        choice->setObjectName(side == Side::Right
                                  ? QStringLiteral("diff.right.source")
                                  : QStringLiteral("diff.left.source"));
        choice->setAccessibleName(side == Side::Right ? tr("Right document")
                                                      : tr("Left document"));
        m_choices[side] = choice;
        row->addWidget(choice, 1);
        panelLayout->addLayout(row);
        m_views[side] = new QHexView(panel);
        m_views[side]->setReadOnly(true);
        m_views[side]->setRenderer(new DiffRenderer(
            m_views[side]->document().data(), m_views[side]->cursor(),
            QFontMetricsF(m_views[side]->font()), m_views[side],
            side == Side::Right));
        m_views[side]->setObjectName(side == Side::Right
                                         ? QStringLiteral("diff.right.view")
                                         : QStringLiteral("diff.left.view"));
        m_views[side]->setAccessibleName(
            side == Side::Right ? tr("Right document") : tr("Left document"));
        panelLayout->addWidget(m_views[side], 1);
        connect(choice, &QComboBox::currentIndexChanged, this,
                [this, side] { bindSource(side); });
        connect(m_views[side]->verticalScrollBar(), &QScrollBar::valueChanged,
                this, [this, side] { scrollTogether(side); });
    }
}

void DiffDialog::buildDifferenceList() {
    m_listPanel = new QWidget(m_contentSplitter);
    auto listLayout = new QVBoxLayout(m_listPanel);
    listLayout->setContentsMargins(0, 0, 0, 0);
    m_countLabel = new QLabel(tr("Differences"), m_listPanel);
    listLayout->addWidget(m_countLabel);
    m_search = new SearchBar(m_listPanel, QStringLiteral("diffSearch"));
    m_search->hide();
    m_table = new QTableViewExt(m_listPanel);
    m_table->setSearchEnabled(false);
    m_table->setAsyncSortingEnabled(false);
    m_table->setSortingEnabled(false);
    m_table->setObjectName(QStringLiteral("diff.results"));
    m_table->setAccessibleName(tr("Difference list"));
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setDefaultSectionSize(DifferenceRowHeight);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_model = new DiffResultModel(this);
    m_table->setModel(m_model);
    for (int column = 0; column < DiffResultModel::ColumnCount; ++column) {
        m_table->setColumnSortable(column, false);
        QTableViewExt::ColumnWidthPolicy policy;
        policy.source = QTableViewExt::WidthSource::Weight;
        policy.update = QTableViewExt::WidthUpdate::Continuous;
        const bool offset = column == DiffResultModel::LeftOffset ||
                            column == DiffResultModel::RightOffset;
        policy.weight = offset ? 2 : 1;
        policy.minimumPixels =
            m_table->fontMetrics().horizontalAdvance(
                m_model->headerData(column, Qt::Horizontal, Qt::DisplayRole)
                    .toString()) +
            32;
        m_table->setColumnWidthPolicy(column, policy);
    }
    listLayout->addWidget(m_table, 1);
    listLayout->addWidget(m_search);
    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &DiffDialog::selectCurrentDifference);
    connect(m_search, &SearchBar::sigOnSearchRequested, this,
            &DiffDialog::search);
    connect(m_search, &SearchBar::sigOnQueryChanged, this,
            &DiffDialog::cancelSearch);
    connect(m_search, &SearchBar::sigOnCloseRequested, this,
            &DiffDialog::closeSearch);
    connect(new QShortcut(QKeySequence::Find, this), &QShortcut::activated,
            this, &DiffDialog::showSearch);
}

void DiffDialog::selectCurrentDifference(const QModelIndex &index) {
    const auto source = m_table->mapToSource(index);
    if (!source.isValid()) {
        return;
    }
    m_selected = m_model->globalRow(source.row());
    locateRange(m_model->range(source.row()));
}

void DiffDialog::cancelSearch() {
    if (m_searchCancel) {
        *m_searchCancel = true;
    }
    m_search->setState(SearchBar::State::Idle);
}

void DiffDialog::closeSearch() {
    cancelSearch();
    m_search->hide();
}

void DiffDialog::buildStatusBar() {
    auto statusBar = new QStatusBar(this);
    m_status = new QLabel(statusBar);
    m_status->setObjectName(QStringLiteral("diff.status"));
    m_status->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_progress = new QProgressBar(statusBar);
    m_progress->setObjectName(QStringLiteral("diff.progress"));
    m_progress->hide();
    m_progress->setTextVisible(false);
    m_progress->setRange(0, ProgressScale);
    m_progress->setMaximumWidth(200);
    statusBar->addWidget(m_status, 1);
    statusBar->addPermanentWidget(m_progress);
    setStatusBar(statusBar);
}

void DiffDialog::reloadSettings() {
    const auto &manager = SettingManager::instance();
    m_viewsSplitter->setOrientation(manager.diffVertical() ? Qt::Vertical
                                                           : Qt::Horizontal);
    m_arrangementButtons[PaneLayout::SideBySide]->setChecked(
        !manager.diffVertical());
    m_arrangementButtons[PaneLayout::Stacked]->setChecked(
        manager.diffVertical());
    m_syncButton->setChecked(manager.diffSyncScroll());
}

DiffDialog::~DiffDialog() {
    if (m_cancel) {
        *m_cancel = true;
    }
    if (m_searchCancel) {
        *m_searchCancel = true;
    }
}

void DiffDialog::refreshCandidates() {
    QVector<QPointer<EditorView>> candidates;
    QSet<QHexDocument *> seen;
    const auto &editors = EditorView::instances();
    for (auto editor : editors) {
        if (!editor ||
            editor->documentType() == EditorView::DocumentType::InValid ||
            !editor->hexEditor() || !editor->hexEditor()->document()) {
            continue;
        }
        if (editor->isCloneFile()) {
            const auto origin = editor->cloneParent();
            if (!editors.contains(origin) ||
                origin->documentType() == EditorView::DocumentType::InValid ||
                !origin->hexEditor() ||
                origin->hexEditor()->document() !=
                    editor->hexEditor()->document()) {
                continue;
            }
        }
        auto document = editor->hexEditor()->document().data();
        if (seen.contains(document)) {
            continue;
        }
        seen.insert(document);
        candidates.append(editor);
    }
    m_candidates = std::move(candidates);
    for (const auto side : {Side::Left, Side::Right}) {
        QSignalBlocker blocker(m_choices[side]);
        m_choices[side]->clear();
        for (const auto &editor : std::as_const(m_candidates)) {
            m_choices[side]->addItem(editor->infoFileName());
            m_choices[side]->setItemData(m_choices[side]->count() - 1,
                                         editor->infoTooltip(),
                                         Qt::ToolTipRole);
        }
        const auto previous = m_candidates.indexOf(m_editors[side]);
        m_choices[side]->setCurrentIndex(previous >= 0 ? previous : -1);
    }
    updateControls();
}

void DiffDialog::bindSource(Side side) {
    const auto index = m_choices[side]->currentIndex();
    const auto editor = index >= 0 && index < m_candidates.size()
                            ? m_candidates[index]
                            : QPointer<EditorView>();
    if (m_editors[side] == editor) {
        updateControls();
        return;
    }
    invalidate(tr("Sources changed — compare again"));
    for (const auto &connection : std::as_const(m_sourceConnections[side])) {
        disconnect(connection);
    }
    m_sourceConnections[side].clear();
    m_editors[side] = editor;
    if (editor) {
        auto document = editor->hexEditor()->document();
        m_views[side]->setDocument(document, m_views[side]->cursor());
        m_sourceConnections[side].append(
            connect(document.data(), &QHexDocument::sigOnContentsChanged, this,
                    &DiffDialog::sourceContentChanged));
        m_sourceConnections[side].append(
            connect(document.data(), &QHexDocument::sigOnDocumentSaved, this,
                    &DiffDialog::sourceSaved));
        m_sourceConnections[side].append(
            connect(editor.data(), &QObject::destroyed, this, [this, side] {
                invalidate(tr("Source closed — choose another document"));
                for (const auto &connection :
                     std::as_const(m_sourceConnections[side])) {
                    disconnect(connection);
                }
                m_sourceConnections[side].clear();
                m_editors[side] = nullptr;
                m_views[side]->resetDocument();
                m_choices[side]->setCurrentIndex(-1);
                updateControls();
            }));
    } else {
        m_views[side]->resetDocument();
    }
    applyTheme();
    queueComparison();
    updateControls();
}

void DiffDialog::sourceContentChanged() {
    requestComparison(tr("Source content changed"));
}

void DiffDialog::sourceSaved(bool saved) {
    if (!saved || !m_results) {
        return;
    }
    // Saved bytes are unchanged; only refresh independent read handles.
    if (m_running || !recaptureSources()) {
        requestComparison(tr("Source file changed"));
    }
}

bool DiffDialog::recaptureSources() {
    WingDiff::Source sources[SideCount];
    for (const auto side : {Side::Left, Side::Right}) {
        auto captured = WingDiff::Source::capture(
            m_views[side]->document().data(), m_options.memoryLimit / 16);
        if (!captured.source.valid()) {
            return false;
        }
        sources[side] = std::move(captured.source);
    }
    for (const auto side : {Side::Left, Side::Right}) {
        m_sources[side] = std::move(sources[side]);
    }
    return true;
}

bool DiffDialog::usable() const {
    return m_editors[Side::Left] && m_editors[Side::Right] &&
           m_views[Side::Left]->document() != m_views[Side::Right]->document();
}

void DiffDialog::invalidate(const QString &reason) {
    m_recompareTimer.stop();
    m_recomparePending = false;
    m_phase = m_hasCompared ? Phase::Stale : Phase::Idle;
    if (m_cancel) {
        *m_cancel = true;
    }
    if (m_searchCancel) {
        *m_searchCancel = true;
    }
    ++m_generation;
    m_invalid = true;
    m_results.reset();
    m_model->setStore({});
    applyTheme();
    m_sources[Side::Left] = {};
    m_sources[Side::Right] = {};
    m_finalStatus = reason;
    for (auto view : m_views) {
        if (view) {
            view->viewport()->update();
        }
    }
    updateControls();
}

void DiffDialog::start() {
    if (m_running || !usable()) {
        return;
    }
    invalidate({});
    m_hasCompared = true;
    const auto &manager = SettingManager::instance();
    m_options.fileLimit = manager.diffFileLimit();
    m_options.memoryLimit = manager.diffMemoryLimit();
    m_options.detailLimit = manager.diffDetailLimit();
    m_options.precise = manager.diffPrecise();
    m_options.normalize();
    for (const auto side : {Side::Left, Side::Right}) {
        if (quint64(m_views[side]->document()->length()) >
            m_options.fileLimit) {
            m_phase = Phase::Failed;
            m_finalStatus = tr("%1 exceeds the configured file size limit")
                                .arg(side == Side::Right ? tr("Right document")
                                                         : tr("Left document"));
            tick();
            return;
        }
        auto captured = WingDiff::Source::capture(
            m_views[side]->document().data(), m_options.memoryLimit / 16);
        m_sources[side] = std::move(captured.source);
        if (!m_sources[side].valid()) {
            m_phase = Phase::Failed;
            m_finalStatus = readErrorText(captured.error);
            tick();
            return;
        }
    }
    m_results = std::make_shared<WingDiff::ResultStore>(
        m_options.memoryLimit * 3 / 8 - WingDiff::MiB);
    m_model->setStore(m_results);
    applyTheme();
    m_selected = 0;
    m_lastCount = 0;
    m_cancel = std::make_shared<std::atomic_bool>(false);
    const auto cancelToken = m_cancel;
    const auto a = m_sources[Side::Left], b = m_sources[Side::Right];
    const auto results = m_results;
    const auto options = m_options;
    m_invalid = false;
    m_running = true;
    m_phase = Phase::Comparing;
    m_elapsed.start();
    updateControls();
    m_worker.setFuture(QtConcurrent::run([a, b, results, options, cancelToken] {
        auto left = a.open(cancelToken), right = b.open(cancelToken);
        if (!left.device || !right.device) {
            WingDiff::Outcome failure;
            failure.error = left.error == QHexReadError::SourceChanged ||
                                    right.error == QHexReadError::SourceChanged
                                ? WingDiff::Error::SourceChanged
                                : WingDiff::Error::OpenFailed;
            if (cancelToken->load()) {
                failure.status = WingDiff::Status::Cancelled;
                failure.error = WingDiff::Error::None;
            }
            return failure;
        }
        return WingDiff::BinaryDiff().compare(
            left.device.get(), right.device.get(), options, *results,
            [cancelToken] { return cancelToken->load(); });
    }));
}

void DiffDialog::cancel() {
    m_recompareTimer.stop();
    m_recomparePending = false;
    m_autoCompareButton->setChecked(false);
    m_phase = Phase::Cancelling;
    if (m_cancel) {
        *m_cancel = true;
    }
    if (m_searchCancel) {
        *m_searchCancel = true;
    }
    m_finalStatus = tr("Cancelling...");
    updateControls();
}

void DiffDialog::updateControls() {
    if (!m_startButton) {
        return;
    }
    m_progress->setVisible(m_running);
    updateComparisonState();
    m_startButton->setEnabled(!m_running && usable());
    m_cancelButton->setEnabled(m_running);
    for (auto button : std::as_const(m_navigation)) {
        button->setEnabled(m_model->rowCount() > 0);
    }

    m_swapButton->setEnabled(!m_running);
    for (auto choice : m_choices) {
        if (choice) {
            choice->setEnabled(!m_running);
        }
    }
}

void DiffDialog::requestComparison(const QString &reason) {
    invalidate(reason);
    queueComparison();
    updateControls();
}

void DiffDialog::queueComparison() {
    if (m_hasCompared && m_autoCompareButton->isChecked() && usable()) {
        m_recomparePending = true;
        m_phase = Phase::Waiting;
        m_recompareTimer.start();
    }
}

void DiffDialog::tryRecompare() {
    if (!m_recomparePending || !usable() || !m_autoCompareButton->isChecked()) {
        return;
    }
    if (!m_running) {
        start();
    }
}

void DiffDialog::setAutoCompare(bool enabled) {
    if (enabled) {
        m_hasCompared = true;
        queueComparison();
    } else {
        m_recomparePending = false;
        m_recompareTimer.stop();
        if (m_phase == Phase::Waiting) {
            m_phase = Phase::Stale;
        }
    }
    updateControls();
}

void DiffDialog::updateComparisonState() {
    QString text;
    if (!usable()) {
        text = tr("Choose two different documents");
    } else {
        switch (m_phase) {
        case Phase::Idle:
            text = tr("Not compared");
            break;
        case Phase::Waiting:
            text = tr("Source changed · waiting to compare");
            break;
        case Phase::Comparing:
            text = tr("Comparing...");
            break;
        case Phase::Cancelling:
            text = tr("Cancelling...");
            break;
        case Phase::Complete:
            text = !m_results || !m_results->publishedDifferences()
                       ? tr("Files are identical")
                       : (m_results->coarseCount()
                              ? tr("Comparison complete · includes coarse "
                                   "intervals")
                              : tr("Comparison complete · differences found"));
            break;
        case Phase::Cancelled:
            text = tr("Cancelled · partial results");
            break;
        case Phase::Stale:
            text = tr("Results out of date · compare again");
            break;
        case Phase::Failed:
            text = tr("Comparison failed");
            break;
        }
    }
    QString role = QStringLiteral("idle");
    if (usable()) {
        if (m_phase == Phase::Comparing || m_phase == Phase::Cancelling) {
            role = QStringLiteral("running");
        } else if (m_phase == Phase::Waiting || m_phase == Phase::Stale ||
                   m_phase == Phase::Failed || m_phase == Phase::Cancelled) {
            role = QStringLiteral("attention");
        } else if (m_phase == Phase::Complete) {
            role = m_results && !m_results->publishedDifferences()
                       ? QStringLiteral("equal")
                       : QStringLiteral("complete");
        }
    }
}

void DiffDialog::tick() {
    for (const auto side : {Side::Left, Side::Right}) {
        if (m_editors[side] && m_editors[side]->hexEditor()->document() !=
                                   m_views[side]->document()) {
            auto document = m_editors[side]->hexEditor()->document();
            invalidate(tr("Source reloaded — compare again"));
            m_views[side]->setDocument(document, m_views[side]->cursor());
            // Rebind change notifications after reload, which may replace the
            // document.
            m_editors[side] = nullptr;
            bindSource(side);
        }
        if (m_results && !m_sources[side].sourceUnchanged()) {
            invalidate(tr("Source file changed · reload or compare again"));
            break;
        }
    }
    if (m_results) {
        m_results->publish();
        const auto covered = m_results->coveredOffsets();
        const auto coverage =
            tr("Compared ranges end at left offset %1 and right offset %2")
                .arg(QStringLiteral("0x%1").arg(covered.first, 0, 16),
                     QStringLiteral("0x%1").arg(covered.second, 0, 16));
        m_progress->setToolTip(coverage);
        m_progress->setAccessibleDescription(coverage);
        m_status->setToolTip(coverage);
        const auto count = m_results->publishedDifferences();
        if (m_running || count != m_lastCount) {
            m_model->refresh();
        }
        m_countLabel->setText(tr("Differences · %1 intervals").arg(count));
        const auto total = quint64(m_sources[Side::Left].length) +
                           quint64(m_sources[Side::Right].length);
        m_progress->setValue(
            total ? int(m_results->processed.load() * ProgressScale / total)
                  : ProgressScale);
        const auto stage =
            m_running && m_phase == Phase::Comparing
                ? (m_results->summarizing ? tr("Summarizing") : tr("Comparing"))
                : m_finalStatus;
        m_status->setText(
            tr("%1 · %2 intervals (%3 coarse) · %4 s%5")
                .arg(stage)
                .arg(count)
                .arg(m_results->coarseCount())
                .arg((m_running ? m_elapsed.elapsed() : m_durationMs) / 1000)
                .arg(m_results->summarizing
                         ? (m_results->searchLimited
                                ? tr(" · search budget reached")
                                : tr(" · detail or memory limit reached"))
                         : QString()));
        m_lastCount = count;
    } else {
        m_status->setText(m_finalStatus);
        m_status->setToolTip({});
        m_progress->setToolTip({});
        m_progress->setAccessibleDescription({});
        m_progress->setValue(0);
        m_countLabel->setText(tr("Differences"));
    }
    if (m_phase == Phase::Cancelled && m_results) {
        const auto covered = m_results->coveredOffsets();
        m_status->setText(m_status->text() + tr(" · covered L 0x%1 / R 0x%2")
                                                 .arg(covered.first, 0, 16)
                                                 .arg(covered.second, 0, 16));
    }
    if (m_running) {
        for (auto view : m_views) {
            view->viewport()->update();
        }
    }
    updateControls();
}

void DiffDialog::locateRange(const WingDiff::Range &range) {
    m_scrolling = true;
    auto lview = m_views[Side::Left];
    lview->renderer()->setCursorVisible(true);
    lview->cursor()->moveTo(qsizetype(range.leftOffset));

    auto rview = m_views[Side::Right];
    rview->renderer()->setCursorVisible(true);
    rview->cursor()->moveTo(qsizetype(range.rightOffset));

    for (auto view : m_views) {
        view->ensureCurrentLineVisible();
    }
    m_scrolling = false;
}

void DiffDialog::selectDifference(quint64 row) {
    if (!m_results || row >= m_results->publishedDifferences()) {
        return;
    }
    m_selected = row;
    m_model->refresh();
    const auto source = m_model->index(int(row), 0);
    const auto index = m_table->mapFromSource(source);
    m_table->selectionModel()->setCurrentIndex(
        index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_table->scrollTo(index);
    locateRange(m_model->range(source.row()));
}

void DiffDialog::navigate(int direction) {
    if (!m_results || !m_results->publishedDifferences()) {
        return;
    }
    if (direction < 0) {
        selectDifference(m_selected ? m_selected - 1 : 0);
    } else {
        selectDifference(
            std::min(m_selected + 1, m_results->publishedDifferences() - 1));
    }
}

void DiffDialog::scrollTogether(Side side) {
    if (m_scrolling || !m_syncButton->isChecked() || !m_results) {
        return;
    }
    const auto source = m_views[side];
    const auto target = m_views[oppositeSide(side)];
    const auto sourceLines = quint64(source->documentLines());
    const auto sourceFactor =
        sourceLines >= INT_MAX ? sourceLines / INT_MAX + 1 : 1;
    const auto offset = quint64(source->verticalScrollBar()->value()) *
                        sourceFactor *
                        quint64(source->document()->hexLineWidth());
    const auto mapped = m_results->mapOffset(offset, side == Side::Right);
    if (mapped == std::numeric_limits<quint64>::max()) {
        return;
    }
    const auto row = mapped / quint64(target->document()->hexLineWidth());
    const auto lines = quint64(target->documentLines());
    const auto factor = lines >= INT_MAX ? lines / INT_MAX + 1 : 1;
    m_scrolling = true;
    target->verticalScrollBar()->setValue(int(row / factor));
    m_scrolling = false;
}

void DiffDialog::setOrientation(bool vertical) {
    m_viewsSplitter->setOrientation(vertical ? Qt::Vertical : Qt::Horizontal);
    SettingManager::instance().setDiffVertical(vertical);
}

void DiffDialog::applyTheme() {
    const bool dark =
        ThemeManager::instance().currentTheme() == ThemeManager::Theme::Dark;
    for (const auto side : {Side::Left, Side::Right}) {
        auto view = m_views[side];
        if (!view) {
            continue;
        }
        view->setHeaderColor(dark ? QColor(168, 176, 182)
                                  : QColor(80, 90, 100));
        view->setAddressColor(dark ? QColor(168, 176, 182)
                                   : QColor(80, 90, 100));
        view->setBorderColor(dark ? QColor(90, 96, 102)
                                  : QColor(180, 185, 190));
        view->setSelectionColor(QColor(255, 255, 255));
        view->setSelBackgroundColor(dark ? QColor(45, 100, 145)
                                         : QColor(45, 110, 165));
        view->setBytesBackground(dark ? QColor(35, 38, 41)
                                      : QColor(255, 255, 255));
        view->setBytesAlterBackground(dark ? QColor(42, 46, 50)
                                           : QColor(245, 245, 245));
        view->setBytesColor(dark ? QColor(239, 240, 241) : QColor(49, 54, 59));
        auto renderer = static_cast<DiffRenderer *>(view->renderer());
        renderer->setDarkTheme(dark);
        renderer->setResults(m_results);
    }
}

void DiffDialog::showSearch() {
    m_listPanel->show();
    m_listButton->setChecked(true);
    m_search->show();
    m_search->activate();
}

void DiffDialog::search(bool backwards) {
    if (m_searchCancel) {
        *m_searchCancel = true;
    }
    if (!m_results) {
        return;
    }
    const auto query = m_search->query();
    if (query.text.isEmpty()) {
        m_search->setState(SearchBar::State::Idle);
        return;
    }
    if (!query.pattern.isValid()) {
        m_search->setState(SearchBar::State::InvalidPattern,
                           query.pattern.errorString());
        return;
    }
    m_searchCancel = std::make_shared<std::atomic_bool>(false);
    const auto token = m_searchCancel;
    const auto results = m_results;
    results->publish();
    const auto count = results->publishedDifferences();
    const auto start = m_selected;
    const auto left = m_sources[Side::Left], right = m_sources[Side::Right];
    const auto generation = m_generation;
    m_search->setState(SearchBar::State::Searching,
                       tr("Searching %1 committed intervals").arg(count));
    auto watcher = new QFutureWatcher<qint64>(this);
    connect(watcher, &QFutureWatcher<qint64>::finished, this,
            [this, watcher, token, generation] {
                const auto row = watcher->result();
                watcher->deleteLater();
                if (token->load() || generation != m_generation) {
                    return;
                }
                m_search->setState(row < 0 ? SearchBar::State::NoMatch
                                           : SearchBar::State::Idle);
                if (row >= 0) {
                    selectDifference(quint64(row));
                }
            });
    watcher->setFuture(QtConcurrent::run([results, count, start, query,
                                          backwards, left, right,
                                          token]() -> qint64 {
        for (quint64 n = 0; n < count && !token->load(); ++n) {
            const auto distance = n + 1;
            qint64 candidate = backwards ? qint64(start) - qint64(distance)
                                         : qint64(start + distance);
            if (!query.options.wrap &&
                (candidate < 0 || candidate >= qint64(count))) {
                break;
            }
            const auto row = quint64(
                (candidate % qint64(count) + qint64(count)) % qint64(count));
            WingDiff::Range range;
            if (!results->difference(row, &range)) {
                continue;
            }
            for (int column :
                 {DiffResultModel::Kind, DiffResultModel::LeftOffset,
                  DiffResultModel::LeftLength, DiffResultModel::RightOffset,
                  DiffResultModel::RightLength, DiffResultModel::Precision}) {
                if (query.matches(DiffResultModel::field(range, column))) {
                    return qint64(row);
                }
            }
        }
        return -1;
    }));
}
