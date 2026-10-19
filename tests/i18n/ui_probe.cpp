#include <QAbstractButton>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QStyle>
#include <QTabWidget>
#include <QTranslator>
#include <QUiLoader>
#include <cstdio>
#include <memory>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc < 3) {
        return 2;
    }
    QTranslator translator;
    if (QString::fromLocal8Bit(argv[2]) != QStringLiteral("en_US")) {
        if (!translator.load(QString::fromLocal8Bit(argv[2]))) {
            return 3;
        }
        app.installTranslator(&translator);
    }
    const QDir root(QString::fromLocal8Bit(argv[1]));
    const QStringList forms{
        QStringLiteral("src/control/gotowidget.ui"),
        QStringLiteral("src/settings/generalsettingdialog.ui"),
        QStringLiteral("src/settings/editorsettingdialog.ui"),
        QStringLiteral("src/settings/lspsettingdialog.ui"),
        QStringLiteral("src/settings/qeditconfig.ui"),
        QStringLiteral("src/settings/othersettingsdialog.ui"),
        QStringLiteral("src/settings/pluginsettingdialog.ui"),
        QStringLiteral("src/dialog/aboutsoftwaredialog.ui"),
        QStringLiteral("src/dialog/crashreport.ui")};
    int failures = 0;
    for (const auto &form : forms) {
        QFile source(root.filePath(form));
        if (!source.open(QIODevice::ReadOnly)) {
            return 4;
        }
        QUiLoader loader;
        std::unique_ptr<QWidget> widget(loader.load(&source));
        if (!widget) {
            std::fprintf(stderr, "Cannot load %s\n", qPrintable(form));
            return 5;
        }
        const auto defaultSize = widget->size();
        for (const auto &size : {defaultSize, widget->minimumSizeHint()}) {
            widget->resize(size.expandedTo(widget->minimumSizeHint()));
            widget->show();
            app.processEvents();
            const auto checkVisibleControls = [&]() {
                for (const auto button :
                     widget->findChildren<QAbstractButton *>()) {
                    if (!button->isVisible() || button->text().isEmpty()) {
                        continue;
                    }
                    if (button->width() + 2 <
                        button->minimumSizeHint().width()) {
                        std::fprintf(
                            stderr, "%s: button %s is too narrow: %d < %d\n",
                            qPrintable(form), qPrintable(button->objectName()),
                            button->width(), button->minimumSizeHint().width());
                        ++failures;
                    }
                }
                for (const auto label : widget->findChildren<QLabel *>()) {
                    if (!label->isVisible() || label->wordWrap() ||
                        label->text().isEmpty() ||
                        label->text().contains(QLatin1Char('<'))) {
                        continue;
                    }
                    const auto required =
                        label->fontMetrics().horizontalAdvance(label->text());
                    if (required > label->contentsRect().width() + 2) {
                        std::fprintf(
                            stderr, "%s: label %s is too narrow: %d < %d\n",
                            qPrintable(form), qPrintable(label->objectName()),
                            label->contentsRect().width(), required);
                        ++failures;
                    }
                }
            };
            checkVisibleControls();
            for (const auto tabs : widget->findChildren<QTabWidget *>()) {
                for (int page = 0; page < tabs->count(); ++page) {
                    tabs->setCurrentIndex(page);
                    app.processEvents();
                    checkVisibleControls();
                }
            }
        }
    }
    std::printf("UI sizing checks: %d issues across %lld forms\n", failures,
                static_cast<long long>(forms.size()));
    return failures == 0 ? 0 : 1;
}
