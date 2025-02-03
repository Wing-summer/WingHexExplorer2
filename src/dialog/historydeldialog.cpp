#include "historydeldialog.h"
#include "ui_historydeldialog.h"
#include "utilities.h"

#include <QListWidgetItem>

HistoryDelDialog::HistoryDelDialog(
    const QList<RecentFileManager::RecentInfo> &files, bool fileNameOnly,
    QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryDelDialog), _infos(files),
      _fileNameOnly(fileNameOnly) {
    ui->setupUi(this);

    for (auto &info : files) {
        QMimeDatabase db;
        auto mt = db.mimeTypeForFile(info.fileName);
        auto name = RecentFileManager::getDisplayFileName(info) +
                    QStringLiteral(" (") + mt.name() + QStringLiteral(")");
        auto lw = new QListWidgetItem(name);

        if (info.isWorkSpace) {
            lw->setIcon(ICONRES(QStringLiteral("pro")));
            auto font = lw->font();
            font.setUnderline(true);
            lw->setFont(font);
        } else {
            lw->setIcon(
                Utilities::getIconFromFile(qApp->style(), info.fileName));
        }

        lw->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable |
                     Qt::ItemIsSelectable);
        lw->setCheckState(Qt::Unchecked);
        ui->lwHistory->addItem(lw);
    }

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(600, 400);
    Utilities::moveToCenter(_dialog);
}

HistoryDelDialog::~HistoryDelDialog() { delete ui; }

QVector<int> HistoryDelDialog::getResult() const {
    QVector<int> ret;
    auto total = ui->lwHistory->count();
    for (int i = 0; i < total; ++i) {
        auto cs = ui->lwHistory->item(i)->checkState();
        if (cs == Qt::Checked) {
            ret.append(i);
        }
    }
    return ret;
}

int HistoryDelDialog::exec() { return _dialog->exec(); }

void HistoryDelDialog::on_lwHistory_currentRowChanged(int currentRow) {
    auto info = _infos.at(currentRow);
    ui->tbInfo->setText(
        RecentFileManager::getDisplayTooltip(info, _fileNameOnly));
}

void HistoryDelDialog::on_buttonBox_accepted() { _dialog->accept(); }

void HistoryDelDialog::on_buttonBox_rejected() { _dialog->reject(); }
