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

#ifndef WINGFILEDIALOG_H
#define WINGFILEDIALOG_H

#include <QFileDialog>

/**
 * @brief A hacker class for QFileDialog
 */
class WingFileDialog {
public:
    static QString getExistingDirectory(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(),
        QFileDialog::Options options = QFileDialog::ShowDirsOnly);

    static QUrl getExistingDirectoryUrl(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QUrl &dir = QUrl(),
        QFileDialog::Options options = QFileDialog::ShowDirsOnly,
        const QStringList &supportedSchemes = QStringList());

    static void getOpenFileContent(
        const QString &nameFilter,
        const std::function<void(const QString &, const QByteArray &)>
            &fileOpenCompleted,
        const QString &caption = QString(), QWidget *parent = nullptr);

    static QString getOpenFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    static QStringList getOpenFileNames(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    static QUrl
    getOpenFileUrl(QWidget *parent = nullptr,
                   const QString &caption = QString(), const QUrl &dir = QUrl(),
                   const QString &filter = QString(),
                   QString *selectedFilter = nullptr,
                   QFileDialog::Options options = QFileDialog::Options(),
                   const QStringList &supportedSchemes = QStringList());

    static QList<QUrl>
    getOpenFileUrls(QWidget *parent = nullptr,
                    const QString &caption = QString(),
                    const QUrl &dir = QUrl(), const QString &filter = QString(),
                    QString *selectedFilter = nullptr,
                    QFileDialog::Options options = QFileDialog::Options(),
                    const QStringList &supportedSchemes = QStringList());

    static QString getSaveFileName(
        QWidget *parent = nullptr, const QString &caption = QString(),
        const QString &dir = QString(), const QString &filter = QString(),
        QString *selectedFilter = nullptr,
        QFileDialog::Options options = QFileDialog::Options());

    static QUrl
    getSaveFileUrl(QWidget *parent = nullptr,
                   const QString &caption = QString(), const QUrl &dir = QUrl(),
                   const QString &filter = QString(),
                   QString *selectedFilter = nullptr,
                   QFileDialog::Options options = QFileDialog::Options(),
                   const QStringList &supportedSchemes = QStringList());

    static void saveFileContent(const QByteArray &fileContent,
                                const QString &fileNameHint,
                                const QString &caption = QString(),
                                QWidget *parent = nullptr);

private:
    WingFileDialog();

    Q_DISABLE_COPY_MOVE(WingFileDialog)
};

#endif // WINGFILEDIALOG_H
