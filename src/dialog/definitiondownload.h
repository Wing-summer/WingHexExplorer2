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

#ifndef QTEXTPAD_DEFINITIONDOWNLOAD_H
#define QTEXTPAD_DEFINITIONDOWNLOAD_H

#include "dialog/framelessdialogbase.h"

#include <QElapsedTimer>

class QPlainTextEdit;
class QDialogButtonBox;

namespace KSyntaxHighlighting {
class Repository;
class DefinitionDownloader;
} // namespace KSyntaxHighlighting

class DefinitionDownloadDialog : public FramelessDialogBase {
    Q_OBJECT

public:
    explicit DefinitionDownloadDialog(
        KSyntaxHighlighting::Repository *repository, QWidget *parent);

public Q_SLOTS:
    void downloadFinished();

protected:
    void closeEvent(QCloseEvent *) override;

private:
    KSyntaxHighlighting::DefinitionDownloader *m_downloader;
    QPlainTextEdit *m_status;
    QDialogButtonBox *m_buttonBox;
    QElapsedTimer m_timer;
};

#endif // QTEXTPAD_DEFINITIONDOWNLOAD_H
