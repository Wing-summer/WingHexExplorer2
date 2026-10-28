#include "control/delegate/richtextitemdelegate.h"
#include "control/delegate/wrappingitemdelegate.h"
#include "control/qtableviewext.h"
#include "control/searchabletextbrowser.h"
#include "control/searchabletreewidget.h"
#include "control/searchbar.h"
#include "control/tablecolumnwidthcontroller.h"
#include "model/asynctableproxymodel.h"

#include <QAbstractItemModelTester>
#include <QAction>
#include <QElapsedTimer>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTest>
#include <QTextDocument>
#include <QThread>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <cmath>
#include <limits>

struct GuiText {
    QString text;
};
Q_DECLARE_METATYPE(GuiText)
static std::atomic<bool> wrongConversionThread{false};

class TableModel : public QAbstractTableModel {
public:
    int count = 1000;
    mutable bool wrongThread = false;
    int rowCount(const QModelIndex &p = {}) const override {
        return p.isValid() ? 0 : count;
    }
    int columnCount(const QModelIndex &p = {}) const override {
        return p.isValid() ? 0 : 4;
    }
    QVariant data(const QModelIndex &i, int role) const override {
        wrongThread |= QThread::currentThread() != thread();
        if (!i.isValid() || role != Qt::DisplayRole) {
            return {};
        }
        if (i.column() == 0) {
            return count - i.row();
        }
        if (i.column() == 1) {
            return i.row() % 3;
        }
        return QStringLiteral("row %1").arg(i.row());
    }
    void resetCount(int n) {
        beginResetModel();
        count = n;
        endResetModel();
    }
};

class TableTests : public QObject {
    Q_OBJECT
private slots:
    void sorting();
    void superseded();
    void headerClicks();
    void sourceChanges();
    void numericLimits();
    void mixedNumericPrecision();
    void textSorting();
    void customKeyConversion();
    void modelState();
    void extremeWeights();
    void contentSampling();
    void richTextWidths();
    void searchOptions();
    void shortcutPrecedence();
    void widthConstraints();
    void manualWidths();
    void deferredWidths();
    void search();
    void widths();
    void resultColumnsFill();
    void emptyResultColumnsFill();
    void searchBarPresentation();
    void searchWithScrollBars();
    void treeColumnRatios();
    void centeredHeaders();
    void searchBarHeightConsistency();
    void uniformIconInk();
    void wrappingDiagnostics();
    void reusableSearchBar();
    void lazyTreeSearch();
    void treeSearchScrollBars();
    void decodedTextSearch();
    void textSearchScrollBars();
    void rapidEmptyResize();
    void performance_data();
    void performance();
};
void TableTests::sorting() {
    TableModel source;
    QTableViewExt view;
    view.setAsyncSortingEnabled(true);
    view.setModel(&source);
    auto proxy = qobject_cast<AsyncTableProxyModel *>(view.model());
    QAbstractItemModelTester tester(
        proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QPersistentModelIndex persistent(proxy->index(0, 0));
    view.selectionModel()->setCurrentIndex(proxy->index(0, 0),
                                           QItemSelectionModel::ClearAndSelect);
    proxy->sort(0);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(proxy->index(0, 0).data().toInt(), 1);
    QCOMPARE(persistent.row(), 999);
    QCOMPARE(view.mapToSource(view.currentIndex()).row(), 0);
    QCOMPARE(view.mapToSource(proxy->index(0, 0)).row(), 999);
    QCOMPARE(view.mapFromSource(source.index(999, 0)).row(), 0);
    proxy->sort(1);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.mapToSource(proxy->index(0, 0)).row(), 0);
    QCOMPARE(view.mapToSource(proxy->index(1, 0)).row(), 3);
    view.setColumnSortable(1, false);
    QCOMPARE(view.mapToSource(proxy->index(1, 0)).row(), 1);
    proxy->sort(1, Qt::DescendingOrder);
    QVERIFY(!view.isSorting());
    view.setAsyncSortingEnabled(false);
    QCOMPARE(view.model(), &source);
    QVERIFY(!source.wrongThread);
}
void TableTests::superseded() {
    TableModel source;
    source.count = 100000;
    QTableViewExt view;
    view.setAsyncSortingEnabled(true);
    view.setModel(&source);
    view.model()->sort(0);
    QTest::qWait(5);
    view.model()->sort(1, Qt::DescendingOrder);
    source.resetCount(17);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.model()->rowCount(), 17);
    QCOMPARE(view.model()->index(0, 1).data().toInt(), 2);
    view.model()->sort(0);
    view.setModel(nullptr);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.model()->rowCount(), 0);
    QVERIFY(!source.wrongThread);
    auto heapSource = new TableModel;
    view.setModel(heapSource);
    view.model()->sort(0);
    delete heapSource;
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.model()->rowCount(), 0);
}
void TableTests::headerClicks() {
    TableModel source;
    QTableViewExt view;
    view.setAsyncSortingEnabled(true);
    view.setModel(&source);
    view.setColumnSortable(2, false);
    view.resize(500, 220);
    view.show();
    QTest::qWait(10);
    const auto click = [&](int column) {
        QTest::mouseClick(
            view.horizontalHeader()->viewport(), Qt::LeftButton, Qt::NoModifier,
            QPoint(view.horizontalHeader()->sectionViewportPosition(column) +
                       15,
                   10));
    };
    click(0);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.model()->index(0, 0).data().toInt(), 1);
    click(0);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.model()->index(0, 0).data().toInt(), 1000);
    click(2);
    QVERIFY(!view.isSorting());
    QCOMPARE(view.horizontalHeader()->sortIndicatorSection(), 0);
    view.setColumnSortable(0, false);
    QVERIFY(!view.horizontalHeader()->isSortIndicatorShown());
    QCOMPARE(view.mapToSource(view.model()->index(1, 0)).row(), 1);
}
void TableTests::sourceChanges() {
    QStandardItemModel source(3, 2);
    for (int i = 0; i < 3; ++i) {
        source.setData(source.index(i, 0), 3 - i);
    }
    AsyncTableProxyModel proxy;
    proxy.setSourceModel(&source);
    QAbstractItemModelTester tester(
        &proxy, QAbstractItemModelTester::FailureReportingMode::QtTest);
    proxy.sort(0);
    QTRY_VERIFY(!proxy.isSorting());
    QVERIFY(proxy.setData(proxy.index(0, 0), -5));
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(source.index(2, 0).data().toInt(), -5);
    source.insertRow(0);
    source.setData(source.index(0, 0), -10);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.rowCount(), 4);
    QCOMPARE(proxy.index(0, 0).data().toInt(), -10);
    source.removeRow(0);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.rowCount(), 3);
    source.insertColumn(0);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.columnCount(), 3);
    source.clear();
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.rowCount(), 0);
    // Destruction during active work must neither wait nor access the view.
    TableModel big;
    big.count = 100000;
    auto view = new QTableViewExt;
    view->setAsyncSortingEnabled(true);
    view->setModel(&big);
    view->sortByColumn(0, Qt::AscendingOrder);
    QTest::qWait(10);
    delete view;
    QTest::qWait(10);
}
void TableTests::numericLimits() {
    QStandardItemModel source(5, 1);
    source.setData(source.index(0, 0),
                   QVariant::fromValue(std::numeric_limits<quint64>::max()));
    source.setData(
        source.index(1, 0),
        QVariant::fromValue(std::numeric_limits<quint64>::max() - 1));
    source.setData(source.index(2, 0), QVariant::fromValue(qint64(-1)));
    source.setData(source.index(3, 0), QVariant::fromValue(quint64(0)));
    source.setData(source.index(4, 0),
                   QVariant::fromValue(std::numeric_limits<qint64>::max()));
    AsyncTableProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0);
    QTRY_VERIFY(!proxy.isSorting());
    const QList<int> expected{2, 3, 4, 1, 0};
    for (int r = 0; r < expected.size(); ++r) {
        QCOMPARE(proxy.mapToSource(proxy.index(r, 0)).row(), expected.at(r));
    }
}
void TableTests::mixedNumericPrecision() {
    const QList<QVariant> values{
        QVariant::fromValue(std::numeric_limits<quint64>::max()),
        std::ldexp(1.0, 64),
        QVariant::fromValue(std::numeric_limits<quint64>::max() - 1),
        QVariant::fromValue(std::numeric_limits<qint64>::max()),
        std::ldexp(1.0, 63),
        -0.5,
        QVariant::fromValue(qint64(-1)),
        std::numeric_limits<double>::quiet_NaN(),
        QVariant::fromValue(quint64(0)),
        0.5,
    };
    QStandardItemModel source(values.size(), 1);
    for (int r = 0; r < values.size(); ++r) {
        source.setData(source.index(r, 0), values.at(r));
    }
    AsyncTableProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0);
    QTRY_VERIFY(!proxy.isSorting());
    const QList<int> expected{7, 6, 5, 8, 9, 3, 4, 2, 0, 1};
    for (int r = 0; r < expected.size(); ++r) {
        QCOMPARE(proxy.mapToSource(proxy.index(r, 0)).row(), expected.at(r));
    }
    proxy.sort(0, Qt::DescendingOrder);
    QTRY_VERIFY(!proxy.isSorting());
    for (int r = 0; r < expected.size(); ++r) {
        QCOMPARE(proxy.mapToSource(proxy.index(r, 0)).row(),
                 expected.at(expected.size() - r - 1));
    }
}
void TableTests::textSorting() {
    QStandardItemModel source(3, 2);
    source.setData(source.index(0, 0), QStringLiteral("zebra"));
    source.setData(source.index(1, 0), QStringLiteral("apple"));
    source.setData(source.index(2, 0), QStringLiteral("apple"));
    AsyncTableProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0, Qt::DescendingOrder);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.mapToSource(proxy.index(0, 0)).row(), 0);
    QCOMPARE(proxy.mapToSource(proxy.index(1, 0)).row(), 1);
    QCOMPARE(proxy.mapToSource(proxy.index(2, 0)).row(), 2);
    QSignalSpy busy(&proxy, &AsyncTableProxyModel::sortingChanged);
    source.setData(source.index(1, 1), QStringLiteral("unrelated"));
    QVERIFY(!proxy.isSorting());
    QVERIFY(busy.isEmpty());
    proxy.sort(0);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.index(0, 0).data().toString(), QStringLiteral("apple"));
}
void TableTests::customKeyConversion() {
    QMetaType::registerConverter<GuiText, QString>([](const GuiText &value) {
        if (QThread::currentThread() != qApp->thread()) {
            wrongConversionThread = true;
        }
        return value.text;
    });
    QStandardItemModel source(2, 1);
    source.setData(source.index(0, 0),
                   QVariant::fromValue(GuiText{QStringLiteral("z")}));
    source.setData(source.index(1, 0),
                   QVariant::fromValue(GuiText{QStringLiteral("a")}));
    AsyncTableProxyModel proxy;
    proxy.setSourceModel(&source);
    proxy.sort(0);
    QTRY_VERIFY(!proxy.isSorting());
    QCOMPARE(proxy.mapToSource(proxy.index(0, 0)).row(), 1);
    QVERIFY(!wrongConversionThread.load());
}
void TableTests::modelState() {
    TableModel first, second;
    QTableViewExt view;
    view.resize(400, 200);
    view.setAsyncSortingEnabled(true);
    view.setModel(&first);
    view.show();
    QTest::qWait(10);
    view.setCurrentIndex(view.model()->index(500, 0));
    view.scrollTo(view.currentIndex());
    QTest::qWait(10);
    const int scroll = view.verticalScrollBar()->value();
    QVERIFY(scroll > 0);
    view.setModel(&second);
    QTest::qWait(10);
    view.setModel(&first);
    QTest::qWait(10);
    QCOMPARE(view.mapToSource(view.currentIndex()).row(), 500);
    QCOMPARE(view.verticalScrollBar()->value(), scroll);
    view.sortByColumn(0, Qt::AscendingOrder);
    QTRY_VERIFY(!view.isSorting());
    QCOMPARE(view.mapToSource(view.currentIndex()).row(), 500);
}
void TableTests::extremeWeights() {
    QStandardItemModel source(2, 3);
    QTableViewExt view;
    view.setModel(&source);
    view.resize(620, 200);
    view.horizontalHeader()->setMinimumSectionSize(10);
    QTableViewExt::ColumnWidthPolicy p;
    p.source = QTableViewExt::WidthSource::Weight;
    p.update = QTableViewExt::WidthUpdate::Continuous;
    p.weight = 1e300;
    p.maximumPixels = 100;
    view.setColumnWidthPolicy(0, p);
    p.weight = 1e-300;
    p.maximumPixels.reset();
    view.setColumnWidthPolicy(1, p);
    p.weight = 1e300;
    p.maximumPixels = 100;
    view.setColumnWidthPolicy(2, p);
    view.show();
    QTRY_COMPARE(view.columnWidth(0), 100);
    QTRY_COMPARE(view.columnWidth(2), 100);
    QTRY_VERIFY(
        std::abs(view.columnWidth(1) - (view.viewport()->width() - 200)) <= 1);
    QSignalSpy adjusted(&view, &QTableViewExt::columnWidthsAdjusted);
    QTest::qWait(50);
    QVERIFY2(adjusted.size() < 5,
             "Column allocation must settle instead of looping");
}
void TableTests::searchOptions() {
    QStandardItemModel source(3, 2);
    source.setData(source.index(0, 0), QStringLiteral("Alpha"));
    source.setData(source.index(1, 0), QStringLiteral("alpha"));
    source.setData(source.index(2, 1), QStringLiteral("<b>other</b>"));
    source.setData(source.index(2, 1), QStringLiteral("plain"), Qt::UserRole);
    QTableViewExt view;
    view.setModel(&source);
    view.setSearchEnabled(true);
    view.setColumnSearchRole(1, Qt::UserRole);
    view.resize(320, 160);
    view.show();
    view.activateWindow();
    view.setFocus();
    QTest::qWait(10);
    QTest::keyClick(&view, Qt::Key_F, Qt::ControlModifier);
    if (const auto path = qEnvironmentVariable("WING_TABLE_PREVIEW");
        !path.isEmpty()) {
        QTest::qWait(20);
        QVERIFY(view.grab().save(path));
    }
    auto input = view.findChild<QLineEdit *>();
    QVERIFY(input->hasFocus());
    QVERIFY(view.viewport()->geometry().bottom() <
            view.findChild<QWidget *>(QStringLiteral("tableSearchBar"))
                ->geometry()
                .top());
    QSignalSpy found(&view, &QTableViewExt::searchFinished);
    view.findChild<QAction *>(QStringLiteral("matchCase"))->setChecked(true);
    view.setSearchText(QStringLiteral("alpha"));
    view.findNext();
    QTRY_COMPARE(found.size(), 1);
    QCOMPARE(view.currentIndex(), source.index(1, 0));
    view.findChild<QAction *>(QStringLiteral("wrapAround"))->setChecked(false);
    view.findNext();
    QTRY_COMPARE(found.size(), 2);
    QVERIFY(!found.last().first().value<QModelIndex>().isValid());
    view.setSearchText(QStringLiteral("plain"));
    view.findNext();
    QTRY_COMPARE(found.size(), 3);
    QCOMPARE(view.currentIndex(), source.index(2, 1));
    view.setColumnHidden(1, true);
    view.findNext();
    QTRY_COMPARE(found.size(), 4);
    QVERIFY(!found.last().first().value<QModelIndex>().isValid());
    view.setColumnHidden(1, false);
    view.setRowHidden(2, true);
    view.findNext();
    QTRY_COMPARE(found.size(), 5);
    QVERIFY(!found.last().first().value<QModelIndex>().isValid());
    view.setRowHidden(2, false);
    view.findChild<QAction *>(QStringLiteral("wrapAround"))->setChecked(true);
    view.horizontalHeader()->setSectionsMovable(true);
    view.horizontalHeader()->moveSection(1, 0);
    view.setCurrentIndex({});
    view.setSearchText(QStringLiteral("plain"));
    view.findNext();
    QTRY_COMPARE(found.size(), 6);
    QCOMPARE(view.currentIndex(), source.index(2, 1));
    // Focusing the search field does not discard a match selection.
    view.setFocus();
    input->setFocus();
    QVERIFY(view.selectionModel()->hasSelection());
    QTest::keyClick(input, Qt::Key_Escape);
    QVERIFY(view.findChild<QWidget *>(QStringLiteral("tableSearchBar"))
                ->isHidden());
    view.setSearchEnabled(false);
    view.showSearch();
    QVERIFY(view.findChild<QWidget *>(QStringLiteral("tableSearchBar"))
                ->isHidden());
}
void TableTests::shortcutPrecedence() {
    QWidget window;
    QVBoxLayout layout(&window);
    QTableViewExt view;
    layout.addWidget(&view);
    QStandardItemModel source(2, 1);
    source.setData(source.index(0, 0), QStringLiteral("match"));
    source.setData(source.index(1, 0), QStringLiteral("match"));
    view.setModel(&source);
    view.setSearchEnabled(true);
    bool globalFind = false;
    QShortcut globalShortcut(QKeySequence::Find, &window);
    connect(&globalShortcut, &QShortcut::activated, this,
            [&] { globalFind = true; });
    window.show();
    window.activateWindow();
    view.setFocus();
    QTest::qWait(10);
    QTest::keyClick(&view, Qt::Key_F, Qt::ControlModifier);
    QVERIFY(view.findChild<QWidget *>(QStringLiteral("tableSearchBar"))
                ->isVisible());
    QVERIFY(!globalFind);
    view.setSearchText(QStringLiteral("match"));
    view.setCurrentIndex({});
    QSignalSpy found(&view, &QTableViewExt::searchFinished);
    auto input = view.findChild<QLineEdit *>();
    QTest::keyClick(input, Qt::Key_F3);
    QTRY_COMPARE(found.size(), 1);
    QCOMPARE(view.currentIndex().row(), 0);
    QTest::keyClick(input, Qt::Key_F3);
    QTRY_COMPARE(found.size(), 2);
    QCOMPARE(view.currentIndex().row(), 1);
    QTest::keyClick(input, Qt::Key_F3, Qt::ShiftModifier);
    QTRY_COMPARE(found.size(), 3);
    QCOMPARE(view.currentIndex().row(), 0);
    view.setSearchEnabled(false);
    view.setFocus();
    QTest::keyClick(&view, Qt::Key_F, Qt::ControlModifier);
    QVERIFY(globalFind);
}
void TableTests::search() {
    QStandardItemModel source(3, 2);
    source.setData(source.index(0, 0), QStringLiteral("Alpha"));
    source.setData(source.index(1, 0), QStringLiteral("alphabet"));
    source.setData(source.index(2, 1), QStringLiteral("Alpha"));
    QTableViewExt view;
    view.setModel(&source);
    view.setSearchEnabled(true);
    view.resize(420, 220);
    view.show();
    view.showSearch();
    QSignalSpy found(&view, &QTableViewExt::searchFinished);
    view.setSearchText(QStringLiteral("alpha"));
    view.findNext();
    QTRY_COMPARE(found.size(), 1);
    QCOMPARE(view.currentIndex(), source.index(0, 0));
    view.findNext();
    QTRY_COMPARE(found.size(), 2);
    QCOMPARE(view.currentIndex(), source.index(1, 0));
    view.findChild<QAction *>(QStringLiteral("wholeWords"))->setChecked(true);
    view.findNext();
    QTRY_COMPARE(found.size(), 3);
    QCOMPARE(view.currentIndex(), source.index(2, 1));
    view.setSearchableColumns({0});
    view.findPrevious();
    QTRY_COMPARE(found.size(), 4);
    QCOMPARE(view.currentIndex(), source.index(0, 0));
    view.setSearchableColumns({});
    view.findNext();
    QTRY_COMPARE(found.size(), 5);
    QVERIFY(!found.last().first().value<QModelIndex>().isValid());
    view.resetSearchableColumns();
    view.findChild<QAction *>(QStringLiteral("regularExpressions"))
        ->setChecked(true);
    view.setSearchText(QStringLiteral("["));
    view.findNext();
    QVERIFY(view.findChild<QLabel *>(QStringLiteral("tableSearchStatus"))
                ->toolTip()
                .startsWith(QStringLiteral("Invalid")));
    if (const auto path = qEnvironmentVariable("WING_TABLE_PREVIEW");
        !path.isEmpty()) {
        QTest::qWait(20);
        QVERIFY(view.grab().save(path));
    }
    auto input = view.findChild<QLineEdit *>();
    QVERIFY(input->hasFocus() || !QApplication::activeWindow());
    QTest::keyClick(input, Qt::Key_Escape);
    QVERIFY(view.findChild<QWidget *>(QStringLiteral("tableSearchBar"))
                ->isHidden());
}
void TableTests::resultColumnsFill() {
    QStandardItemModel source(0, 5);
    QTableViewExt view;
    view.setModel(&source);
    view.horizontalHeader()->setMinimumSectionSize(30);
    view.horizontalHeader()->setStretchLastSection(true);
    view.resize(800, 200);
    QTableViewExt::ColumnWidthPolicy fixed;
    fixed.source = QTableViewExt::WidthSource::Specified;
    fixed.value = 80;
    for (int column = 0; column < 3; ++column) {
        view.setColumnWidthPolicy(column, fixed);
    }
    QTableViewExt::ColumnWidthPolicy shared;
    shared.source = QTableViewExt::WidthSource::Weight;
    shared.update = QTableViewExt::WidthUpdate::Continuous;
    view.setColumnWidthPolicy(3, shared);
    view.setColumnWidthPolicy(4, shared);
    view.setColumnHidden(4, true);
    view.show();
    source.insertRow(0);
    QTRY_COMPARE(view.columnWidth(3), view.viewport()->width() - 240);
    view.setColumnHidden(4, false);
    QTRY_VERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
    QTRY_COMPARE(view.columnWidth(3) + view.columnWidth(4),
                 view.viewport()->width() - 240);
    view.resize(1000, 200);
    QTRY_COMPARE(view.columnWidth(3) + view.columnWidth(4),
                 view.viewport()->width() - 240);
    QVERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
    view.setColumnHidden(4, true);
    QTRY_COMPARE(view.columnWidth(3), view.viewport()->width() - 240);
    view.setColumnHidden(4, false);
    QTRY_VERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
}
void TableTests::emptyResultColumnsFill() {
    QStandardItemModel source(0, 5);
    QTableViewExt view;
    view.setAsyncSortingEnabled(true);
    view.setModel(&source);
    view.horizontalHeader()->setMinimumSectionSize(30);
    view.horizontalHeader()->setStretchLastSection(true);
    view.resize(800, 200);
    QTableViewExt::ColumnWidthPolicy contents;
    contents.maximumRatio = 0.25;
    for (int column = 0; column < 3; ++column) {
        view.setColumnWidthPolicy(column, contents);
    }
    QTableViewExt::ColumnWidthPolicy shared;
    shared.source = QTableViewExt::WidthSource::Weight;
    shared.update = QTableViewExt::WidthUpdate::Continuous;
    view.setColumnWidthPolicy(3, shared);
    view.setColumnWidthPolicy(4, shared);
    const auto totalWidth = [&view] {
        int total = 0;
        for (int column = 0; column < 5; ++column) {
            if (!view.isColumnHidden(column)) {
                total += view.columnWidth(column);
            }
        }
        return total;
    };
    view.show();
    // Startup has headers but no rows. It must already fill the viewport.
    QTRY_COMPARE_WITH_TIMEOUT(totalWidth(), view.viewport()->width(), 1000);
    QVERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
    const int headerWidth = view.columnWidth(0);
    view.resize(1000, 200);
    QTRY_COMPARE(totalWidth(), view.viewport()->width());
    QVERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
    view.setColumnHidden(4, true);
    QTRY_COMPARE(totalWidth(), view.viewport()->width());
    view.setColumnHidden(4, false);
    QTRY_COMPARE(totalWidth(), view.viewport()->width());
    QTRY_VERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
    // Header-only layout must not consume the one-time content measurement.
    source.insertRow(0);
    source.setData(source.index(0, 0), QString(100, QChar('W')));
    QTRY_VERIFY(view.columnWidth(0) > headerWidth);
    QTRY_COMPARE(totalWidth(), view.viewport()->width());
    source.removeRow(0);
    view.resize(900, 200);
    QTRY_COMPARE(totalWidth(), view.viewport()->width());
    QVERIFY(std::abs(view.columnWidth(3) - view.columnWidth(4)) <= 1);
}
void TableTests::searchBarPresentation() {
    QStandardItemModel source(2, 2);
    source.setData(source.index(0, 0), QStringLiteral("Alpha"));
    QTableViewExt view;
    view.setModel(&source);
    view.setSearchEnabled(true);
    view.resize(300, 180);
    view.show();
    view.showSearch();
    auto bar = view.findChild<QWidget *>(QStringLiteral("tableSearchBar"));
    auto input =
        view.findChild<QLineEdit *>(QStringLiteral("tableSearchInput"));
    auto status = view.findChild<QLabel *>(QStringLiteral("tableSearchStatus"));
    QTest::qWait(20);
    const auto buttons =
        bar->findChildren<QToolButton *>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(buttons.size(), 4);
    const int baselineHeight = bar->height();
    for (auto button : buttons) {
        QVERIFY(button->text().isEmpty());
        QVERIFY(!button->icon().isNull());
        QVERIFY(!button->toolTip().isEmpty());
        QVERIFY(!button->accessibleName().isEmpty());
        QCOMPARE(button->size(), buttons.first()->size());
        QCOMPARE(button->width(), button->height());
        QCOMPARE(button->iconSize(), buttons.first()->iconSize());
        QCOMPARE(button->geometry().center().y(),
                 buttons.first()->geometry().center().y());
    }
    QSignalSpy found(&view, &QTableViewExt::searchFinished);
    view.setSearchText(QStringLiteral("missing"));
    view.findNext();
    QTRY_COMPARE(found.size(), 1);
    QVERIFY(input->property("searchError").toBool());
    QVERIFY(!input->styleSheet().contains(QStringLiteral("background-color")));
    QVERIFY(status->text().isEmpty());
    QVERIFY(!status->pixmap().isNull());
    QCOMPARE(input->accessibleDescription(), QStringLiteral("No matches"));
    QCOMPARE(status->toolTip(), QStringLiteral("No matches"));
    QTest::qWait(10);
    QCOMPARE(bar->height(), baselineHeight);
    view.findChild<QAction *>(QStringLiteral("regularExpressions"))
        ->setChecked(true);
    view.setSearchText(QStringLiteral("["));
    view.findNext();
    QVERIFY(input->property("searchError").toBool());
    QVERIFY(status->toolTip().startsWith(QStringLiteral("Invalid")));
    view.setSearchText({});
    view.findNext();
    QVERIFY(!input->property("searchError").toBool());
    QVERIFY(!input->styleSheet().contains(QStringLiteral("background-color")));
    QVERIFY(status->isHidden());
    QTest::qWait(10);
    QCOMPARE(bar->height(), baselineHeight);
}
void TableTests::searchWithScrollBars() {
    QStandardItemModel source(200, 3);
    QTableViewExt view;
    view.setModel(&source);
    view.setSearchEnabled(true);
    view.resize(360, 220);
    for (int c = 0; c < 3; ++c) {
        view.setColumnWidth(c, 300);
    }
    view.show();
    view.activateWindow();
    view.showSearch();
    auto bar = view.findChild<QWidget *>(QStringLiteral("tableSearchBar"));
    const auto rectInView = [&view](QWidget *widget) {
        return QRect(widget->mapTo(&view, QPoint()), widget->size());
    };
    const auto uncovered = [&] {
        const auto search = rectInView(bar);
        for (auto scroll :
             {view.horizontalScrollBar(), view.verticalScrollBar()}) {
            if (scroll->isVisible() && search.intersects(rectInView(scroll))) {
                return false;
            }
        }
        return !search.intersects(view.viewport()->geometry());
    };
    QTRY_VERIFY(view.horizontalScrollBar()->isVisible());
    QTRY_VERIFY(view.verticalScrollBar()->isVisible());
    QTRY_VERIFY_WITH_TIMEOUT(uncovered(), 1000);
    const auto close =
        view.findChild<QToolButton *>(QStringLiteral("tableSearchClose"));
    QCOMPARE(view.childAt(close->mapTo(&view, close->rect().center())), close);
    for (int c = 0; c < 3; ++c) {
        view.setColumnWidth(c, 50);
    }
    QTRY_VERIFY(!view.horizontalScrollBar()->isVisible());
    QTRY_VERIFY(uncovered());
    view.setLayoutDirection(Qt::RightToLeft);
    QTRY_VERIFY(uncovered());
    for (int c = 0; c < 3; ++c) {
        view.setColumnWidth(c, 300);
    }
    QTRY_VERIFY(view.horizontalScrollBar()->isVisible());
    QTRY_VERIFY(uncovered());
    QTest::mouseClick(close, Qt::LeftButton);
    QVERIFY(bar->isHidden());
}
void TableTests::treeColumnRatios() {
    QStandardItemModel source(0, 2);
    QTreeView tree;
    tree.setModel(&source);
    tree.header()->setMinimumSectionSize(30);
    tree.resize(600, 220);
    auto widths = new TableColumnWidthController(&tree);
    QTableViewExt::ColumnWidthPolicy policy;
    policy.source = QTableViewExt::WidthSource::Weight;
    policy.update = QTableViewExt::WidthUpdate::Continuous;
    policy.weight = 2;
    widths->setPolicy(0, policy);
    policy.weight = 3;
    widths->setPolicy(1, policy);
    tree.show();
    QTRY_COMPARE(tree.columnWidth(0) + tree.columnWidth(1),
                 tree.viewport()->width());
    QTRY_VERIFY(std::abs(tree.columnWidth(0) * 3 - tree.columnWidth(1) * 2) <=
                5);
    source.insertRows(0, 100);
    QTRY_VERIFY(tree.verticalScrollBar()->isVisible());
    QTRY_COMPARE(tree.columnWidth(0) + tree.columnWidth(1),
                 tree.viewport()->width());
    tree.resize(800, 220);
    QTRY_COMPARE(tree.columnWidth(0) + tree.columnWidth(1),
                 tree.viewport()->width());
    const auto header = tree.header();
    const QPoint edge(
        header->sectionViewportPosition(0) + tree.columnWidth(0) - 1, 10);
    QTest::mousePress(header->viewport(), Qt::LeftButton, Qt::NoModifier, edge);
    QTest::mouseMove(header->viewport(), edge - QPoint(40, 0));
    QTest::mouseRelease(header->viewport(), Qt::LeftButton, Qt::NoModifier,
                        edge - QPoint(40, 0));
    const int manual = tree.columnWidth(0);
    tree.resize(900, 220);
    QTest::qWait(20);
    QCOMPARE(tree.columnWidth(0), manual);
    QTRY_COMPARE(tree.columnWidth(0) + tree.columnWidth(1),
                 tree.viewport()->width());
    widths->reset(0);
    QTRY_VERIFY(std::abs(tree.columnWidth(0) * 3 - tree.columnWidth(1) * 2) <=
                5);
    QStandardItemModel other(1, 2);
    tree.setModel(&other);
    tree.resize(700, 220);
    QTRY_COMPARE(tree.columnWidth(0) + tree.columnWidth(1),
                 tree.viewport()->width());
    QTRY_VERIFY(std::abs(tree.columnWidth(0) * 3 - tree.columnWidth(1) * 2) <=
                5);
}
void TableTests::searchBarHeightConsistency() {
    const QString paths[] = {QFINDTESTDATA("../../theme/light/stylesheet.qss"),
                             QFINDTESTDATA("../../theme/dark/stylesheet.qss")};
    for (const auto &path : paths) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto theme = QString::fromUtf8(file.readAll());
        // Exercise the real input and cell-editor rules without unrelated
        // icons.
        QRegularExpression rules(QStringLiteral(
            R"((?:QLineEdit|QAbstractItemView[^{}]*QLineEdit|SearchBar[^{}]*)\s*\{[^{}]*\})"));
        QString style;
        auto matches = rules.globalMatch(theme);
        while (matches.hasNext()) {
            style += matches.next().captured();
        }
        QVERIFY(!style.isEmpty());
        QTableViewExt table;
        SearchableTextBrowser browser;
        SearchableTreeWidget tree;
        tree.setStyleSheet(style);
        tree.resize(420, 200);
        tree.show();
        tree.showSearch();
        table.setStyleSheet(style);
        browser.setStyleSheet(style);
        table.setSearchEnabled(true);
        browser.setSearchEnabled(true);
        table.resize(420, 200);
        browser.resize(420, 200);
        table.show();
        browser.show();
        table.showSearch();
        browser.showSearch();
        auto tableBar = table.findChild<SearchBar *>();
        QVERIFY(tableBar);
        auto textBar = browser.searchBar();
        QTRY_COMPARE(tableBar->height(), textBar->height());
        QTRY_COMPARE(tree.searchBar()->height(), textBar->height());
        QCOMPARE(tableBar->lineEdit()->height(), textBar->lineEdit()->height());
        QCOMPARE(tableBar->lineEdit()->sizeHint().height(),
                 textBar->lineEdit()->sizeHint().height());
        const auto normalBase =
            textBar->lineEdit()->palette().color(QPalette::Base);
        const int height = textBar->height();
        textBar->setState(SearchBar::State::NoMatch);
        QCOMPARE(textBar->lineEdit()->toolTip(), QStringLiteral("No matches"));
        QVERIFY(textBar->lineEdit()->palette().color(QPalette::Base) !=
                normalBase);
        textBar->setState(SearchBar::State::Searching);
        QCOMPARE(textBar->lineEdit()->toolTip(),
                 QStringLiteral("Searching..."));
        QCOMPARE(textBar->lineEdit()->palette().color(QPalette::Base),
                 normalBase);
        textBar->setState(SearchBar::State::Idle);
        QCOMPARE(textBar->height(), height);
        // Actual cell editors retain their compact appearance.
        auto editor = new QLineEdit(table.viewport());
        editor->ensurePolished();
        QCOMPARE(tableBar->lineEdit()->sizeHint().height(),
                 editor->sizeHint().height() +
                     tableBar->lineEdit()->textMargins().top() +
                     tableBar->lineEdit()->textMargins().bottom());
    }
}

void TableTests::centeredHeaders() {
    QStandardItemModel source(1, 1);
    source.setHeaderData(0, Qt::Horizontal, QStringLiteral("CENTER"));
    QTableViewExt view;
    view.setModel(&source);
    view.resize(380, 180);
    auto header = view.horizontalHeader();
    header->setDefaultAlignment(Qt::AlignCenter);
    header->setHighlightSections(false);
    header->setStretchLastSection(false);
    view.setColumnWidth(0, 320);
    view.show();
    const QString paths[] = {QFINDTESTDATA("../../theme/light/stylesheet.qss"),
                             QFINDTESTDATA("../../theme/dark/stylesheet.qss")};
    for (int theme = 0; theme < 2; ++theme) {
        QFile file(paths[theme]);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto stylesheet = QString::fromUtf8(file.readAll());
        const QRegularExpression rule(
            QStringLiteral(R"(QHeaderView[^{}]*\{[^{}]*\})"));
        auto matches = rule.globalMatch(stylesheet);
        QString headerStyles;
        while (matches.hasNext()) {
            headerStyles += matches.next().captured() + QChar('\n');
        }
        QVERIFY(!headerStyles.isEmpty());
        view.setStyleSheet(headerStyles);
        for (bool sorted : {false, true}) {
            header->setSortIndicator(0, Qt::AscendingOrder);
            header->setSortIndicatorShown(sorted);
            QTest::qWait(10);
            const auto pixels = header->grab();
            const auto image = pixels.toImage();
            const qreal scale = pixels.devicePixelRatioF();
            const int left = header->sectionViewportPosition(0);
            const int width = header->sectionSize(0);
            QRect ink;
            for (int y = int(2 * scale);
                 y < int((header->height() - 2) * scale); ++y) {
                for (int x = int((left + 30) * scale);
                     x < int((left + width - 30) * scale); ++x) {
                    const auto color = image.pixelColor(x, y);
                    const bool isText = theme == 0 ? color.lightness() < 90
                                                   : color.lightness() > 180;
                    if (isText) {
                        ink |= QRect(x, y, 1, 1);
                    }
                }
            }
            QVERIFY(!ink.isEmpty());
            const qreal center = (ink.left() + ink.right() + 1) / (2 * scale);
            const qreal expected = left + width / 2.;
            QVERIFY2(
                std::abs(center - expected) <= 2,
                qPrintable(
                    QStringLiteral(
                        "theme %1 sorted %2: text center %3, section center %4")
                        .arg(theme)
                        .arg(sorted)
                        .arg(center)
                        .arg(expected)));
        }
    }
}
void TableTests::uniformIconInk() {
    QTableViewExt view;
    view.setSearchEnabled(true);
    auto bar = view.findChild<QWidget *>(QStringLiteral("tableSearchBar"));
    const auto buttons =
        bar->findChildren<QToolButton *>(QString(), Qt::FindDirectChildrenOnly);
    QRect reference;
    for (auto button : buttons) {
        const auto pixmap = button->icon().pixmap(QSize(16, 16), 2.0);
        QCOMPARE(pixmap.devicePixelRatioF(), qreal(2));
        QCOMPARE(pixmap.size(), QSize(32, 32));
        const auto image = pixmap.toImage();
        QRect ink;
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                if (image.pixelColor(x, y).alpha() >= 32) {
                    ink |= QRect(x, y, 1, 1);
                }
            }
        }
        QVERIFY(!ink.isEmpty());
        if (reference.isEmpty()) {
            reference = ink;
        }
        QVERIFY(std::abs(ink.width() - reference.width()) <= 2);
        QVERIFY(std::abs(ink.height() - reference.height()) <= 2);
        QVERIFY(std::abs(ink.center().x() - reference.center().x()) <= 1);
        QVERIFY(std::abs(ink.center().y() - reference.center().y()) <= 1);
    }
}
void TableTests::wrappingDiagnostics() {
    QStandardItemModel source;
    source.appendRow(new QStandardItem(
        QStringLiteral("Diagnostic text with details. ").repeated(20)));
    source.appendRow(new QStandardItem(QString(180, QChar('x'))));
    QListView view;
    view.setModel(&source);
    view.setItemDelegate(new WrappingItemDelegate(&view));
    view.setWordWrap(true);
    view.setResizeMode(QListView::Adjust);
    view.setSpacing(3);
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.resize(400, 220);
    view.show();
    for (int r = 0; r < 2; ++r) {
        QTRY_VERIFY(view.visualRect(source.index(r, 0)).width() <=
                    view.viewport()->width());
        QVERIFY(view.visualRect(source.index(r, 0)).height() >
                view.fontMetrics().height());
    }
    QTest::qWait(30);
    const auto previous = view.visualRect(source.index(1, 0)).height();
    view.resize(250, 220);
    QTest::qWait(30);
    QTRY_VERIFY(view.visualRect(source.index(1, 0)).height() > previous);
    QTRY_VERIFY(view.visualRect(source.index(1, 0)).width() <=
                view.viewport()->width());
}
namespace {
class LazySymbolTree : public SearchableTreeWidget {
public:
    int loads = 0;
    QTreeWidgetItem *module;
    LazySymbolTree() {
        setColumnCount(2);
        auto category =
            new QTreeWidgetItem(this, {QStringLiteral("Symbol group")});
        category->setData(0, Qt::UserRole, QStringLiteral("category"));
        module = new QTreeWidgetItem(category, {QStringLiteral("Module")});
        module->setData(0, Qt::UserRole, QStringLiteral("pending"));
        module->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        setSortingEnabled(true);
        sortItems(0, Qt::AscendingOrder);
    }

protected:
    void prepareSearchItem(QTreeWidgetItem *item) override {
        if (item->data(0, Qt::UserRole).toString() !=
            QStringLiteral("pending")) {
            return;
        }
        ++loads;
        item->setData(0, Qt::UserRole, QStringLiteral("loaded"));
        if (item == module) {
            auto child =
                new QTreeWidgetItem(item, {QStringLiteral("Namespace")});
            child->setData(0, Qt::UserRole, QStringLiteral("pending"));
            child->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        } else {
            for (const auto &name :
                 {QStringLiteral("Alpha"), QStringLiteral("alphabet"),
                  QStringLiteral("ALPHA")}) {
                new QTreeWidgetItem(item,
                                    {name, QStringLiteral("signatureOnly")});
            }
            auto hidden =
                new QTreeWidgetItem(item, {QStringLiteral("HiddenNeedle")});
            hidden->setHidden(true);
        }
    }
    bool isSearchableItem(const QTreeWidgetItem *item) const override {
        return item->data(0, Qt::UserRole).toString() !=
               QStringLiteral("category");
    }
};
} // namespace

void TableTests::lazyTreeSearch() {
    LazySymbolTree tree;
    tree.resize(420, 220);
    tree.show();
    tree.setFocus();
    QTest::keyClick(&tree, Qt::Key_F, Qt::ControlModifier);
    auto bar = tree.searchBar();
    QVERIFY(bar->isVisible());
    QCOMPARE(tree.loads, 0);
    QSignalSpy found(&tree, &SearchableTreeWidget::searchFinished);
    bar->setSearchText(QStringLiteral("Alpha"));
    tree.findNext();
    QTRY_COMPARE(found.size(), 1);
    QCOMPARE(tree.loads, 2);
    QVERIFY(tree.currentItem()->text(0).compare(QStringLiteral("alpha"),
                                                Qt::CaseInsensitive) == 0);
    QVERIFY(tree.currentItem()->parent()->isExpanded());
    auto first = tree.currentItem();
    bar->lineEdit()->setFocus();
    QVERIFY(first->isSelected());
    tree.findNext();
    QTRY_COMPARE(found.size(), 2);
    QVERIFY(tree.currentItem() != first);
    tree.findPrevious();
    QTRY_COMPARE(found.size(), 3);
    QCOMPARE(tree.currentItem(), first);
    auto options = bar->options();
    options.caseSensitivity = Qt::CaseSensitive;
    options.wholeWords = true;
    bar->setOptions(options);
    bar->setSearchText(QStringLiteral("Alpha"));
    tree.findNext();
    QTRY_COMPARE(found.size(), 4);
    QCOMPARE(tree.currentItem()->text(0), QStringLiteral("Alpha"));
    for (const auto &query :
         {QStringLiteral("Symbol group"), QStringLiteral("signatureOnly"),
          QStringLiteral("HiddenNeedle")}) {
        bar->setSearchText(query);
        tree.findNext();
        QTRY_COMPARE(bar->state(), SearchBar::State::NoMatch);
    }
    options.wrap = false;
    options.regularExpression = true;
    bar->setOptions(options);
    bar->setSearchText(QStringLiteral("["));
    tree.findNext();
    QCOMPARE(bar->state(), SearchBar::State::InvalidPattern);
    tree.setCurrentItem(tree.topLevelItem(0));
    bar->setSearchText(QStringLiteral("Alpha"));
    tree.findPrevious();
    QTRY_COMPARE(bar->state(), SearchBar::State::NoMatch);
    // A reset cancels pending scans and input debounce before deleting items.
    tree.findNext();
    tree.clear();
    QTest::qWait(180);
    QCOMPARE(bar->state(), SearchBar::State::Idle);
    QTest::keyClick(bar->lineEdit(), Qt::Key_Escape);
    QVERIFY(bar->isHidden());
    // Destruction must also cancel a queued search.
    auto pending = new LazySymbolTree;
    pending->show();
    pending->showSearch();
    pending->searchBar()->setSearchText(QStringLiteral("missing"));
    pending->findNext();
    delete pending;
    QCoreApplication::processEvents();
}
void TableTests::treeSearchScrollBars() {
    SearchableTreeWidget tree;
    tree.setColumnCount(2);
    tree.setColumnWidth(0, 500);
    tree.setColumnWidth(1, 500);
    for (int row = 0; row < 100; ++row) {
        new QTreeWidgetItem(&tree, {QString::number(row)});
    }
    tree.resize(360, 220);
    tree.show();
    tree.showSearch();
    QTRY_VERIFY(tree.horizontalScrollBar()->isVisible());
    QTRY_VERIFY(tree.verticalScrollBar()->isVisible());
    for (auto direction : {Qt::LeftToRight, Qt::RightToLeft}) {
        tree.setLayoutDirection(direction);
        QCoreApplication::processEvents();
        const QRect bar(tree.searchBar()->mapTo(&tree, QPoint()),
                        tree.searchBar()->size());
        QVERIFY(!bar.intersects(tree.viewport()->geometry()));
        for (auto scroll :
             {tree.horizontalScrollBar(), tree.verticalScrollBar()}) {
            QVERIFY(!bar.intersects(
                QRect(scroll->mapTo(&tree, QPoint()), scroll->size())));
        }
    }
}

void TableTests::reusableSearchBar() {
    QWidget window;
    QHBoxLayout layout(&window);
    SearchBar bar(&window);
    layout.addWidget(&bar);
    window.resize(420, 120);
    window.show();
    bar.activate();
    QSignalSpy requested(&bar, &SearchBar::searchRequested);
    QSignalSpy close(&bar, &SearchBar::closeRequested);
    bar.setSearchText(QStringLiteral("first"));
    bar.setSearchText(QStringLiteral("second"));
    QTRY_COMPARE(requested.size(), 1);
    QVERIFY(!requested.first().first().toBool());
    const auto margins = bar.layout()->contentsMargins();
    QVERIFY(margins.left() >= 8 && margins.right() >= 8);
    QVERIFY(margins.top() >= 6 && margins.bottom() >= 6);
    QVERIFY(bar.lineEdit()->textMargins().left() >= 6);
    QVERIFY(bar.lineEdit()->textMargins().top() >= 3);
    SearchBar::Options options;
    options.caseSensitivity = Qt::CaseSensitive;
    options.wholeWords = true;
    options.wrap = false;
    bar.setOptions(options);
    bar.cancelPendingSearch();
    QCOMPARE(bar.options().caseSensitivity, Qt::CaseSensitive);
    QVERIFY(bar.options().wholeWords);
    QVERIFY(!bar.options().wrap);
    QTest::keyClick(bar.lineEdit(), Qt::Key_Return, Qt::ShiftModifier);
    QTRY_COMPARE(requested.size(), 2);
    QVERIFY(requested.last().first().toBool());
    for (auto button : bar.findChildren<QToolButton *>()) {
        if (button->parentWidget() != &bar) {
            continue;
        }
        const auto count = requested.size();
        QTest::keyClick(button, Qt::Key_F3);
        QCOMPARE(requested.size(), count + 1);
    }
    QTest::keyClick(bar.lineEdit(), Qt::Key_Escape);
    QCOMPARE(close.size(), 1);
}
void TableTests::decodedTextSearch() {
    SearchableTextBrowser browser;
    browser.setSearchEnabled(true);
    browser.setHtml(QStringLiteral("<b>Alpha</b> alphabet<br>Alpha"));
    const auto original = browser.toHtml();
    browser.resize(420, 220);
    browser.show();
    browser.activateWindow();
    browser.setFocus();
    QTest::qWait(10);
    QTest::keyClick(&browser, Qt::Key_F, Qt::ControlModifier);
    QVERIFY(browser.searchBar()->isVisible());
    browser.setSearchText(QStringLiteral("alpha"));
    QSignalSpy found(&browser, &SearchableTextBrowser::searchFinished);
    browser.findNext();
    QTRY_COMPARE(found.size(), 1);
    QCOMPARE(browser.textCursor().selectedText(), QStringLiteral("Alpha"));
    browser.findNext();
    QTRY_COMPARE(found.size(), 2);
    QCOMPARE(browser.textCursor().selectedText(), QStringLiteral("alpha"));
    auto options = browser.searchBar()->options();
    options.wholeWords = true;
    browser.searchBar()->setOptions(options);
    browser.findNext();
    QTRY_COMPARE(found.size(), 3);
    QCOMPARE(browser.textCursor().selectionStart(), 15);
    QCOMPARE(browser.toHtml(), original);
    options.wrap = false;
    browser.searchBar()->setOptions(options);
    browser.findNext();
    QTRY_COMPARE(found.size(), 4);
    QCOMPARE(browser.searchBar()->state(), SearchBar::State::NoMatch);
    options.wrap = true;
    options.regularExpression = true;
    browser.searchBar()->setOptions(options);
    browser.setSearchText(QStringLiteral("["));
    browser.findNext();
    QCOMPARE(browser.searchBar()->state(), SearchBar::State::InvalidPattern);
    browser.setSearchText(QStringLiteral("Alpha"));
    browser.findPrevious();
    QTRY_COMPARE(found.size(), 5);
    QCOMPARE(browser.textCursor().selectedText(), QStringLiteral("Alpha"));
    browser.setPlainText(QStringLiteral("new decoded value"));
    QCOMPARE(browser.searchBar()->state(), SearchBar::State::Idle);
    QVERIFY(browser.extraSelections().isEmpty());
    auto replacement = new QTextDocument(&browser);
    replacement->setPlainText(QStringLiteral("replacement document"));
    browser.setDocument(replacement);
    browser.setSearchText(QStringLiteral("replacement"));
    browser.findNext();
    QTRY_COMPARE(found.size(), 6);
    QCOMPARE(browser.textCursor().selectedText(),
             QStringLiteral("replacement"));
    browser.setSearchText(QStringLiteral("pending"));
    browser.setSearchEnabled(false);
    QTest::qWait(180);
    QCOMPARE(found.size(), 6);
    browser.setSearchEnabled(true);
    browser.showSearch();
    QTest::keyClick(browser.searchBar()->lineEdit(), Qt::Key_Escape);
    QVERIFY(browser.searchBar()->isHidden());
}
void TableTests::textSearchScrollBars() {
    SearchableTextBrowser browser;
    browser.setLineWrapMode(QTextEdit::NoWrap);
    browser.setPlainText(
        (QString(300, QChar('x')) + QChar('\n')).repeated(100));
    browser.setSearchEnabled(true);
    browser.resize(360, 220);
    browser.show();
    browser.showSearch();
    QTRY_VERIFY(browser.horizontalScrollBar()->isVisible());
    QTRY_VERIFY(browser.verticalScrollBar()->isVisible());
    const auto uncovered = [&] {
        auto bar = browser.searchBar();
        const QRect rect(bar->mapTo(&browser, QPoint()), bar->size());
        for (auto scroll :
             {browser.horizontalScrollBar(), browser.verticalScrollBar()}) {
            if (scroll->isVisible() &&
                rect.intersects(
                    QRect(scroll->mapTo(&browser, QPoint()), scroll->size()))) {
                return false;
            }
        }
        return !rect.intersects(browser.viewport()->geometry());
    };
    QTRY_VERIFY(uncovered());
    browser.setLayoutDirection(Qt::RightToLeft);
    QTRY_VERIFY(uncovered());
}
void TableTests::rapidEmptyResize() {
    QStandardItemModel source(0, 5);
    QTableViewExt view;
    view.setModel(&source);
    view.horizontalHeader()->setMinimumSectionSize(20);
    QTableViewExt::ColumnWidthPolicy policy;
    for (int c = 0; c < 3; ++c) {
        view.setColumnWidthPolicy(c, policy);
    }
    policy.source = QTableViewExt::WidthSource::Weight;
    policy.update = QTableViewExt::WidthUpdate::Continuous;
    view.setColumnWidthPolicy(3, policy);
    view.setColumnWidthPolicy(4, policy);
    view.resize(900, 240);
    view.show();
    QTest::qWait(20);
    bool horizontalRange = false, verticalRange = false;
    connect(view.horizontalScrollBar(), &QScrollBar::rangeChanged, this,
            [&](int, int maximum) { horizontalRange |= maximum > 0; });
    connect(view.verticalScrollBar(), &QScrollBar::rangeChanged, this,
            [&](int, int maximum) { verticalRange |= maximum > 0; });
    for (int width = 880; width >= 300; width -= 20) {
        view.resize(width, 240);
        QApplication::processEvents();
    }
    QVERIFY(!horizontalRange);
    QVERIFY(!verticalRange);
    QVERIFY(!view.horizontalScrollBar()->isVisible());
}
void TableTests::widths() {
    QStandardItemModel source(4, 3);
    QTableViewExt view;
    view.horizontalHeader()->setMinimumSectionSize(20);
    view.resize(620, 220);
    view.setModel(&source);
    QTableViewExt::ColumnWidthPolicy fixed;
    fixed.source = QTableViewExt::WidthSource::Specified;
    fixed.value = 100;
    QVERIFY(view.setColumnWidthPolicy(0, fixed));
    auto weight = fixed;
    weight.source = QTableViewExt::WidthSource::Weight;
    weight.update = QTableViewExt::WidthUpdate::Continuous;
    weight.maximumPixels = 120;
    QVERIFY(view.setColumnWidthPolicy(1, weight));
    weight.maximumPixels.reset();
    QVERIFY(view.setColumnWidthPolicy(2, weight));
    view.show();
    QTRY_COMPARE(view.columnWidth(0), 100);
    QTRY_COMPARE(view.columnWidth(1), 120);
    QTRY_VERIFY(
        std::abs(view.columnWidth(2) - (view.viewport()->width() - 220)) <= 1);
    view.resize(820, 220);
    QTRY_VERIFY(
        std::abs(view.columnWidth(2) - (view.viewport()->width() - 220)) <= 1);
    auto contents = fixed;
    contents.source = QTableViewExt::WidthSource::Contents;
    contents.maximumRatio = .25;
    source.setData(source.index(0, 0), QString(200, QChar('W')));
    QVERIFY(view.setColumnWidthPolicy(0, contents));
    QTRY_COMPARE(view.columnWidth(0), int(view.viewport()->width() * .25));
    const int initial = view.columnWidth(0);
    view.resize(1020, 220);
    QTest::qWait(30);
    QCOMPARE(view.columnWidth(0), initial);
    QStandardItemModel other(2, 3);
    view.setModel(&other);
    QTest::qWait(30);
    view.setModel(&source);
    QTest::qWait(30);
    QCOMPARE(view.columnWidth(0), initial);
    view.resetColumnWidths(0);
    QTRY_COMPARE(view.columnWidth(0), int(view.viewport()->width() * .25));
    contents.weight = 0;
    QVERIFY(!view.setColumnWidthPolicy(0, contents));
}
void TableTests::widthConstraints() {
    QStandardItemModel source(2, 3);
    QTableViewExt view;
    view.horizontalHeader()->setMinimumSectionSize(10);
    view.setModel(&source);
    view.resize(600, 200);
    QTableViewExt::ColumnWidthPolicy p;
    p.source = QTableViewExt::WidthSource::Specified;
    p.unit = QTableViewExt::WidthUnit::ViewportRatio;
    p.value = .2;
    p.update = QTableViewExt::WidthUpdate::Continuous;
    p.minimumPixels = 90;
    p.maximumRatio = .1; // minimum wins a conflicting maximum
    view.setColumnWidthPolicy(0, p);
    p.source = QTableViewExt::WidthSource::Weight;
    p.minimumPixels = 20;
    p.maximumRatio.reset();
    p.maximumPixels = 50;
    view.setColumnWidthPolicy(1, p);
    p.weight = 3;
    p.maximumPixels.reset();
    view.setColumnWidthPolicy(2, p);
    view.show();
    QTRY_COMPARE(view.columnWidth(0), 90);
    QTRY_COMPARE(view.columnWidth(1), 50);
    QTRY_COMPARE(view.columnWidth(2), view.viewport()->width() - 140);
    // All weights capped: retain empty space.
    p.maximumPixels = 60;
    view.setColumnWidthPolicy(2, p);
    QTRY_COMPARE(view.columnWidth(2), 60);
    // Insufficient room: preserve minima and allow scrolling.
    p.minimumPixels = 800;
    view.setColumnWidthPolicy(2, p);
    QTRY_COMPARE(view.columnWidth(2), 800);
    QTRY_VERIFY(view.horizontalScrollBar()->maximum() > 0);
    p.weight = -1;
    QVERIFY(!view.setColumnWidthPolicy(2, p));
}
void TableTests::manualWidths() {
    QStandardItemModel source(2, 2), other(2, 2);
    QTableViewExt view;
    view.setModel(&source);
    view.resize(600, 180);
    QTableViewExt::ColumnWidthPolicy p;
    p.source = QTableViewExt::WidthSource::Weight;
    p.update = QTableViewExt::WidthUpdate::Continuous;
    view.setColumnWidthPolicy(0, p);
    view.setColumnWidthPolicy(1, p);
    view.show();
    QTest::qWait(20);
    QVERIFY(std::abs(view.columnWidth(0) - view.columnWidth(1)) <= 1);
    auto header = view.horizontalHeader();
    const QPoint edge(
        header->sectionViewportPosition(0) + view.columnWidth(0) - 1, 10);
    QTest::mousePress(header->viewport(), Qt::LeftButton, Qt::NoModifier, edge);
    QTest::mouseMove(header->viewport(), edge - QPoint(60, 0), 10);
    QTest::mouseRelease(header->viewport(), Qt::LeftButton, Qt::NoModifier,
                        edge - QPoint(60, 0));
    const int manual = view.columnWidth(0);
    QVERIFY(manual < 260);
    view.resize(800, 180);
    QTest::qWait(20);
    QCOMPARE(view.columnWidth(0), manual);
    QTRY_COMPARE(view.columnWidth(1), view.viewport()->width() - manual);
    view.setModel(&other);
    QTest::qWait(20);
    view.setModel(&source);
    QTest::qWait(20);
    QCOMPARE(view.columnWidth(0), manual);
    view.resetColumnWidths(0);
    QTRY_VERIFY(std::abs(view.columnWidth(0) - view.columnWidth(1)) <= 1);
    view.clearColumnWidthPolicy(0);
    QVERIFY(!view.columnWidthPolicy(0).has_value());
}
void TableTests::deferredWidths() {
    QStandardItemModel source(0, 3);
    QTableViewExt view;
    view.resize(600, 180);
    view.setModel(&source);
    QTableViewExt::ColumnWidthPolicy p;
    p.source = QTableViewExt::WidthSource::Specified;
    p.value = 140;
    view.setColumnWidthPolicy(0, p);
    p.value = 180;
    view.setColumnWidthPolicy(1, p);
    view.setColumnHidden(1, true);
    view.show();
    QTest::qWait(10);
    QVERIFY(view.columnWidth(0) != 140);
    source.insertRow(0);
    QTRY_COMPARE(view.columnWidth(0), 140);
    view.setColumnHidden(1, false);
    QTRY_COMPARE(view.columnWidth(1), 180);
    // Continuous contents remeasure after data changes; initial widths remain.
    p.source = QTableViewExt::WidthSource::Contents;
    p.update = QTableViewExt::WidthUpdate::Continuous;
    p.maximumPixels = 250;
    view.setColumnWidthPolicy(2, p);
    source.setData(source.index(0, 2), QStringLiteral("short"));
    QTest::qWait(20);
    const int before = view.columnWidth(2);
    source.setData(source.index(0, 2), QString(100, QChar('W')));
    QTRY_COMPARE(view.columnWidth(2), 250);
    QVERIFY(before < 250);
    source.clear();
    QCOMPARE(view.columnWidthPolicy(0)->value, qreal(140));
}
void TableTests::contentSampling() {
    class SamplingDelegate : public QStyledItemDelegate {
    public:
        mutable QSet<int> rows;
        mutable int calls = 0;
        QSize sizeHint(const QStyleOptionViewItem &,
                       const QModelIndex &index) const override {
            if (index.column() == 0) {
                rows.insert(index.row());
                ++calls;
            }
            return QSize(150, 20);
        }
    };
    QStandardItemModel source(2001, 2);
    QTableViewExt view;
    SamplingDelegate delegate;
    view.setModel(&source);
    view.setItemDelegate(&delegate);
    QTableViewExt::ColumnWidthPolicy policy;
    policy.update = QTableViewExt::WidthUpdate::Continuous;
    view.setColumnWidthPolicy(0, policy);
    view.resize(600, 200);
    view.show();
    QTRY_VERIFY(delegate.rows.contains(2000));
    QCOMPARE(delegate.rows.size(), 1000);
    QVERIFY(delegate.rows.contains(0));
    const int measuredCalls = delegate.calls;
    source.setData(source.index(0, 1), QStringLiteral("changed other column"));
    QTest::qWait(30);
    QCOMPARE(delegate.calls, measuredCalls);
}
void TableTests::richTextWidths() {
    QStandardItemModel source(2, 1);
    source.setData(source.index(0, 0),
                   QStringLiteral("<b>A reasonably long sentence</b>"));
    QTableViewExt view;
    view.horizontalHeader()->setMinimumSectionSize(10);
    view.setModel(&source);
    view.setItemDelegate(new RichTextItemDelegate(&view));
    QTableViewExt::ColumnWidthPolicy policy;
    policy.maximumPixels = 350;
    view.setColumnWidthPolicy(0, policy);
    view.resize(600, 200);
    view.show();
    QTRY_VERIFY(view.columnWidth(0) > 100);
    QVERIFY(view.columnWidth(0) < 350);
}
void TableTests::performance_data() {
    QTest::addColumn<int>("rows");
    QTest::addColumn<int>("column");
    for (int count : {1000, 10000, 100000}) {
        QTest::newRow(qPrintable(QStringLiteral("numeric-%1").arg(count)))
            << count << 0;
        QTest::newRow(qPrintable(QStringLiteral("text-%1").arg(count)))
            << count << 2;
    }
}
void TableTests::performance() {
    QFETCH(int, rows);
    QFETCH(int, column);
    TableModel source;
    source.count = rows;
    QTableViewExt view;
    view.setAsyncSortingEnabled(true);
    view.setModel(&source);
    view.resize(700, 300);
    view.show();
    QTest::qWait(10);
    QSignalSpy timing(qobject_cast<AsyncTableProxyModel *>(view.model()),
                      &AsyncTableProxyModel::sortingMeasured);
    QElapsedTimer elapsed, gap;
    elapsed.start();
    gap.start();
    qint64 longest = 0;
    int ticks = 0;
    QTimer heartbeat;
    heartbeat.setInterval(1);
    connect(&heartbeat, &QTimer::timeout, this, [&] {
        longest = std::max(longest, gap.restart());
        ++ticks;
    });
    heartbeat.start();
    view.model()->sort(column);
    QTRY_VERIFY_WITH_TIMEOUT(!view.isSorting(), 10000);
    QCOMPARE(timing.size(), 1);
    qInfo() << "snapshot/worker/publish ms" << timing.first();
    qInfo() << rows << "rows: total ms" << elapsed.elapsed()
            << "maximum heartbeat gap ms" << longest << "ticks" << ticks;
    if (column == 0) {
        QCOMPARE(view.model()->index(0, 0).data().toInt(), 1);
    } else {
        QCOMPARE(view.model()->index(0, column).data().toString(),
                 QStringLiteral("row 0"));
    }
    QVERIFY(!source.wrongThread);
    QVERIFY(ticks > 0);
    QVERIFY(longest < 150);
}
QTEST_MAIN(TableTests)
#include "tst_qtableviewext.moc"
