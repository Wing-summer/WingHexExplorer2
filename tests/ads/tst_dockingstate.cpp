#include "AutoHideDockContainer.h"
#include "AutoHideSideBar.h"
#include "AutoHideTab.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockSplitter.h"
#include "DockWidget.h"
#include "DockingState.h"
#include "FloatingDockContainer.h"
#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPainter>
#include <QPointer>
#include <QProcess>
#include <QSettings>
#include <QSignalSpy>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QtTest>

using namespace ads;

namespace {
void drain() {
    QApplication::processEvents();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QApplication::processEvents();
}
CDockWidget *dock(CDockManager *manager, const QString &name) {
    auto widget = new CDockWidget(manager, name);
    widget->setWidget(new QLabel(name));
    return widget;
}
class PaintProbe : public QWidget {
public:
    int paints = 0;
    PaintProbe() { setMinimumSize(100, 80); }

protected:
    void paintEvent(QPaintEvent *) override {
        ++paints;
        QPainter painter(this);
        painter.fillRect(rect(), Qt::red);
    }
};
class ExcludedDock : public CDockWidget {
public:
    explicit ExcludedDock(CDockManager *manager)
        : CDockWidget(manager, QStringLiteral("excluded")) {
        setWidget(new QLabel(QStringLiteral("Excluded")));
    }
    bool isStateSaveEnabled() const override { return false; }
};
state::Layout minimal() {
    state::Layout layout;
    state::Container container;
    state::Node area;
    area.splitter = false;
    area.widgets.append({QStringLiteral("first"), false});
    area.current = QStringLiteral("first");
    container.root.children.append(area);
    container.root.sizes.append(300);
    layout.containers.append(container);
    return layout;
}
} // namespace

class DockingStateTests : public QObject {
    Q_OBJECT
private slots:
    void init() {
        CDockManager::setConfigFlags(CDockManager::DefaultNonOpaqueConfig |
                                     CDockManager::FocusHighlighting);
        CDockManager::setAutoHideConfigFlags(
            CDockManager::DefaultAutoHideConfig);
    }
    void restoreWhileAncestorUpdatesDisabled_data() {
        QTest::addColumn<bool>("shown");
        QTest::newRow("startup-before-show") << false;
        QTest::newRow("shown-window") << true;
    }
    void restoreWhileAncestorUpdatesDisabled() {
        QFETCH(bool, shown);
        QWidget window;
        window.resize(1000, 700);
        if (shown) {
            window.show();
            drain();
        }
        window.setUpdatesEnabled(false);
        auto layout = new QVBoxLayout(&window);
        auto manager = new CDockManager;
        layout->addWidget(manager);
        auto center = new CDockWidget(manager, QStringLiteral("central"));
        auto probe = new PaintProbe;
        center->setWidget(probe);
        manager->setCentralWidget(center);
        auto side = dock(manager, QStringLiteral("side"));
        manager->addDockWidget(LeftDockWidgetArea, side);
        QVERIFY(!manager->updatesEnabled());
        QVERIFY(!manager->testAttribute(Qt::WA_ForceUpdatesDisabled));
        const auto bytes = manager->saveState();
        QVERIFY(manager->restoreState(bytes));
        QVERIFY(!manager->testAttribute(Qt::WA_ForceUpdatesDisabled));
        window.setUpdatesEnabled(true);
        window.show();
        drain();
        QVERIFY(manager->updatesEnabled());
        QVERIFY(center->updatesEnabled());
        QVERIFY(center->isVisible());
        QVERIFY(side->isVisible());
        QVERIFY(probe->paints > 0);
    }
    void secondProcessStartupPaintsPersistedLayout() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto settingsPath =
            directory.filePath(QStringLiteral("layout.ini"));
        QString executable = QCoreApplication::applicationDirPath() +
                             QStringLiteral("/ads_startup_probe");
#ifdef Q_OS_WIN
        executable += QStringLiteral(".exe");
#endif
        for (int launch = 0; launch < 2; ++launch) {
            QProcess process;
            process.start(executable, {settingsPath});
            QVERIFY(process.waitForStarted());
            QVERIFY(process.waitForFinished(30000));
            const auto diagnostics = process.readAllStandardError();
            QVERIFY2(process.exitStatus() == QProcess::NormalExit,
                     diagnostics.constData());
            QVERIFY2(process.exitCode() == 0, diagnostics.constData());
            QSettings settings(settingsPath, QSettings::IniFormat);
            QVERIFY(settings.value(QStringLiteral("layout"))
                        .toByteArray()
                        .startsWith("ADSB"));
        }
    }
    void restoreKeepsInactiveContentsHidden_data() {
        QTest::addColumn<bool>("shown");
        QTest::newRow("during-startup") << false;
        QTest::newRow("visible-window") << true;
    }
    void restoreKeepsInactiveContentsHidden() {
        QFETCH(bool, shown);
        QMainWindow window;
        window.resize(900, 700);
        auto *manager = new CDockManager(&window);
        auto *symbols = dock(manager, QStringLiteral("symbols"));
        auto *area = manager->addDockWidget(RightDockWidgetArea, symbols);
        auto *variables = new CDockWidget(manager, QStringLiteral("variables"));
        auto *tabs = new QTabWidget;
        auto *tree = new QTreeWidget;
        tree->setHeaderLabels(
            {QStringLiteral("Name"), QStringLiteral("Value")});
        tabs->addTab(tree, QStringLiteral("Global"));
        variables->setWidget(tabs);
        manager->addDockWidget(CenterDockWidgetArea, variables, area);
        area->setCurrentDockWidget(symbols);
        const auto bytes = manager->saveState();
        if (shown) {
            window.show();
            drain();
        }
        for (int iteration = 0; iteration < 3; ++iteration) {
            variables->dockAreaWidget()->setCurrentDockWidget(variables);
            QVERIFY(manager->restoreState(bytes));
            window.show();
            drain();
            QCOMPARE(symbols->dockAreaWidget()->currentDockWidget(), symbols);
            QVERIFY(symbols->isVisible());
            QVERIFY(variables->isHidden());
            QVERIFY(!tree->isVisible());
            variables->raise();
            drain();
            QVERIFY(tree->isVisible());
            symbols->raise();
            drain();
            QVERIFY(!tree->isVisible());
        }
    }
    void roundTripAndLifetime() {
        QMainWindow window;
        window.resize(1100, 800);
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto b = dock(manager, QStringLiteral("b"));
        auto c = dock(manager, QStringLiteral("c"));
        auto floating = dock(manager, QStringLiteral("floating"));
        auto pinned = dock(manager, QStringLiteral("pinned"));
        auto central = dock(manager, QStringLiteral("central"));
        manager->setCentralWidget(central);
        auto area = manager->addDockWidget(LeftDockWidgetArea, a);
        manager->addDockWidget(CenterDockWidgetArea, b, area);
        manager->addDockWidget(CenterDockWidgetArea, c, area);
        area->setAllowedAreas(LeftDockWidgetArea | RightDockWidgetArea);
        area->setDockAreaFlags(CDockAreaWidget::HideSingleWidgetTitleBar);
        area->setCurrentDockWidget(b);
        c->toggleView(false);
        auto fw = manager->addDockWidgetFloating(floating);
        fw->resize(460, 320);
        manager->addAutoHideDockWidget(SideBarRight, pinned)->setSize(230);
        window.show();
        drain();
        manager->setDockWidgetFocused(b);
        drain();
        const auto bytes = manager->saveState(7);
        state::Layout decoded;
        QVERIFY(state::decode(bytes, 7, decoded));
        QCOMPARE(decoded.centralWidget, QStringLiteral("central"));
        QSignalSpy restored(manager, &CDockManager::stateRestored);
        QSignalSpy view(b, &CDockWidget::viewToggled);
        QList<QPointer<CDockWidget>> alive{a, b, c, floating, pinned, central};
        for (int iteration = 0; iteration < 5; ++iteration) {
            QVERIFY(manager->restoreState(bytes, 7));
            drain();
            for (const auto &widget : alive) {
                QVERIFY(widget);
            }
            QVERIFY(manager->isVisible());
            QVERIFY(central->isVisible());
            QVERIFY(b->isVisible());
            QVERIFY(b->width() > 0);
            QVERIFY(b->height() > 0);
            QVERIFY(a->dockAreaWidget()->isVisible());
            QCOMPARE(manager->dockContainers().size(), 2);
            QCOMPARE(a->dockAreaWidget(), b->dockAreaWidget());
            QCOMPARE(b->dockAreaWidget()->currentDockWidget(), b);
            QCOMPARE(b->dockAreaWidget()->dockWidget(0), a);
            QCOMPARE(b->dockAreaWidget()->dockWidget(1), b);
            QCOMPARE(b->dockAreaWidget()->dockWidget(2), c);
            QVERIFY(c->isClosed());
            QVERIFY(!c->toggleViewAction()->isChecked());
            QCOMPARE(int(a->dockAreaWidget()->allowedAreas()),
                     int(LeftDockWidgetArea | RightDockWidgetArea));
            QCOMPARE(a->dockAreaWidget()->dockAreaFlags(),
                     CDockAreaWidget::DockAreaFlags(
                         CDockAreaWidget::HideSingleWidgetTitleBar));
            QVERIFY(pinned->isAutoHide());
            QCOMPARE(pinned->autoHideDockContainer()->sideBarLocation(),
                     SideBarRight);
            QVERIFY(floating->dockContainer()->isFloating());
            QCOMPARE(floating->dockContainer()->floatingWidget()->size(),
                     QSize(460, 320));
        }
        QCOMPARE(restored.count(), 5);
        QCOMPARE(view.count(), 5);
        if (!decoded.focusedWidget.isEmpty()) {
            QCOMPARE(manager->focusedDockWidget(), b);
        }
        // Extra floating containers are detached once and destroyed later.
        auto extra = dock(manager, QStringLiteral("extra"));
        QPointer<CFloatingDockContainer> extraWindow =
            manager->addDockWidgetFloating(extra);
        QVERIFY(manager->restoreState(bytes, 7));
        drain();
        QVERIFY(extraWindow.isNull());
        QVERIFY(extra->isClosed());
        QCOMPARE(extra->parentWidget(), manager);
    }
    void exclusionsAndMissingWidgets() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto b = dock(manager, QStringLiteral("b"));
        auto c = dock(manager, QStringLiteral("c"));
        auto excluded = new ExcludedDock(manager);
        auto left = manager->addDockWidget(LeftDockWidgetArea, a);
        manager->addDockWidget(CenterDockWidgetArea, excluded, left);
        auto right = manager->addDockWidget(RightDockWidgetArea, b);
        manager->addDockWidget(CenterDockWidgetArea, c, right);
        right->setCurrentDockWidget(b);
        auto bytes = manager->saveState();
        QVERIFY(!bytes.contains("excluded"));
        manager->removeDockWidget(b);
        delete b;
        QVERIFY(manager->restoreState(bytes));
        drain();
        QVERIFY(excluded->isClosed());
        QCOMPARE(c->dockAreaWidget()->currentDockWidget(), c);
        QCOMPARE(c->dockAreaWidget()->dockWidgetsCount(), 1);
        manager->removeDockWidget(a);
        delete a;
        QVERIFY(manager->restoreState(bytes));
        drain();
        QCOMPARE(manager->dockAreaCount(), 1);
        QCOMPARE(c->dockAreaWidget()->parentSplitter()->count(), 1);
        QCOMPARE(c->dockAreaWidget()->currentDockWidget(), c);
    }
    void invalidInputDoesNotMutate() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("first"));
        auto area = manager->addDockWidget(LeftDockWidgetArea, a);
        auto valid = manager->saveState(19);
        QSignalSpy start(manager, &CDockManager::restoringState);
        const auto reject = [&](const QByteArray &bytes, int version = 19) {
            QVERIFY(!manager->restoreState(bytes, version));
            QCOMPARE(a->dockAreaWidget(), area);
            QCOMPARE(manager->saveState(19), valid);
            QVERIFY(!manager->isRestoringState());
        };
        reject(valid, 20);
        reject(QByteArray("<?xml version=\"1.0\"?><QtAdvancedDockingSystem/>"));
        reject(qCompress(QByteArray("<?xml version=\"1.0\"?>")));
        for (int size = 0; size < valid.size(); ++size) {
            reject(valid.left(size));
        }
        reject(valid + 'x');
        for (int offset : {0, 4, 6, 10, 14, 18}) {
            auto broken = valid;
            broken[offset] = char(0xff);
            reject(broken);
        }
        auto layout = minimal();
        layout.containers[0].root.children[0].widgets.append(
            {QStringLiteral("first"), false});
        reject(state::encode(layout, 19));
        layout = minimal();
        layout.containers[0].root.orientation = Qt::Orientation(9);
        reject(state::encode(layout, 19));
        layout = minimal();
        layout.containers[0].root.sizes[0] = -1;
        reject(state::encode(layout, 19));
        layout = minimal();
        layout.containers[0].root.children[0].flags = 2;
        reject(state::encode(layout, 19));
        layout = minimal();
        layout.containers[0].root.children[0].allowedAreas = 0x200;
        reject(state::encode(layout, 19));
        layout = minimal();
        layout.containers[0].root.children[0].current =
            QStringLiteral("absent");
        reject(state::encode(layout, 19));
        layout = minimal();
        state::Node node = layout.containers[0].root;
        for (int i = 0; i < 258; ++i) {
            state::Node parent;
            parent.children.append(std::move(node));
            parent.sizes.append(1);
            node = std::move(parent);
        }
        layout.containers[0].root = std::move(node);
        reject(state::encode(layout, 19));
        QCOMPARE(start.count(), 0);
    }
    void visibilityReentryAndCentralMismatch() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto central = dock(manager, QStringLiteral("central"));
        manager->setCentralWidget(central);
        const auto bytes = manager->saveState();
        bool reentryRejected = false;
        connect(manager, &CDockManager::restoringState, manager,
                [&] { reentryRejected = !manager->restoreState(bytes); });
        manager->hide();
        QVERIFY(manager->isHidden());
        manager->setUpdatesEnabled(false);
        QVERIFY(manager->restoreState(bytes));
        QVERIFY(reentryRejected);
        QVERIFY(manager->isHidden());
        QVERIFY(!manager->updatesEnabled());
        auto invalid = minimal();
        QVERIFY(!manager->restoreState(state::encode(invalid, 0)));
        QCOMPARE(manager->centralWidget(), central);
        QCOMPARE(central->dockAreaWidget()->currentDockWidget(), central);
    }
    void autoHideDisabledAndPerspectives() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto pinned = dock(manager, QStringLiteral("pinned"));
        manager->addDockWidget(LeftDockWidgetArea, a);
        manager->addAutoHideDockWidget(SideBarLeft, pinned);
        const auto bytes = manager->saveState();
        CDockManager::setAutoHideConfigFlags(CDockManager::AutoHideFlags());
        QVERIFY(manager->restoreState(bytes));
        drain();
        QVERIFY(pinned->isClosed());
        QVERIFY(!pinned->isAutoHide());
        manager->addPerspective(QStringLiteral("saved"));
        a->toggleView(false);
        manager->openPerspective(QStringLiteral("saved"));
        drain();
        QVERIFY(!a->isClosed());
    }
    void transitionsAndClosedCurrent() {
        QMainWindow window;
        window.resize(1000, 700);
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto b = dock(manager, QStringLiteral("b"));
        auto area = manager->addDockWidget(LeftDockWidgetArea, a);
        manager->addDockWidget(CenterDockWidgetArea, b, area);
        area->setCurrentDockWidget(b);
        window.show();
        drain();
        const auto normal = manager->saveState();
        manager->addAutoHideDockWidget(SideBarBottom, b)->setSize(180);
        const auto autoHidden = manager->saveState();
        for (int i = 0; i < 3; ++i) {
            QVERIFY(manager->restoreState(normal));
            drain();
            QVERIFY(!b->isAutoHide());
            QCOMPARE(a->dockAreaWidget(), b->dockAreaWidget());
            QCOMPARE(b->dockAreaWidget()->currentDockWidget(), b);
            QVERIFY(manager->restoreState(autoHidden));
            drain();
            QVERIFY(b->isAutoHide());
            QCOMPARE(b->autoHideDockContainer()->sideBarLocation(),
                     SideBarBottom);
        }
        state::Layout layout;
        QVERIFY(state::decode(normal, 0, layout));
        auto &savedArea = layout.containers[0].root.children[0];
        // Select a closed saved tab: the first surviving open tab wins.
        savedArea.widgets[1].closed = true;
        savedArea.current = QStringLiteral("b");
        QVERIFY(manager->restoreState(state::encode(layout, 0)));
        drain();
        QVERIFY(b->isClosed());
        QCOMPARE(a->dockAreaWidget()->currentDockWidget(), a);
        savedArea.widgets[0].closed = true;
        QVERIFY(manager->restoreState(state::encode(layout, 0)));
        drain();
        QVERIFY(a->isClosed());
        QVERIFY(a->dockAreaWidget()->isHidden());
    }
    void consecutiveRestoresBeforeDeferredDeletion() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto b = dock(manager, QStringLiteral("b"));
        manager->addDockWidget(LeftDockWidgetArea, a);
        const auto single = manager->saveState();
        manager->addDockWidgetFloating(b);
        const auto floating = manager->saveState();
        for (int i = 0; i < 5; ++i) {
            QVERIFY(manager->restoreState(single));
            QVERIFY(manager->restoreState(floating));
            QCOMPARE(manager->dockContainers().size(), 2);
            QCOMPARE(manager->floatingWidgets().size(), 1);
        }
        drain();
        QCOMPARE(manager->dockContainers().size(), 2);
        QVERIFY(b->dockContainer()->isFloating());
    }
    void sideBarOrderAndSize() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        auto a = dock(manager, QStringLiteral("a"));
        auto b = dock(manager, QStringLiteral("b"));
        auto ca = manager->addAutoHideDockWidget(SideBarLeft, a);
        auto cb = manager->addAutoHideDockWidget(SideBarLeft, b);
        ca->setSize(210);
        cb->setSize(320);
        const auto bytes = manager->saveState();
        auto bar = manager->autoHideSideBar(SideBarLeft);
        bar->addAutoHideWidget(cb, 0);
        QCOMPARE(bar->tab(0)->dockWidget(), b);
        ca->setSize(500);
        QVERIFY(manager->restoreState(bytes));
        drain();
        QCOMPARE(bar->tab(0)->dockWidget(), a);
        QCOMPARE(bar->tab(1)->dockWidget(), b);
        state::Layout decoded;
        QVERIFY(state::decode(manager->saveState(), 0, decoded));
        QCOMPARE(decoded.containers[0].sideBars[0].size, 210);
        QCOMPARE(decoded.containers[0].sideBars[1].size, 320);
    }
    void codecValidation() {
        auto layout = minimal();
        auto bytes = state::encode(layout, 31);
        QVERIFY(bytes.startsWith("ADSB"));
        state::Layout decoded;
        QVERIFY(state::decode(bytes, 31, decoded));
        QCOMPARE(state::encode(decoded, 31), bytes);
        decoded.centralWidget = QStringLiteral("sentinel");
        auto badLength = bytes;
        for (int i = 10; i < 14; ++i) {
            badLength[i] = char(0xff);
        }
        QVERIFY(!state::decode(badLength, 31, decoded));
        QCOMPARE(decoded.centralWidget, QStringLiteral("sentinel"));
        layout.containers[0].sideBars.append(
            {{QStringLiteral("side"), false}, 4, 120});
        QVERIFY(!state::decode(state::encode(layout, 31), 31, decoded));
        layout.containers[0].sideBars[0].location = 0;
        layout.containers[0].sideBars[0].size = -1;
        QVERIFY(!state::decode(state::encode(layout, 31), 31, decoded));
        layout = minimal();
        state::Container floating;
        floating.floating = true;
        floating.geometry = QByteArray::fromHex("01d9d0cb00030000");
        layout.containers.append(floating);
        QVERIFY(!state::decode(state::encode(layout, 31), 31, decoded));
        // Mutate all bytes through the decoder, including length and tag
        // fields. Successful mutations must still produce a canonically
        // decodable state.
        for (int i = 0; i < bytes.size(); ++i) {
            auto mutated = bytes;
            mutated[i] = char(uchar(mutated[i]) ^ 0xff);
            if (state::decode(mutated, 31, decoded)) {
                state::Layout repeated;
                QVERIFY(
                    state::decode(state::encode(decoded, 31), 31, repeated));
            }
        }
    }
    void emptyLayout() {
        QMainWindow window;
        auto manager = new CDockManager(&window);
        QVERIFY(manager->restoreState(manager->saveState()));
        drain();
        QCOMPARE(manager->dockAreaCount(), 0);
    }
};
QTEST_MAIN(DockingStateTests)
#include "tst_dockingstate.moc"
