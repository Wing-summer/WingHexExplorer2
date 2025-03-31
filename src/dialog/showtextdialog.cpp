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

#include "showtextdialog.h"

#include "class/qkeysequences.h"
#include "class/wingmessagebox.h"
#include "control/codeedit.h"
#include "control/popupactionwidget.h"
#include "control/settingspopup.h"
#include "control/toast.h"
#include "dialog/definitiondownload.h"
#include "dialog/encodingdialog.h"

#include <QScrollBar>
#include <QThread>
#include <QVBoxLayout>

#include <KSyntaxHighlighting/Repository>

constexpr auto EMPTY_FUNC = [] {};

class SyntaxPopupAction : public PopupActionWidget {
public:
    explicit SyntaxPopupAction(ShowTextDialog *parent)
        : PopupActionWidget(parent) {}

protected:
    QWidget *createWidget(QWidget *menuParent) override {
        auto popup = new SyntaxPopup(menuParent);
        connect(popup, &SyntaxPopup::syntaxSelected, this,
                [this](const KSyntaxHighlighting::Definition &syntax) {
                    auto window = qobject_cast<ShowTextDialog *>(parent());
                    window->setSyntax(syntax);

                    // Don't close the popup right after clicking, so the user
                    // can briefly see the visual feedback for the item they
                    // selected.
                    closePopUpWidget();
                });
        return popup;
    }
};

ShowTextDialog::ShowTextDialog(QWidget *parent) : FramelessMainWindow(parent) {
    this->setUpdatesEnabled(false);

    // build up UI
    buildUpRibbonBar();

    auto cw = new QWidget(this);
    cw->setSizePolicy(
        QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto layout = new QVBoxLayout(cw);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->setSpacing(0);
    layout->addWidget(q_check_ptr(m_ribbon));

    m_edit = new CodeEdit(this);
    m_edit->setShowSymbolMark(false);

    m_edit->setReadOnly(true);
    m_edit->setAcceptDrops(false);
    m_edit->setUndoRedoEnabled(false);
    layout->addWidget(m_edit);

    m_status = new QStatusBar(this);
    layout->addWidget(m_status);
    buildUpContent(cw);

    m_syntaxButton = new QToolButton(this);
    m_syntaxButton->setAutoRaise(true);
    m_syntaxButton->setPopupMode(QToolButton::InstantPopup);
    m_syntaxButton->addAction(new SyntaxPopupAction(this));

    m_cancelButton = new QPushButton(this);
    m_cancelButton->setText(tr("Cancel"));
    connect(m_cancelButton, &QPushButton::clicked, this,
            [this]() { m_continue = false; });
    m_status->addPermanentWidget(m_cancelButton);
    m_cancelButton->hide();

    m_status->addPermanentWidget(m_syntaxButton);

    // ok, preparing for starting...
    this->setWindowTitle(tr("TextBroswer"));
    this->setWindowIcon(ICONRES(QStringLiteral("viewtxt")));
    this->setMinimumSize(800, 600);

    this->setUpdatesEnabled(true);
}

ShowTextDialog::~ShowTextDialog() {}

void ShowTextDialog::setSyntax(const KSyntaxHighlighting::Definition &syntax) {
    m_edit->setSyntax(syntax);
    if (syntax.isValid()) {
        m_syntaxButton->setText(syntax.translatedName() + QChar(' ') +
                                QChar(char16_t(0x25BC)));
    } else {
        m_syntaxButton->setText(tr("Plain Text") + QChar(' ') +
                                QChar(char16_t(0x25BC)));
    }
}

void ShowTextDialog::load(QHexBuffer *buffer, const QString &mime,
                          const QString &encoding) {
    constexpr auto MAXLEN = 1024 * 1024 * 1024;
    if (buffer->length() > MAXLEN) {
        WingMessageBox::critical(this, this->windowTitle(), tr("TooHugeFile"));
        return;
    }

    m_syntaxButton->hide();
    setSyntax(WingCodeEdit::nullSyntax());
    show();
    m_status->showMessage(tr("Loading..."));

    QString enc;
    auto ecs = Utilities::getEncodings();
    if (encoding.isEmpty() || !ecs.contains(encoding)) {
        enc = QStringLiteral("ASCII");
    } else {
        enc = encoding;
    }

    this->buffer = buffer->read(qsizetype(0), buffer->length());
    setEncoding(enc);
    setSyntax(WingCodeEdit::syntaxRepo().definitionForMimeType(mime));
}

void ShowTextDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    m_ribbon->setAcceptDrops(false);
    buildEditPage(m_ribbon->addTab(tr("Edit")));
}

RibbonTabContent *ShowTextDialog::buildEditPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();
    {
        auto pannel = tab->addGroup(tr("General"));

        auto a = addPannelAction(pannel, QStringLiteral("copy"), tr("Copy"),
                                 &ShowTextDialog::on_copyfile);
        setPannelActionToolTip(a, QKeySequence::Copy);

        a = addPannelAction(pannel, QStringLiteral("find"), tr("Find"),
                            &ShowTextDialog::on_findfile);
        setPannelActionToolTip(a, QKeySequence::Find);

        a = addPannelAction(pannel, QStringLiteral("jmp"), tr("Goto"),
                            &ShowTextDialog::on_gotoline);
        setPannelActionToolTip(a,
                               shortcuts.keySequence(QKeySequences::Key::GOTO));

        addPannelAction(pannel, QStringLiteral("encoding"), tr("Encoding"),
                        &ShowTextDialog::on_encoding,
                        shortcuts.keySequence(QKeySequences::Key::ENCODING));
        addPannelAction(pannel, QStringLiteral("defines"), tr("UpdateDefs"),
                        &ShowTextDialog::on_updateDefs);
    }

    return tab;
}

void ShowTextDialog::on_copyfile() {
    m_edit->copy();
    Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                 tr("CopyToClipBoard"));
}

void ShowTextDialog::on_findfile() {
    m_edit->showSearchReplaceBar(true, false);
}

void ShowTextDialog::on_gotoline() { m_edit->showGotoBar(true); }

void ShowTextDialog::on_encoding() {
    EncodingDialog d;
    if (d.exec()) {
        auto res = d.getResult();
        setEncoding(res);
    }
}

void ShowTextDialog::on_updateDefs() {
    auto downloadDialog =
        new DefinitionDownloadDialog(&WingCodeEdit::syntaxRepo(), this);
    downloadDialog->setAttribute(Qt::WA_DeleteOnClose);
    downloadDialog->show();
    downloadDialog->raise();
    downloadDialog->activateWindow();
}

void ShowTextDialog::setEncoding(const QString &enc) {
    m_cancelButton->setVisible(true);
    m_continue = true;

    auto e = Utilities::realEncodingName(enc);
    auto en = QStringConverter::encodingForName(e.toUtf8());
    QTextStream ss(this->buffer);
    ss.setEncoding(en.value_or(QStringConverter::Utf8)); // fallback to UTF-8
    ss.setAutoDetectUnicode(false);
    auto total = this->buffer.size();
    m_edit->clear();
    m_edit->setEnabled(false);
    while (m_continue && !ss.atEnd()) {
        auto p = ss.readLine();
        m_edit->appendPlainText(p);
        m_edit->verticalScrollBar()->setValue(0);
        m_status->showMessage(
            tr("Loading...") + QStringLiteral(" (") +
            QString::number(100.0 * ss.pos() / total, 'g', 3) +
            QStringLiteral("%)"));
        qApp->processEvents();
    }

    m_cancelButton->hide();
    m_syntaxButton->setVisible(true);
    m_edit->setEnabled(true);
    m_continue = false;

    auto cursor = m_edit->textCursor();
    cursor.setPosition(0);
    m_edit->setTextCursor(cursor);
    m_status->showMessage(enc);
}

void ShowTextDialog::closeEvent(QCloseEvent *event) {
    m_continue = false;
    m_edit->clear();
    buffer.clear();
    FramelessMainWindow::closeEvent(event);
}
