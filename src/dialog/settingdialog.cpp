#include "settingdialog.h"
#include "ui_settingdialog.h"

#include "../dbghelper.h"
#include "../utilities.h"

#include <QApplication>
#include <QPushButton>
#include <QShowEvent>

SettingDialog::SettingDialog(QWidget *parent)
    : FramelessDialog(parent), ui(new Ui::SettingDialog) {
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Reset)
        ->setToolTip(tr("Reset All settings"));
    ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)
        ->setToolTip(tr("Reset settings of the current page"));

    ui->splitter->setStretchFactor(1, 1);
    connect(ui->listWidget, &QListWidget::currentRowChanged, ui->stackedWidget,
            [=](int index) {
                ui->stackedWidget->setCurrentIndex(index);
                auto page = m_pages[index];
                // if (page->blocked()) {
                //     pmb::PopMsgBox::instance().enqueueMessage(
                //         qAppName(), tr("Please stop scanning task to edit"),
                //         Qt::black);
                // }
            });
}

SettingDialog::~SettingDialog() { delete ui; }

void SettingDialog::addPage(QWidget *page) {
    if (page == nullptr) {
        qWarning("SettingDialog::addPage: Invaild Page");
        return;
    }
    m_pages << page;
}

void SettingDialog::build() {
    ASSERT_SINGLETON;
    // for (auto &page : m_pages) {
    //     auto name = page->displayName();
    //     auto icon = page->categoryIcon();
    //     auto item = new QListWidgetItem(icon, name);
    //     item->setSizeHint(QSize(0, 40));
    //     ui->listWidget->addItem(item);
    //     ui->stackedWidget->addWidget(page);
    // }
    // if (m_pages.size()) {
    //     ui->listWidget->setCurrentRow(0);
    // }
}

void SettingDialog::showConfig(int index) {
    if (index >= 0 && index < m_pages.size()) {
        ui->listWidget->setCurrentRow(index);
    }
    Utilities::moveToCenter(this);
    exec();
}

void SettingDialog::reload() {
    for (auto &page : m_pages) {
        // page->reload();
    }
}

void SettingDialog::setBlocked(bool blocked) {
    for (auto &page : m_pages) {
        // if (page->canBlocked()) {
        //     page->setBlocked(blocked);
        // }
    }
}

void SettingDialog::on_buttonBox_clicked(QAbstractButton *button) {
    auto btnbox = ui->buttonBox;
    auto btn = qobject_cast<QPushButton *>(button);
    if (btn == btnbox->button(QDialogButtonBox::Ok)) {
        //   HANDLE_CONFIG;
        //   for (auto &page : m_pages) {
        //     page->apply(CONFIG);
        //   }
        //   done(1);
        // } else if (btn == btnbox->button(QDialogButtonBox::Apply)) {
        //   HANDLE_CONFIG;
        //   for (auto &page : m_pages) {
        //     page->apply(CONFIG);
        //   }
        // } else if (btn == btnbox->button(QDialogButtonBox::RestoreDefaults))
        // {
        //   auto index = ui->listWidget->currentRow();
        //   HANDLE_CONFIG;
        //   m_pages[index]->reset(CONFIG);
        // } else if (btn == btnbox->button(QDialogButtonBox::Reset)) {
        //   auto res = QMessageBox::warning(
        //       this, qAppName(),
        //       tr("This will reset all settings. Are you sure to continue?"),
        //       QMessageBox::Yes | QMessageBox::No);
        //   if (res == QMessageBox::No)
        //     return;

        //   HANDLE_CONFIG;
        //   for (auto &page : m_pages) {
        //     page->reset(CONFIG);
        //   }
        // } else if (btn == btnbox->button(QDialogButtonBox::Cancel)) {
        //   for (auto &page : m_pages) {
        //     page->cancel();
        //   }
        done(0);
    }
}

void SettingDialog::showEvent(QShowEvent *event) {
    FramelessDialog::showEvent(event);
    // if (m_pages[ui->listWidget->currentRow()]->blocked()) {
    //   pmb::PopMsgBox::instance().enqueueMessage(
    //       qAppName(), tr("Please stop scanning task to edit"), Qt::black);
    // }
}
