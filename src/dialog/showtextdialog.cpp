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
#include "class/wingprogressdialog.h"
#include "control/codeedit.h"
#include "control/toast.h"
#include "dialog/encodingdialog.h"
#include "qeditor.h"

#include <QThread>
#include <QVBoxLayout>

constexpr auto EMPTY_FUNC = [] {};

ShowTextDialog::ShowTextDialog(QWidget *parent) : FramelessDialogBase(parent) {
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

    m_edit = new CodeEdit(false, this, false);
    auto editor = m_edit->editor();
    editor->setCodec(QStringLiteral("ASCII"));
    connect(editor, &QEditor::needLoading, this,
            [this]() { load(m_last.buffer, m_last.offset, m_last.size); });

    editor->setFlag(QEditor::ReadOnly, true);
    editor->setAcceptDrops(false);
    editor->setUndoRedoEnabled(false);
    editor->createSimpleBasicContextMenu(true, true);
    layout->addWidget(editor);

    m_status = new QStatusBar(this);
    layout->addWidget(m_status);
    buildUpContent(cw);

    // ok, preparing for starting...
    this->setWindowTitle(tr("TextBroswer"));
    this->setWindowIcon(ICONRES(QStringLiteral("viewtxt")));
    this->setMinimumSize(800, 600);

    this->setUpdatesEnabled(true);
}

ShowTextDialog::~ShowTextDialog() {}

void ShowTextDialog::load(QHexBuffer *buffer, const QString encoding,
                          qsizetype offset, qsizetype size) {
    auto editor = m_edit->editor();
    editor->blockSignals(true);
    editor->setCodec(encoding);
    load(buffer, offset, size);
    editor->blockSignals(false);
}

void ShowTextDialog::load(QHexBuffer *buffer, qsizetype offset,
                          qsizetype size) {
    if (buffer == nullptr || offset < 0) {
        return;
    }

    m_canceled = false;

    if (size < 0) {
        size = buffer->length();
    }

    if (size >= std::numeric_limits<decltype(size)>::max()) {
        WingMessageBox::critical(this, this->windowTitle(), tr("TooHugeFile"));
        return;
    }

    auto editor = m_edit->editor();
    editor->setUpdatesEnabled(false);

    auto doc = editor->document();
    auto orign = offset;

    WingProgressDialog pdialog(tr("Loading"), tr("Cancel"), 0, size - orign);
    pdialog.setAutoClose(false);
    pdialog.setAutoReset(true);

    connect(&pdialog, &WingProgressDialog::canceled, this,
            &ShowTextDialog::on_cancel);
    pdialog.open();

    QEventLoop loop;

    auto th = QThread::create([&] {
        QByteArray ba;
        auto codec = editor->codecName();

        doc->startChunkLoading();
        constexpr auto CHUNK_SIZE = 100000;

        do {
            ba = buffer->read(offset, CHUNK_SIZE);
            offset += ba.size();

            if (codec.size())
                doc->addChunk(QCE::convertString(codec, ba));
            else
                doc->addChunk(QString::fromLatin1(ba));

            QMetaObject::invokeMethod(
                qApp, [&] { pdialog.setValue(offset - orign); });
            qApp->processEvents();
        } while (!m_canceled && (offset < size) && ba.size());

        pdialog.setRange(-1, -1);
        doc->stopChunkLoading();
        editor->setCursor(QDocumentCursor(doc));
        pdialog.close();
    });

    connect(th, &QThread::finished, this,
            [this, buffer, orign, th, &offset, &loop] {
                // record
                m_last.buffer = buffer;
                m_last.offset = orign;
                m_last.size = offset;

                show();
                loop.quit();
                th->deleteLater();
            });

    th->start();

    loop.exec();
    editor->setUpdatesEnabled(true);
    m_status->showMessage(editor->codecName());
}

void ShowTextDialog::buildUpRibbonBar() {
    m_ribbon = new Ribbon(this);
    m_ribbon->setAcceptDrops(false);
    buildEditPage(m_ribbon->addTab(tr("Edit")));
    buildViewPage(m_ribbon->addTab(tr("View")));
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
    }

    return tab;
}

RibbonTabContent *ShowTextDialog::buildViewPage(RibbonTabContent *tab) {
    auto shortcuts = QKeySequences::instance();

    auto pannel = tab->addGroup(tr("Display"));
    auto menu = new QMenu(this);
    menu->addAction(newAction(QStringLiteral("80%"),
                              [this] { this->setCurrentEditorScale(0.8); }));
    menu->addAction(newAction(QStringLiteral("90%"),
                              [this] { this->setCurrentEditorScale(0.9); }));
    menu->addAction(newAction(QStringLiteral("100%"),
                              [this] { this->setCurrentEditorScale(1.0); }));
    menu->addSeparator();
    menu->addAction(newAction(QStringLiteral("120%"),
                              [this] { this->setCurrentEditorScale(1.2); }));
    menu->addAction(newAction(QStringLiteral("150%"),
                              [this] { this->setCurrentEditorScale(1.5); }));
    menu->addAction(newAction(QStringLiteral("200%"),
                              [this] { this->setCurrentEditorScale(2.0); }));
    menu->addAction(newAction(QStringLiteral("250%"),
                              [this] { this->setCurrentEditorScale(2.5); }));
    menu->addAction(newAction(QStringLiteral("300%"),
                              [this] { this->setCurrentEditorScale(3.0); }));
    addPannelAction(pannel, QStringLiteral("scale"), tr("Scale"), EMPTY_FUNC,
                    {}, menu);
    addPannelAction(pannel, QStringLiteral("scalereset"), tr("ResetScale"),
                    [this] { this->setCurrentEditorScale(1.0); });

    return tab;
}

void ShowTextDialog::setCurrentEditorScale(qreal rate) {
    m_edit->editor()->setScaleRate(rate);
}

void ShowTextDialog::on_copyfile() {
    m_edit->editor()->copy();
    Toast::toast(this, NAMEICONRES(QStringLiteral("copy")),
                 tr("CopyToClipBoard"));
}

void ShowTextDialog::on_findfile() { m_edit->editor()->find(); }

void ShowTextDialog::on_gotoline() { m_edit->editor()->gotoLine(); }

void ShowTextDialog::on_encoding() {
    EncodingDialog d;
    if (d.exec()) {
        auto res = d.getResult();
        m_edit->editor()->setCodec(res);
    }
}

void ShowTextDialog::on_cancel() { m_canceled = true; }
