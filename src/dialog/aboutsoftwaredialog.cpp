/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** This program is free software: you can redistribute it and/or modify it under
** the terms of the GNU Affero General Public License as published by the Free
** Software Foundation, version 3.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
** details.
**
** You should have received a copy of the GNU Affero General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
** =============================================================================
*/

#include "aboutsoftwaredialog.h"
#include "ui_aboutsoftwaredialog.h"

#include "class/languagemanager.h"
#include "utilities.h"

AboutSoftwareDialog::AboutSoftwareDialog(QWidget *parent)
    : QWidget(parent), ui(new Ui::AboutSoftwareDialog) {
    ui->setupUi(this);
    auto data = LanguageManager::instance().data();
    ui->tbAbout->setMarkdown(data.about);
    ui->tbBaseObj->setMarkdown(readContent(
        QStringLiteral(":/com.wingsummer.winghex/src/components.md")));
    ui->tbTr->setMarkdown(readContent(
        QStringLiteral(":/com.wingsummer.winghex/src/translist.md")));
    ui->tbDev->setMarkdown(data.dev);
    ui->lblVersion->setText(qApp->applicationVersion());
    ui->lblBuildDate->setText(QStringLiteral(__DATE__));

    ui->tbLicense->setMarkdown(
        readContent(QStringLiteral(":/com.wingsummer.winghex/src/LICENSE.md")));

    _dialog = new FramelessDialogBase(parent);
    _dialog->buildUpContent(this);
    _dialog->setWindowTitle(this->windowTitle());
    _dialog->setMinimumSize(800, 600);

    Utilities::moveToCenter(_dialog);
}

AboutSoftwareDialog::~AboutSoftwareDialog() {
    delete ui;
    _dialog->deleteLater();
}

int AboutSoftwareDialog::exec() { return _dialog->exec(); }

QString AboutSoftwareDialog::readContent(const QString &path) {
    QFile file(path);
    auto ret = file.open(QFile::ReadOnly);
    Q_ASSERT(ret);
    Q_UNUSED(ret);
    return QString::fromUtf8(file.readAll());
}
