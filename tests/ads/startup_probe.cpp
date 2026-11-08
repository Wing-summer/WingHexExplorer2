#include <QApplication>
#include <QPainter>
#include <QSettings>
#include <QVBoxLayout>

#include "DockManager.h"
#include "DockWidget.h"

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

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc != 2) {
        return 1;
    }
    QSettings settings(QString::fromLocal8Bit(argv[1]), QSettings::IniFormat);
    QWidget window;
    window.resize(1000, 700);
    // Match MainWindow/ScriptingDialog: disable the host during construction,
    // then restore persisted state before enabling updates and showing it.
    window.setUpdatesEnabled(false);
    auto layout = new QVBoxLayout(&window);
    auto manager = new ads::CDockManager;
    layout->addWidget(manager);
    auto central = new ads::CDockWidget(manager, QStringLiteral("central"));
    auto centerPaint = new PaintProbe;
    central->setWidget(centerPaint);
    manager->setCentralWidget(central);
    auto side = new ads::CDockWidget(manager, QStringLiteral("side"));
    auto sidePaint = new PaintProbe;
    side->setWidget(sidePaint);
    manager->addDockWidget(ads::LeftDockWidgetArea, side);
    const auto saved = settings.value(QStringLiteral("layout")).toByteArray();
    if (!saved.isEmpty() && !manager->restoreState(saved)) {
        return 2;
    }
    window.setUpdatesEnabled(true);
    window.show();
    QApplication::processEvents();
    QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QApplication::processEvents();
    if (!manager->updatesEnabled() || !central->isVisible() ||
        !side->isVisible() || !centerPaint->paints || !sidePaint->paints) {
        return 3;
    }
    settings.setValue(QStringLiteral("layout"), manager->saveState());
    settings.sync();
    return settings.status() == QSettings::NoError ? 0 : 4;
}
