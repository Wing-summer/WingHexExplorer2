#include "checksumdialog.h"
#include "src/utilities.h"

#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QShortcut>

CheckSumDialog::CheckSumDialog(QWidget *parent) : FramelessDialog(parent) {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);

    this->setFixedSize(500, 600);
    auto l = new QLabel(tr("ChooseEncoding"), this);
    layout->addWidget(l);
    layout->addSpacing(5);
    hashlist = new QListWidget(this);
    for (auto &item : Utilities::supportedHashAlgorithmStringList()) {
        auto w = new QListWidgetItem(item, hashlist);
        w->setCheckState(Qt::Checked);
    }
    layout->addWidget(hashlist);
    layout->addSpacing(10);
    auto dbbox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(dbbox, &QDialogButtonBox::accepted, this,
            &CheckSumDialog::on_accept);
    connect(dbbox, &QDialogButtonBox::rejected, this,
            &CheckSumDialog::on_reject);
    auto key = QKeySequence(Qt::Key_Return);
    auto s = new QShortcut(key, this);
    connect(s, &QShortcut::activated, this, &CheckSumDialog::on_accept);
    layout->addWidget(dbbox);

    buildUpContent(widget);
    this->setWindowTitle(tr("CheckSum"));
}

const QVector<int> &CheckSumDialog::getResults() { return _result; }

void CheckSumDialog::on_accept() {
    _result.clear();
    for (int i = 0; i < hashlist->count(); ++i) {
        auto item = hashlist->item(i);
        if (item->checkState() == Qt::Checked) {
            _result.append(i);
        }
    }
    done(1);
}

void CheckSumDialog::on_reject() { done(0); }
