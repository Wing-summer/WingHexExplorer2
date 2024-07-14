#include "openregiondialog.h"
#include "../control/toast.h"
#include "../utilities.h"
#include <QDialogButtonBox>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QShortcut>

OpenRegionDialog::OpenRegionDialog(const QString &filename, int start,
                                   int length, QWidget *parent)
    : FramelessDialog(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    layout->addWidget(new QLabel(tr("ChooseFile"), this));
    layout->addSpacing(5);

    filepath = new QPathEdit(this);
    filepath->setDialogOptions(QFileDialog::Option::ShowDirsOnly);
    filepath->setStyle(QPathEdit::Style::JoinedButton);
    filepath->setPathMode(QPathEdit::PathMode::ExistingFolder);
    filepath->setAllowEmptyPath(false);
    connect(filepath, &QPathEdit::pathChanged, filepath,
            &QPathEdit::setToolTip);
    filepath->setPath(filename);

    layout->addWidget(filepath);
    layout->addSpacing(10);

    layout->addWidget(new QLabel(tr("Start"), this));
    layout->addSpacing(5);
    sbStart = new QSpinBox(this);
    sbStart->setRange(0, INT_MAX);
    sbStart->setValue(start);
    sbStart->setPrefix("0x");
    sbStart->setDisplayIntegerBase(16);
    layout->addWidget(sbStart);
    layout->addSpacing(10);

    layout->addWidget(new QLabel(tr("Len"), this));
    layout->addSpacing(5);
    sbLength = new QSpinBox(this);
    sbLength->setRange(1, INT_MAX);
    sbLength->setValue(length);
    layout->addWidget(sbLength);

    layout->addSpacing(20);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &OpenRegionDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &OpenRegionDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &OpenRegionDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);

    this->setWindowTitle(tr("OpenRegion"));
}

RegionFileResult OpenRegionDialog::getResult() { return res; }

void OpenRegionDialog::on_accept() {
    auto file = filepath->path();
    if (file.length()) {
        if (!QFile::exists(file)) {
            Toast::toast(this, NAMEICONRES(QStringLiteral("open")),
                         tr("FileNotExist"));
        }
    } else {
        Toast::toast(this, NAMEICONRES(QStringLiteral("open")),
                     tr("NoFileSelected"));
        return;
    }
    res.filename = file;
    res.start = sbStart->value();
    res.length = sbLength->value();
    done(1);
}

void OpenRegionDialog::on_reject() { done(0); }
