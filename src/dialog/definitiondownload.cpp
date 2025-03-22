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

#include "definitiondownload.h"

#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <KSyntaxHighlighting/DefinitionDownloader>

class ResizedPlainTextEdit : public QPlainTextEdit {
public:
    ResizedPlainTextEdit(QWidget *parent) : QPlainTextEdit(parent) {}

protected:
    QSize sizeHint() const override {
        const auto &metrics = fontMetrics();
        return QSize(
            metrics.boundingRect(QString(60, QLatin1Char('x'))).width(),
            metrics.height() * 10);
    }
};

DefinitionDownloadDialog::DefinitionDownloadDialog(
    KSyntaxHighlighting::Repository *repository, QWidget *parent)
    : FramelessDialogBase(parent) {
    setWindowTitle(tr("Update Syntax Definitions"));
    setModal(true);

    auto w = new QWidget(this);
    m_status = new ResizedPlainTextEdit(w);
    m_status->setReadOnly(true);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    m_buttonBox->button(QDialogButtonBox::Close)->setEnabled(false);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto layout = new QVBoxLayout(w);
    layout->setSpacing(10);
    layout->addWidget(m_status);
    layout->addWidget(m_buttonBox);
    buildUpContent(w);

    m_downloader =
        new KSyntaxHighlighting::DefinitionDownloader(repository, this);
    QObject::connect(
        m_downloader,
        &KSyntaxHighlighting::DefinitionDownloader::informationMessage,
        m_status, &QPlainTextEdit::appendPlainText);
    QObject::connect(m_downloader,
                     &KSyntaxHighlighting::DefinitionDownloader::done, this,
                     &DefinitionDownloadDialog::downloadFinished);

    m_status->appendPlainText(
        tr("Updating syntax definitions from online repository..."));
    m_downloader->start();
    m_timer.start();
}

void DefinitionDownloadDialog::downloadFinished() {
    qint64 elapsed = m_timer.elapsed();
    QString timeStr;
    if (elapsed >= 60 * 60 * 1000) {
        qint64 hours = elapsed / (60 * 60 * 1000);
        elapsed %= (60 * 60 * 1000);
        qint64 minutes = elapsed / (60 * 1000);
        timeStr =
            tr("%1:%2 hours").arg(hours).arg(minutes, 2, 10, QLatin1Char('0'));
    } else if (elapsed >= 60 * 1000) {
        qint64 minutes = elapsed / (60 * 1000);
        elapsed %= (60 * 1000);
        qint64 seconds = elapsed / 1000;
        timeStr = tr("%1:%2 minutes")
                      .arg(minutes)
                      .arg(seconds, 2, 10, QLatin1Char('0'));
    } else if (elapsed >= 1000) {
        qint64 seconds = elapsed / 1000;
        elapsed %= 1000;
        timeStr =
            tr("%1.%2 seconds").arg(seconds).arg((int)qRound(elapsed / 100.0));
    } else {
        timeStr = tr("%1 ms").arg(elapsed);
    }
    m_status->appendPlainText(
        tr("Update operation completed (%1)").arg(timeStr));
    m_buttonBox->button(QDialogButtonBox::Close)->setEnabled(true);
    m_downloader->deleteLater();
}

void DefinitionDownloadDialog::closeEvent(QCloseEvent *e) {
    if (!m_buttonBox->button(QDialogButtonBox::Close)->isEnabled())
        e->ignore();
}
