/****************************************************************************
**
** Copyright (C) 2006-2009 fullmetalcoder <fullmetalcoder@hotmail.fr>
**
** This file is part of the Edyuk project <http://edyuk.org>
**
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qeditconfig.h"

/*!
        \file qeditconfig.cpp
        \brief Implementation of the QEditConfig class.

        \see QEditConfig
*/

#include "control/codeedit.h"
#include "ui_qeditconfig.h"
#include "utilities.h"

#include "class/skinmanager.h"

#include <QDebug>

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

/*!
        \ingroup dialogs
        @{

        \class QEditConfig
        \brief A minimalistic, easy to embed, settings widget.

*/

QEditConfig::QEditConfig(bool isConsole, QWidget *w)
    : WingHex::SettingPage(w), ui(new Ui::QEditConfig()) {
    ui->setupUi(this);

    loadThemes();
    loadIndentations();

    QFile code(QStringLiteral(":/com.wingsummer.winghex/src/TESTCODE.as"));
    auto ret = code.open(QFile::ReadOnly);
    Q_ASSERT(ret);
    Q_UNUSED(ret);

    auto cbuffer = code.readAll();
    _edit = new CodeEdit(this);
    _edit->setPlainText(QString::fromUtf8(cbuffer));

    _edit->setReadOnly(true);
    _edit->setUndoRedoEnabled(false);
    _edit->setSyntax(WingCodeEdit::syntaxRepo().definitionForName(
        QStringLiteral("AngelScript")));
    ui->layoutEdit->addWidget(_edit);

    connect(ui->cbTheme, QOverload<int>::of(&QComboBox::currentIndexChanged),
            _edit, [this](int index) {
                if (index == 0) {
                    switch (SkinManager::instance().currentTheme()) {
                    case SkinManager::Theme::Dark:
                        _edit->setTheme(WingCodeEdit::syntaxRepo().defaultTheme(
                            KSyntaxHighlighting::Repository::DarkTheme));
                        break;
                    case SkinManager::Theme::Light:
                        _edit->setTheme(WingCodeEdit::syntaxRepo().defaultTheme(
                            KSyntaxHighlighting::Repository::LightTheme));
                        break;
                    }
                } else {
                    _edit->setTheme(WingCodeEdit::syntaxRepo().theme(
                        ui->cbTheme->itemText(index)));
                }
            });

    auto font = _edit->font();
    ui->cbFont->setCurrentFont(font);
    ui->spnFontSize->setValue(font.pointSize());
    connect(ui->cbFont, &QFontComboBox::currentFontChanged, _edit,
            [this](const QFont &font) {
                auto f = font;
                f.setPointSize(ui->spnFontSize->value());
                _edit->setFont(f);
            });
    connect(ui->spnFontSize, QOverload<int>::of(&QSpinBox::valueChanged), _edit,
            [this](int value) {
                auto f = ui->cbFont->currentFont();
                f.setPointSize(value);
                _edit->setFont(f);
            });
    connect(ui->spnTabWidth, QOverload<int>::of(&QSpinBox::valueChanged), _edit,
            &CodeEdit::setIndentWidth);
    connect(ui->cbIndentation,
            QOverload<int>::of(&QComboBox::currentIndexChanged), _edit,
            [this](int index) {
                bool b;
                auto iden = WingCodeEdit::IndentationMode(
                    ui->cbIndentation->itemData(index).toInt(&b));
                if (b) {
                    _edit->setIndentationMode(iden);
                }
            });
    connect(ui->chkShowLineNumber, &QCheckBox::toggled, _edit,
            &CodeEdit::setShowLineNumbers);
    connect(ui->chkShowFolding, &QCheckBox::toggled, _edit,
            &CodeEdit::setShowFolding);
    connect(ui->chkShowIndentGuides, &QCheckBox::toggled, _edit,
            &CodeEdit::setShowIndentGuides);
    connect(ui->chkLongLineEdge, &QCheckBox::toggled, _edit,
            &CodeEdit::setShowLongLineEdge);
    connect(ui->chkWordWrap, &QCheckBox::toggled, _edit,
            &CodeEdit::setWordWrap);
    connect(ui->chkShowWhitespace, &QCheckBox::toggled, _edit,
            &CodeEdit::setShowWhitespace);
    connect(ui->chkMatchBraces, &QCheckBox::toggled, _edit,
            &CodeEdit::setMatchBraces);
    connect(ui->chkAutoCloseChar, &QCheckBox::toggled, _edit,
            &CodeEdit::setAutoCloseChar);

    reload();
}

QEditConfig::~QEditConfig() { delete ui; }

/*!
        \brief Apply changes
*/
void QEditConfig::apply() {
    QFont font = ui->cbFont->currentFont();
    font.setPointSize(ui->spnFontSize->value());
}

/*!
        \brief Reset the subcontrols to reflect the current settings

        The name can be a bit misleading at first, it has been chosen
        because it directly maps to the effect a "cancel" button would
        have on the widget
*/
void QEditConfig::cancel() { reload(); }

/*!
        \brief Restore default values for all subcontrols

        \note The widgets are changed but these changes are NOT applied.
*/
void QEditConfig::reset() {
    // restore default configuration
    auto font = qApp->font();
    ui->cbFont->setFont(font);
    ui->spnFontSize->setValue(font.pointSize());

    ui->spnTabWidth->setValue(4);

    ui->chkShowWhitespace->setChecked(true);
    ui->chkShowIndentGuides->setChecked(true);
    ui->chkShowLineNumber->setChecked(true);

    // ui->chkDetectLE->setChecked(true);
    // ui->cbLineEndings->setCurrentIndex(0);

    // ui->chkReplaceTabs->setChecked(true);
    // ui->chkAutoRemoveTrailingWhitespace->setChecked(true);
    // ui->chkPreserveTrailingIndent->setChecked(false);

    apply();
}

void QEditConfig::reload() {
    // reload the current config
    // auto dfont = QEditor::defaultFont();
    // ui->cbFont->setFont(dfont);
    // ui->spnFontSize->setValue(dfont.pointSize());

    // ui->spnTabWidth->setValue(QEditor::defaultTabStop());

    // auto ss = QEditor::defaultShowSpaces();
    // ui->chkShowTabsInText->setChecked(ss.testFlag(QDocument::ShowTabs));
    // ui->chkShowLeadingWhitespace->setChecked(
    //     ss.testFlag(QDocument::ShowLeading));
    // ui->chkShowTrailingWhitespace->setChecked(
    //     ss.testFlag(QDocument::ShowTrailing));

    // auto le = QEditor::defaultLineEnding();
    // ui->chkDetectLE->setChecked(le == QDocument::Conservative);
    // ui->cbLineEndings->setCurrentIndex(le ? le - 1 : 0);

    // int flag = QEditor::defaultFlags();
    // ui->chkReplaceTabs->setChecked(flag & QEditor::ReplaceTabs);
    // ui->chkAutoRemoveTrailingWhitespace->setChecked(flag &
    //                                                 QEditor::RemoveTrailing);
    // ui->chkPreserveTrailingIndent->setChecked(flag &
    //                                           QEditor::PreserveTrailingIndent);
}

void QEditConfig::loadThemes() {
    QStringList themes;
    themes.append(tr("Default"));
    for (auto &th : CodeEdit::syntaxRepo().themes()) {
        themes.append(th.name());
    }
    ui->cbTheme->addItems(themes);
}

void QEditConfig::loadIndentations() {
    ui->cbIndentation->addItem(tr("IndentMixed"),
                               int(WingCodeEdit::IndentationMode::IndentMixed));
    ui->cbIndentation->addItem(
        tr("IndentSpaces"), int(WingCodeEdit::IndentationMode::IndentSpaces));
    ui->cbIndentation->addItem(tr("IndentTabs"),
                               int(WingCodeEdit::IndentationMode::IndentTabs));
}

QIcon QEditConfig::categoryIcon() const { return ICONRES("file"); }

QString QEditConfig::name() const { return tr("Edit"); }

QString QEditConfig::id() const { return QStringLiteral("Edit"); }
