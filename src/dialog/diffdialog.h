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

#ifndef DIFFDIALOG_H
#define DIFFDIALOG_H

#include <type_traits>
#include <utility>

#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QKeySequence>
#include <QPointer>
#include <QProgressBar>
#include <QShortcut>
#include <QTimer>
#include <QToolButton>

#include "QWingRibbon/ribbon.h"
#include "QWingRibbon/ribbonbuttongroup.h"

#include "class/diff/binarydiff.h"
#include "class/diff/diffsource.h"
#include "class/settings/thememanager.h"
#include "control/editorview.h"
#include "control/qtableviewext.h"
#include "framelessmainwindow.h"
#include "model/diffresultmodel.h"

class DiffDialog final : public FramelessMainWindow {
    Q_OBJECT

public:
    explicit DiffDialog(EditorView *initial = nullptr,
                        QWidget *parent = nullptr);
    virtual ~DiffDialog() override;

signals:
    void sigOnSettingsRequested();

private:
    enum Side { Left, Right, SideCount };

    enum PaneLayout { SideBySide, Stacked, LayoutCount };
    enum class Phase {
        Idle,
        Waiting,
        Comparing,
        Cancelling,
        Complete,
        Cancelled,
        Stale,
        Failed
    };

    static constexpr Side oppositeSide(Side side) {
        return side == Side::Left ? Side::Right : Side::Left;
    }

private:
    template <typename Func>
    QToolButton *
    addPannelAction(RibbonButtonGroup *group, const QString &text,
                    ThemeManager::StandardIcon icon, Func &&action,
                    bool checkable = false, const QKeySequence &shortcut = {},
                    QMenu *menu = nullptr, const QString &description = {}) {
        auto result = new QToolButton(group);
        result->setText(text);
        result->setAccessibleName(text);
        result->setAccessibleDescription(description.isEmpty() ? text
                                                               : description);
        result->setToolTip(
            shortcut.isEmpty()
                ? text
                : text + QStringLiteral(" (") +
                      shortcut.toString(QKeySequence::NativeText) +
                      QStringLiteral(")"));
        result->setIcon(ThemeManager::instance().standardIcon(icon));
        result->setCheckable(checkable);
        result->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        result->setMenu(menu);
        if (menu) {
            result->setPopupMode(QToolButton::InstantPopup);
        }
        if (!shortcut.isEmpty()) {
            connect(new QShortcut(shortcut, this), &QShortcut::activated,
                    result, &QToolButton::click);
        }
        if constexpr (!std::is_same_v<std::decay_t<Func>, std::nullptr_t>) {
            connect(result, &QToolButton::clicked, this,
                    std::forward<Func>(action));
        }
        group->addButton(result);
        return result;
    }

private:
    void comparisonFinished();
    void sourceContentChanged();
    void sourceSaved(bool saved);
    void swapSides();
    void firstDifference();
    void previousDifference();
    void nextDifference();
    void lastDifference();
    void showSideBySide();
    void showStacked();
    void resetLayout();
    void setSyncScrolling(bool enabled);
    void setListVisible(bool visible);
    void setAsciiVisible(bool visible);
    void setAddressesVisible(bool visible);
    void selectCurrentDifference(const QModelIndex &index);
    void cancelSearch();
    void closeSearch();
    void buildUpRibbonBar();
    void buildComparePage();
    void buildViewPage();
    void buildViews();
    void buildDifferenceList();
    void buildStatusBar();
    void reloadSettings();
    void refreshCandidates();
    void bindSource(Side side);
    void invalidate(const QString &reason);
    void start();
    void cancel();
    void tick();
    void updateControls();
    void updateComparisonState();
    void requestComparison(const QString &reason);
    void queueComparison();
    void tryRecompare();
    void setAutoCompare(bool enabled);
    bool recaptureSources();
    void locateRange(const WingDiff::Range &range);
    void selectDifference(quint64 row);
    void navigate(int direction);
    void scrollTogether(Side side);
    void search(bool backwards);
    void showSearch();
    void setOrientation(bool vertical);
    void applyTheme();
    bool usable() const;

private:
    QPointer<EditorView> m_editors[SideCount];
    QVector<QPointer<EditorView>> m_candidates;
    QList<QMetaObject::Connection> m_sourceConnections[SideCount];
    QHexView *m_views[SideCount] = {};
    QComboBox *m_choices[SideCount] = {};
    WingDiff::Source m_sources[SideCount];
    Ribbon *m_ribbon;
    QSplitter *m_viewsSplitter;
    QSplitter *m_contentSplitter;
    QWidget *m_listPanel;
    QTableViewExt *m_table;
    DiffResultModel *m_model;
    SearchBar *m_search;
    QLabel *m_status;
    QLabel *m_countLabel;
    QProgressBar *m_progress;
    QList<QToolButton *> m_navigation;

private:
    QToolButton *m_startButton;
    QToolButton *m_cancelButton;
    QToolButton *m_swapButton;
    QToolButton *m_autoCompareButton;
    QToolButton *m_syncButton;
    QToolButton *m_arrangementButtons[LayoutCount] = {};
    QToolButton *m_listButton;

private:
    QTimer m_timer;
    QTimer m_recompareTimer;
    QElapsedTimer m_elapsed;

private:
    QFutureWatcher<WingDiff::Outcome> m_worker;
    std::shared_ptr<WingDiff::ResultStore> m_results;
    std::shared_ptr<std::atomic_bool> m_cancel;
    std::shared_ptr<std::atomic_bool> m_searchCancel;
    WingDiff::Options m_options;
    quint64 m_generation = 0;
    quint64 m_selected = 0;
    quint64 m_lastCount = 0;
    Phase m_phase = Phase::Idle;
    bool m_hasCompared = false;
    bool m_recomparePending = false;
    bool m_running = false;
    bool m_invalid = false;
    bool m_scrolling = false;
    qint64 m_durationMs = 0;
    QString m_finalStatus;
};
#endif
