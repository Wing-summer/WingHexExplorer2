#include <cstdio>

#include <QApplication>
#include <QCloseEvent>
#include <QTimer>
#include <QWidget>

#include "class/application/startuptrace.h"

class Probe : public QWidget {
public:
    int closes = 0;

protected:
    void closeEvent(QCloseEvent *event) override {
        ++closes;
        QWidget::closeEvent(event);
    }
};

int main(int argc, char **argv) {
    StartupTrace::start();
    StartupTrace::PhaseTimer phases;
    QApplication application(argc, argv);
    phases.checkpoint("probe.application");
    Probe window;
    window.resize(300, 200);
    StartupTrace::watchWindow(&window);
    window.show();
    QTimer::singleShot(200, &application, [] { QCoreApplication::exit(0); });
    const auto result = application.exec();
    std::printf("closes=%d\n", window.closes);
    return result;
}
