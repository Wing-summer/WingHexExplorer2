#include "driverselectordialog.h"
#include "src/utilities.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QScrollBar>
#include <QShortcut>

DriverSelectorDialog::DriverSelectorDialog(QWidget *parent)
    : FramelessDialog(parent) {
    auto widget = new QWidget(this);
    widget->setMinimumWidth(400);
    auto layout = new QVBoxLayout(widget);

    drivers = new QListWidget(this);
    drivers->setSortingEnabled(false);
    QStorageInfo si;
    auto ico = ICONRES("opendriver");
    auto infos = si.mountedVolumes();
    layout->addWidget(new QLabel("PleaseChooseDriver", this));
    layout->addSpacing(5);
    for (auto &item : infos) {
        if (item.isValid()
#ifdef Q_OS_LINUX
            && item.device().at(0) == '/'
#endif
        ) {
#ifdef Q_OS_WINDOWS
            drivers->addItem(new QListWidgetItem(ico, item.rootPath()));
#else
            drivers->addItem(new QListWidgetItem(ico, item.device()));
#endif
            m_infos.push_back(item);
        }
    }
    layout->addWidget(drivers);
    layout->addSpacing(5);
    infob = new QTextBrowser(this);
    layout->addWidget(infob);
    layout->addSpacing(5);
    layout->addWidget(new QLabel(tr("DriverTips"), this));
    layout->addSpacing(5);
    auto dbbox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(dbbox, &QDialogButtonBox::accepted, this,
            &DriverSelectorDialog::on_accepted);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &DriverSelectorDialog::on_rejected);
    layout->addWidget(dbbox);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &DriverSelectorDialog::on_accepted);
    connect(drivers, &QListWidget::itemSelectionChanged, this,
            &DriverSelectorDialog::on_list_selectionChanged);

    drivers->setCurrentRow(0); // 我不信你没有驱动器

    buildUpContent(widget);

    setWindowTitle(tr("OpenDriver"));
}

void DriverSelectorDialog::on_list_selectionChanged() {
    infob->clear();
#define Info(mem, info) infob->append(mem + " : " + info)
    auto item = m_infos.at(drivers->currentRow());
    Info(tr("device"), item.device());
    Info(tr("displayName"), item.displayName());
    Info(tr("fileSystemType"), item.fileSystemType());
    Info(tr("rootPath"), item.rootPath());
    Info(tr("name"), item.name());

    if (item.isReady()) {
        Info(tr("isReady"), "True");
        Info(tr("bytesAvailable"),
             Utilities::processBytesCount(item.bytesAvailable()));
        Info(tr("bytesTotal"), Utilities::processBytesCount(item.bytesTotal()));
    } else {
        Info(tr("isReady"), "False");
    }
    infob->verticalScrollBar()->setValue(0);
}

void DriverSelectorDialog::on_accepted() {
    m_si = m_infos.at(drivers->currentRow());
    done(1);
}

void DriverSelectorDialog::on_rejected() { done(0); }

QStorageInfo DriverSelectorDialog::GetResult() { return m_si; }
