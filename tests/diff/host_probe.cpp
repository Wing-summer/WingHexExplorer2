#include <cstdio>
#include <functional>
#include <random>

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QElapsedTimer>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QThread>
#include <QThreadPool>
#include <QToolButton>
#include <QtConcurrent>

#include "class/settings/settingmanager.h"
#include "class/settings/thememanager.h"
#include "control/editorview.h"
#include "control/qtableviewext.h"
#include "control/searchbar.h"
#include "dialog/diffdialog.h"
#include "dialog/settingdialog.h"
#include "model/diffresultmodel.h"
#include "settings/diffsettingdialog.h"

static bool waitFor(const std::function<bool()> &predicate) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 15000) {
        QApplication::processEvents();
        if (predicate()) {
            return true;
        }
        QThread::msleep(5);
    }
    return false;
}

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "FAIL %d: %s\n", __LINE__, #condition);       \
            return 1;                                                          \
        }                                                                      \
    } while (false)

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("WingCloudStudio"));
    app.setApplicationName(QStringLiteral("WingHexExplorer2"));
    QTemporaryDir config;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       config.path());
    {
        QSettings settings(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME));
        settings.setValue(QStringLiteral("diff.fileLimit"),
                          quint64(2048) * WingDiff::MiB);
        settings.setValue(QStringLiteral("diff.memoryLimit"),
                          quint64(128) * WingDiff::MiB);
        settings.setValue(QStringLiteral("diff.detailLimit"),
                          QStringLiteral("invalid"));
        settings.setValue(QStringLiteral("diff.precise"), true);
    }
    auto &manager = SettingManager::instance();
    CHECK(manager.diffFileLimit() == 2048 * WingDiff::MiB);
    CHECK(manager.diffMemoryLimit() == 128 * WingDiff::MiB);
    CHECK(manager.diffDetailLimit() == 100000);
    CHECK(manager.diffPrecise());
    manager.reset(SettingManager::Diff);
    CHECK(manager.diffFileLimit() == 32768 * WingDiff::MiB);
    manager.setDiffFileLimit(0);
    CHECK(manager.diffFileLimit() == 64 * WingDiff::MiB);
    manager.setDiffMemoryLimit(123);
    CHECK(manager.diffMemoryLimit() == 64 * WingDiff::MiB);
    manager.setDiffDetailLimit(2000000);
    CHECK(manager.diffDetailLimit() == 1000000);
    manager.setDiffPrecise(true);
    {
        QSettings settings(QStringLiteral(APP_ORG), QStringLiteral(APP_NAME));
        CHECK(settings.value(QStringLiteral("diff.precise")).toBool());
    }
    manager.reset(SettingManager::Editor);
    CHECK(manager.diffPrecise());
    auto configuration =
        std::make_unique<SettingDialog>(QIcon(), QStringLiteral("Settings"));
    auto dummy = new DiffSettingDialog(configuration.get());
    auto page = new DiffSettingDialog(configuration.get());
    configuration->addPage(dummy);
    configuration->addPage(page);
    configuration->build();
    CHECK(
        page->findChild<QSpinBox *>(QStringLiteral("sbFileLimit"))->maximum() ==
        65536);
    page->restore();
    CHECK(!manager.diffPrecise() && manager.diffDetailLimit() == 100000);
    page->findChild<QSpinBox *>(QStringLiteral("sbFileLimit"))->setValue(2048);
    CHECK(manager.diffFileLimit() == 2048 * WingDiff::MiB);
    CHECK(
        dummy->findChild<QSpinBox *>(QStringLiteral("sbFileLimit"))->value() ==
        2048);
    manager.reset(SettingManager::All);
    CHECK(manager.diffFileLimit() == 32768 * WingDiff::MiB);
    auto left = std::make_unique<EditorView>();
    auto right = std::make_unique<EditorView>();
    CHECK(left->newFile(1) == WingHex::ErrFile::Success);
    CHECK(right->newFile(2) == WingHex::ErrFile::Success);
    std::mt19937 generator(19);
    QByteArray bytes(1024 * 1024, Qt::Uninitialized);
    for (auto &c : bytes) {
        c = char(generator());
    }
    CHECK(left->hexEditor()->document()->insert(0, bytes));
    for (int i = 0; i < bytes.size(); i += 128) {
        bytes[i] ^= 0x55;
    }
    CHECK(right->hexEditor()->document()->insert(0, bytes));
    auto clone = std::unique_ptr<EditorView>(left->clone());
    CHECK(clone);
    QPointer<DiffDialog> dialog = new DiffDialog();
    bool settingsOpened = false;
    bool settingValuesFit = false;
    QObject::connect(
        dialog, &DiffDialog::sigOnSettingsRequested, configuration.get(), [&] {
            QTimer::singleShot(0, page, [page, &settingValuesFit] {
                settingValuesFit = true;
                for (auto spin : page->findChildren<QSpinBox *>()) {
                    const auto editor = spin->findChild<QLineEdit *>();
                    settingValuesFit =
                        settingValuesFit && editor &&
                        editor->contentsRect().width() >=
                            editor->fontMetrics().horizontalAdvance(
                                spin->text());
                }
                page->window()->grab().save(
                    QStringLiteral("/tmp/wing-diff-settings.png"));
                page->window()->close();
            });
            configuration->showConfig(page);
            settingsOpened = true;
        });
    const auto settingsButton =
        dialog->findChild<QToolButton *>(QStringLiteral("diff.settings"));
    CHECK(settingsButton);
    settingsButton->click();
    CHECK(settingsOpened && settingValuesFit);
    CHECK(configuration->findChild<QStackedWidget *>()->currentWidget() ==
          page);
    configuration->hide();
    dialog->show();
    auto first =
        dialog->findChild<QComboBox *>(QStringLiteral("diff.left.source"));
    auto second =
        dialog->findChild<QComboBox *>(QStringLiteral("diff.right.source"));
    CHECK(first && second && first->count() == 2 && second->count() == 2);
    for (const auto name : {"diff.left.open", "diff.right.open",
                            "diff.left.close", "diff.right.close"}) {
        CHECK(!dialog->findChild<QToolButton *>(QString::fromLatin1(name)));
    }
    auto view = dialog->findChild<QHexView *>(QStringLiteral("diff.left.view"));
    CHECK(first->currentIndex() == -1 && second->currentIndex() == -1);
    CHECK(view && view->document() != left->hexEditor()->document());
    CHECK(view->cursor() != left->hexEditor()->cursor());
    CHECK(view->isReadOnly() && !left->hexEditor()->isReadOnly());
    auto run = dialog->findChild<QToolButton *>(QStringLiteral("diff.compare"));
    auto status = dialog->findChild<QLabel *>(QStringLiteral("diff.status"));
    auto progress =
        dialog->findChild<QProgressBar *>(QStringLiteral("diff.progress"));
    QToolButton *automatic = nullptr;
    QToolButton *cancel = nullptr;
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Auto compare")) {
            automatic = button;
        }
        if (button->text() == QStringLiteral("Cancel")) {
            cancel = button;
        }
    }
    CHECK(run && status && progress && automatic && cancel &&
          !run->isEnabled());
    CHECK(automatic->isChecked());
    QElapsedTimer unselected;
    unselected.start();
    CHECK(waitFor([&] { return unselected.elapsed() > 650; }));
    CHECK(first->currentIndex() == -1 && second->currentIndex() == -1);
    CHECK(!progress->isVisible());
    const auto leftIndex = first->findText(left->infoFileName());
    const auto rightIndex = second->findText(right->infoFileName());
    CHECK(leftIndex >= 0 && rightIndex >= 0);
    first->setCurrentIndex(leftIndex);
    CHECK(view->document() == left->hexEditor()->document());
    QElapsedTimer oneSource;
    oneSource.start();
    CHECK(waitFor([&] { return oneSource.elapsed() > 650; }));
    CHECK(!run->isEnabled() && !progress->isVisible());
    second->setCurrentIndex(second->findText(left->infoFileName()));
    QElapsedTimer sameSource;
    sameSource.start();
    CHECK(waitFor([&] { return sameSource.elapsed() > 650; }));
    CHECK(!run->isEnabled() && !progress->isVisible());
    second->setCurrentIndex(rightIndex);
    CHECK(waitFor([&] {
        return run->isEnabled() &&
               status->text().startsWith(QStringLiteral("Complete"));
    }));
    CHECK(dialog->findChild<DiffResultModel *>()->rowCount() > 2048);
    CHECK(!progress->isVisible());
    run->click();
    CHECK(progress->isVisible());
    CHECK(!run->isEnabled());
    CHECK(waitFor([&] {
        return run->isEnabled() &&
               status->text().startsWith(QStringLiteral("Complete"));
    }));
    auto model = dialog->findChild<DiffResultModel *>();
    CHECK(model && model->rowCount() > 2048);
    const auto rows = model->rowCount();
    CHECK(!progress->isVisible());
    CHECK(status->text().startsWith(QStringLiteral("Complete")));
    auto document = left->hexEditor()->document();
    document->setBaseAddress(0x1000);
    document->setMetabgVisible(!document->metabgVisible());
    document->addBookMark(10, QStringLiteral("Marker"));
    document->sigOnDocumentSaved(true);
    QElapsedTimer untouched;
    untouched.start();
    CHECK(waitFor([&] { return untouched.elapsed() > 650; }));
    CHECK(model->rowCount() == rows && !progress->isVisible() &&
          run->isEnabled());
    auto search = dialog->findChild<SearchBar *>();
    CHECK(search);
    auto table =
        dialog->findChild<QTableViewExt *>(QStringLiteral("diff.results"));
    CHECK(table);
    CHECK(waitFor([&] {
        return qAbs(table->horizontalHeader()->length() -
                    table->viewport()->width()) <= 2;
    }));
    const auto offsetWidth = table->columnWidth(DiffResultModel::LeftOffset);
    dialog->resize(dialog->width() + 200, dialog->height());
    CHECK(waitFor([&] {
        return table->columnWidth(DiffResultModel::LeftOffset) > offsetWidth &&
               qAbs(table->horizontalHeader()->length() -
                    table->viewport()->width()) <= 2;
    }));
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Difference list")) {
            button->click();
        }
    }
    CHECK(!table->isVisible());
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Search")) {
            button->click();
        }
    }
    CHECK(table->isVisible() && search->isVisible());
    CHECK(search->geometry().top() > table->geometry().bottom());
    search->setSearchText(QStringLiteral("0X80000"));
    search->cancelPendingSearch();
    search->sigOnSearchRequested(false);
    CHECK(waitFor([&] { return view->currentOffset() == 0x80000; }));
    CHECK(view->currentOffset() == 0x80000);
    search->sigOnCloseRequested();
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("First")) {
            button->click();
        }
    }
    CHECK(view->currentOffset() == 0);
    CHECK(table->mapToSource(table->currentIndex()).row() == 0);
    auto splitter =
        dialog->findChild<QSplitter *>(QStringLiteral("diff.views"));
    CHECK(splitter);
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Stacked")) {
            button->click();
        }
    }
    CHECK(splitter->orientation() == Qt::Vertical && model->rowCount() == rows);
    for (auto button : dialog->findChildren<QToolButton *>()) {
        if (button->text() == QStringLiteral("Side by side")) {
            button->click();
        }
    }
    CHECK(splitter->orientation() == Qt::Horizontal);
    QApplication::processEvents();
    dialog->grab().save(QStringLiteral("/tmp/wing-diff-ui-light.png"));
    ThemeManager::instance().setTheme(ThemeManager::Theme::Dark);
    QApplication::processEvents();
    dialog->grab().save(QStringLiteral("/tmp/wing-diff-ui-dark.png"));
    const auto before = left->hexEditor()->document()->read(0, 20);
    app.clipboard()->setText(QStringLiteral("FFFF"));
    CHECK(!view->Paste(true, 0));
    CHECK(left->hexEditor()->document()->read(0, 20) == before);
    CHECK(left->hexEditor()->document()->replace(0, QByteArray("Z")));
    CHECK(model->rowCount() == 0);
    CHECK(!progress->isVisible() && automatic->isChecked());
    dialog->grab().save(QStringLiteral("/tmp/wing-diff-state-waiting.png"));
    CHECK(waitFor([&] {
        return run->isEnabled() && model->rowCount() > 0 &&
               status->text().startsWith(QStringLiteral("Complete"));
    }));
    CHECK(!progress->isVisible());
    automatic->click();
    CHECK(!automatic->isChecked());
    CHECK(document->replace(1, QByteArray("Q")));
    CHECK(model->rowCount() == 0);
    CHECK(!automatic->isChecked() && !progress->isVisible());
    QElapsedTimer paused;
    paused.start();
    CHECK(waitFor([&] { return paused.elapsed() > 650; }));
    CHECK(model->rowCount() == 0 && !progress->isVisible());
    run->click();
    CHECK(progress->isVisible());
    cancel->click();
    CHECK(waitFor([&] { return run->isEnabled() && !progress->isVisible(); }));
    CHECK(!automatic->isChecked());
    CHECK(waitFor([&] {
        return status->text().startsWith(QStringLiteral("Cancelled"));
    }));
    // Closing a selected editor detaches its document even if a clone survives.
    left.reset();
    CHECK(waitFor([&] {
        return !run->isEnabled() &&
               status->text().contains(QStringLiteral("closed"));
    }));
    CHECK(model->rowCount() == 0 && !progress->isVisible());
    CHECK(clone->hexEditor()->document()->replace(3, QByteArray("X")));
    QElapsedTimer detached;
    detached.start();
    CHECK(waitFor([&] { return detached.elapsed() > 150; }));
    CHECK(status->text().contains(QStringLiteral("closed")));
    auto replacement = std::make_unique<EditorView>();
    CHECK(replacement->newFile(3) == WingHex::ErrFile::Success);
    CHECK(replacement->hexEditor()->document()->insert(0, bytes));
    first->showPopup();
    CHECK(first->count() == 2);
    const auto replacementIndex = first->findText(replacement->infoFileName());
    CHECK(replacementIndex >= 0);
    first->setCurrentIndex(replacementIndex);
    first->hidePopup();
    CHECK(run->isEnabled());
    run->click();
    dialog->close();
    CHECK(waitFor([&] { return dialog.isNull(); }));
    clone.reset();
    left.reset();
    right.reset();
    CHECK(waitFor([] {
        return QThreadPool::globalInstance()->activeThreadCount() == 0;
    }));
    for (int count : {100000, 1000000}) {
        auto store =
            std::make_shared<WingDiff::ResultStore>(64 * WingDiff::MiB);
        for (int row = 0; row < count; ++row) {
            CHECK(store->append({quint64(row) * 2, 1, quint64(row) * 2, 1,
                                 WingDiff::Type::Replace, false}));
        }
        store->publish();
        DiffResultModel largeModel;
        largeModel.setStore(store);
        const auto rss = [] {
            QFile status(QStringLiteral("/proc/self/status"));
            if (!status.open(QIODevice::ReadOnly)) {
                return qint64(-1);
            }
            const auto lines = status.readAll().split('\n');
            for (const auto &line : lines) {
                if (line.startsWith("VmRSS:")) {
                    return line.simplified().split(' ').value(1).toLongLong();
                }
            }
            return qint64(-1);
        };
        const auto before = rss();
        QElapsedTimer elapsed;
        elapsed.start();
        QTableViewExt largeTable;
        largeTable.setSearchEnabled(false);
        largeTable.setAsyncSortingEnabled(false);
        largeTable.setSortingEnabled(false);
        largeTable.verticalHeader()->setDefaultSectionSize(24);
        largeTable.verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        largeTable.setModel(&largeModel);
        for (int column = 0; column < DiffResultModel::ColumnCount; ++column) {
            QTableViewExt::ColumnWidthPolicy policy;
            policy.source = QTableViewExt::WidthSource::Specified;
            policy.update = QTableViewExt::WidthUpdate::Initial;
            policy.value = 120;
            largeTable.setColumnWidthPolicy(column, policy);
        }
        largeTable.resize(1000, 500);
        largeTable.show();
        app.processEvents();
        const auto target = largeModel.index(count - 1, 0);
        largeTable.setCurrentIndex(target);
        largeTable.scrollTo(target);
        app.processEvents();
        CHECK(largeTable.mapToSource(largeTable.currentIndex()).row() ==
              count - 1);
        CHECK(store->append({quint64(count) * 2, 1, quint64(count) * 2, 1,
                             WingDiff::Type::Replace, false}));
        store->publish();
        largeModel.refresh();
        CHECK(largeTable.currentIndex().row() == count - 1);
        CHECK(largeModel.rowCount() == count + 1);
        const auto widgetRss = rss() - before;
        const auto interactionMs = elapsed.elapsed();
        SearchBar searchInput;
        searchInput.setSearchText(QStringLiteral("0X%1")
                                      .arg(quint64(count - 1) * 2, 0, 16)
                                      .toUpper());
        searchInput.cancelPendingSearch();
        const auto query = searchInput.query();
        const auto scanStarted = elapsed.elapsed();
        auto found = QtConcurrent::run([store, query, count] {
            for (int row = 0; row < count; ++row) {
                WingDiff::Range range;
                if (store->difference(quint64(row), &range) &&
                    query.matches(DiffResultModel::field(
                        range, DiffResultModel::LeftOffset))) {
                    return row;
                }
            }
            return -1;
        });
        CHECK(waitFor([&] { return found.isFinished(); }));
        CHECK(found.result() == count - 1);
        std::printf("SEARCH rows=%d elapsed_ms=%lld\n", count,
                    static_cast<long long>(elapsed.elapsed() - scanStarted));
        std::printf("LIST rows=%d widget_rss_delta_KiB=%lld "
                    "show_scroll_append_ms=%lld\n",
                    count, static_cast<long long>(widgetRss),
                    static_cast<long long>(interactionMs));
    }
    std::puts("DIFF_HOST_PROBE_PASS");
    return 0;
}
