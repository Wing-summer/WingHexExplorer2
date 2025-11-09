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

#include "wingfiledialog.h"
#include "../dialog/framelessdialogbase.h"
#include "settingmanager.h"

QString WingFileDialog::getExistingDirectory(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl = getExistingDirectoryUrl(
        parent, caption, QUrl::fromLocalFile(dir), options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QUrl WingFileDialog::getExistingDirectoryUrl(
    QWidget *parent, const QString &caption, const QUrl &dir,
    QFileDialog::Options options, const QStringList &supportedSchemes) {
    if (SettingManager::instance().useNativeFileDialog()) {
        return QFileDialog::getExistingDirectoryUrl(parent, caption, dir,
                                                    options, supportedSchemes);
    } else {
        auto filedialog = new QFileDialog(parent);
        filedialog->setDirectoryUrl(dir);
        filedialog->setOptions(options | QFileDialog::DontUseNativeDialog);
        filedialog->setFileMode(QFileDialog::Directory);
        filedialog->setSupportedSchemes(supportedSchemes);

        filedialog->setWindowFlag(Qt::Widget);

        FramelessDialogBase d(parent);
        d.buildUpContent(filedialog);
        d.setWindowTitle(caption);
        d.resize(filedialog->width(), filedialog->height());

        QObject::connect(filedialog, &QFileDialog::finished, &d,
                         &FramelessDialogBase::done);

        if (d.exec() == QDialog::Accepted)
            return filedialog->selectedUrls().value(0);
        return QUrl();
    }
}

QString WingFileDialog::getOpenFileName(QWidget *parent, const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl =
        getOpenFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter,
                       selectedFilter, options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QStringList WingFileDialog::getOpenFileNames(QWidget *parent,
                                             const QString &caption,
                                             const QString &dir,
                                             const QString &filter,
                                             QString *selectedFilter,
                                             QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QList<QUrl> selectedUrls =
        getOpenFileUrls(parent, caption, QUrl::fromLocalFile(dir), filter,
                        selectedFilter, options, schemes);
    QStringList fileNames;
    fileNames.reserve(selectedUrls.size());
    for (const QUrl &url : selectedUrls) {
        if (url.isLocalFile() || url.isEmpty())
            fileNames << url.toLocalFile();
        else
            fileNames << url.toString();
    }
    return fileNames;
}

QUrl WingFileDialog::getOpenFileUrl(QWidget *parent, const QString &caption,
                                    const QUrl &dir, const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options,
                                    const QStringList &supportedSchemes) {
    if (SettingManager::instance().useNativeFileDialog()) {
        return QFileDialog::getOpenFileUrl(parent, caption, dir, filter,
                                           selectedFilter, options,
                                           supportedSchemes);
    } else {
        QFileDialog *dialog = new QFileDialog(parent);
        dialog->setFileMode(QFileDialog::ExistingFile);
        dialog->selectNameFilter(filter);
        dialog->setSupportedSchemes(supportedSchemes);
        dialog->setOptions(options | QFileDialog::DontUseNativeDialog);
        dialog->setWindowFlag(Qt::Widget);

        FramelessDialogBase d(parent);
        d.buildUpContent(dialog);
        d.setWindowTitle(caption);
        d.resize(dialog->width(), dialog->height());

        QObject::connect(dialog, &QFileDialog::finished, &d,
                         &FramelessDialogBase::done);

        if (selectedFilter && !selectedFilter->isEmpty())
            dialog->selectNameFilter(*selectedFilter);
        if (d.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog->selectedNameFilter();
            return dialog->selectedUrls().value(0);
        }
        return QUrl();
    }
}

QList<QUrl> WingFileDialog::getOpenFileUrls(
    QWidget *parent, const QString &caption, const QUrl &dir,
    const QString &filter, QString *selectedFilter,
    QFileDialog::Options options, const QStringList &supportedSchemes) {
    if (SettingManager::instance().useNativeFileDialog()) {
        return QFileDialog::getOpenFileUrls(parent, caption, dir, filter,
                                            selectedFilter, options,
                                            supportedSchemes);
    } else {
        QFileDialog *dialog = new QFileDialog(parent);
        dialog->setFileMode(QFileDialog::ExistingFiles);
        dialog->selectNameFilter(filter);
        dialog->setSupportedSchemes(supportedSchemes);
        dialog->setOptions(options | QFileDialog::DontUseNativeDialog);
        dialog->setWindowFlag(Qt::Widget);

        FramelessDialogBase d(parent);
        d.buildUpContent(dialog);
        d.setWindowTitle(caption);
        d.resize(dialog->width(), dialog->height());

        QObject::connect(dialog, &QFileDialog::finished, &d,
                         &FramelessDialogBase::done);

        if (selectedFilter && !selectedFilter->isEmpty())
            dialog->selectNameFilter(*selectedFilter);
        if (d.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog->selectedNameFilter();
            return dialog->selectedUrls();
        }
        return {};
    }
}

QString WingFileDialog::getSaveFileName(QWidget *parent, const QString &caption,
                                        const QString &dir,
                                        const QString &filter,
                                        QString *selectedFilter,
                                        QFileDialog::Options options) {
    const QStringList schemes = QStringList(QStringLiteral("file"));
    const QUrl selectedUrl =
        getSaveFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter,
                       selectedFilter, options, schemes);
    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
        return selectedUrl.toLocalFile();
    else
        return selectedUrl.toString();
}

QUrl WingFileDialog::getSaveFileUrl(QWidget *parent, const QString &caption,
                                    const QUrl &dir, const QString &filter,
                                    QString *selectedFilter,
                                    QFileDialog::Options options,
                                    const QStringList &supportedSchemes) {
    if (SettingManager::instance().useNativeFileDialog()) {
        return QFileDialog::getSaveFileUrl(parent, caption, dir, filter,
                                           selectedFilter, options,
                                           supportedSchemes);
    } else {
        QFileDialog *dialog = new QFileDialog(parent);
        dialog->setFileMode(QFileDialog::AnyFile);
        dialog->selectNameFilter(filter);
        dialog->setSupportedSchemes(supportedSchemes);
        dialog->setOptions(options | QFileDialog::DontUseNativeDialog);
        dialog->setWindowFlag(Qt::Widget);
        dialog->setAcceptMode(QFileDialog::AcceptSave);

        FramelessDialogBase d(parent);
        d.buildUpContent(dialog);
        d.setWindowTitle(caption);
        d.resize(dialog->width(), dialog->height());

        QObject::connect(dialog, &QFileDialog::finished, &d,
                         &FramelessDialogBase::done);

        if (selectedFilter && !selectedFilter->isEmpty())
            dialog->selectNameFilter(*selectedFilter);
        if (d.exec() == QDialog::Accepted) {
            if (selectedFilter)
                *selectedFilter = dialog->selectedNameFilter();
            return dialog->selectedUrls().value(0);
        }
        return QUrl();
    }
}

WingFileDialog::WingFileDialog() {}
