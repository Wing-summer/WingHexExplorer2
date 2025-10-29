#include "historydeldialog.h"
#include "ui_historydeldialog.h"

#include "class/pluginsystem.h"
#include "utilities.h"

#include <QListWidgetItem>

HistoryDelDialog::HistoryDelDialog(
    const QList<RecentFileManager::RecentInfo> &files, bool isScriptFile,
    QWidget *parent)
    : QWidget(parent), ui(new Ui::HistoryDelDialog), _infos(files),
      _isScriptFile(isScriptFile) {
    ui->setupUi(this);

    for (auto &info : files) {
        const auto &url = info.url;
        auto lw = new QListWidgetItem;

        if (url.isLocalFile()) {
            QMimeDatabase db;
            auto fileName = info.url.toLocalFile();
            auto mt = db.mimeTypeForFile(fileName);
            auto title =
                RecentFileManager::getDisplayFileName(info, _isScriptFile);
            lw->setText(title);
            if (_isScriptFile) {
                lw->setIcon(
                    Utilities::getIconFromFile(qApp->style(), fileName));
            } else {
                lw->setText(title + QStringLiteral(" (") + mt.name() +
                            QStringLiteral(")"));
                if (info.isWorkSpace) {
                    lw->setIcon(ICONRES(QStringLiteral("pro")));
                    auto font = lw->font();
                    font.setUnderline(true);
                    lw->setFont(font);
                } else {
                    lw->setIcon(
                        Utilities::getIconFromFile(qApp->style(), fileName));
                }
            }
        } else {
            // plugin extension
            auto &plgsys = PluginSystem::instance();
            auto plgID = url.authority();
            auto devs = plgsys.devices();
            lw->setText(RecentFileManager::getDisplayFileName(info, false));
            auto r = std::find_if(
                devs.begin(), devs.end(), [plgID](IWingDevice *dev) {
                    return plgID.compare(PluginSystem::getPUID(dev),
                                         Qt::CaseInsensitive) == 0;
                });
            if (r != devs.end()) {
                lw->setIcon((*r)->supportedFileIcon());
            } else {
                lw->setIcon(ICONRES(QStringLiteral("devext")));
            }
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
        RecentFileManager::getDisplayTooltip(info, _isScriptFile));
}

void HistoryDelDialog::on_buttonBox_accepted() { _dialog->accept(); }

void HistoryDelDialog::on_buttonBox_rejected() { _dialog->reject(); }
