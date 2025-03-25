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

#include "qeditconfig.h"

/*!
        \file qeditconfig.cpp
        \brief Implementation of the QEditConfig class.

        \see QEditConfig
*/

#include "class/scriptsettings.h"
#include "control/codeedit.h"
#include "control/scriptingconsolebase.h"
#include "ui_qeditconfig.h"
#include "utilities.h"

#include "class/skinmanager.h"

#include <QVBoxLayout>

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
    : WingHex::SettingPage(w), ui(new Ui::QEditConfig()),
      m_isConsole(isConsole) {
    ui->setupUi(this);

    loadThemes();
    loadIndentations();

    if (isConsole) {
        auto console = new ScriptingConsoleBase(this);
        console->initOutput();
        auto outfmt =
            console->channelCharFormat(QConsoleWidget::StandardOutput);
        console->write(
            QStringLiteral(R"(print("WingHexExplorer2 by wingsummer!");)"),
            outfmt);
        console->appendCommandPrompt();
        console->write(QStringLiteral(R"(for(auto i = 0; i < 5; i++) { \)"),
                       outfmt);
        console->appendCommandPrompt(true);
        console->write(QStringLiteral(R"(print(i); \)"), outfmt);
        console->appendCommandPrompt(true);
        console->write(QStringLiteral("}"), outfmt);
        console->newLine();
        console->stdOut(QStringLiteral("01234"));
        _edit = console;
    } else {
        _edit = new WingCodeEdit(this);
        QFile code(QStringLiteral(":/com.wingsummer.winghex/src/TESTCODE.as"));
        auto ret = code.open(QFile::ReadOnly);
        Q_ASSERT(ret);
        Q_UNUSED(ret);
        auto cbuffer = code.readAll();
        _edit->setPlainText(QString::fromUtf8(cbuffer));
    }

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
    connect(ui->chkAutoIden, &QCheckBox::toggled, _edit,
            &CodeEdit::setAutoIndent);

    if (isConsole) {
        ui->chkShowLineNumber->setEnabled(false);
        ui->chkShowFolding->setEnabled(false);
        ui->chkShowIndentGuides->setEnabled(false);
        ui->chkWordWrap->setEnabled(false);
        ui->chkLongLineEdge->setEnabled(false);
        ui->chkAutoIden->setEnabled(false);

        _name = tr("Console");
        _id = QStringLiteral("Console");
        _icon = ICONRES("console");
    } else {
        _name = tr("Edit");
        _id = QStringLiteral("Edit");
        _icon = ICONRES("file");
    }

    reload();
}

QEditConfig::~QEditConfig() { delete ui; }

/*!
        \brief Apply changes
*/
void QEditConfig::apply() {
    auto &set = ScriptSettings::instance();
    if (m_isConsole) {
        if (ui->cbTheme->currentIndex() == 0) {
            set.setConsoleTheme({});
        } else {
            set.setConsoleTheme(ui->cbTheme->currentText());
        }

        set.setConsoleFontFamily(ui->cbFont->currentFont().family());
        set.setConsoleFontSize(ui->spnFontSize->value());
        set.setConsoleTabWidth(ui->spnTabWidth->value());
        set.setConsoleInden(ui->cbIndentation->currentData().toInt());

        set.setConsoleMatchBraces(ui->chkMatchBraces->isChecked());
        set.setConsoleShowWhiteSpace(ui->chkShowWhitespace->isChecked());
        set.setConsoleAutoCloseChar(ui->chkAutoCloseChar->isChecked());
    } else {
        if (ui->cbTheme->currentIndex() == 0) {
            set.setEditorTheme({});
        } else {
            set.setEditorTheme(ui->cbTheme->currentText());
        }

        set.setEditorFontFamily(ui->cbFont->currentFont().family());
        set.setEditorFontSize(ui->spnFontSize->value());
        set.setEditorTabWidth(ui->spnTabWidth->value());
        set.setEditorInden(ui->cbIndentation->currentData().toInt());

        set.setEditorShowLineNumber(ui->chkShowLineNumber->isChecked());
        set.setEditorFolding(ui->chkShowFolding->isChecked());
        set.setEditorShowGuideLine(ui->chkShowIndentGuides->isChecked());
        set.setEditorWordWrap(ui->chkWordWrap->isChecked());
        set.setEditorShowLineEdges(ui->chkLongLineEdge->isChecked());
        set.setEditorMatchBraces(ui->chkMatchBraces->isChecked());
        set.setEditorShowWhiteSpace(ui->chkShowWhitespace->isChecked());
        set.setEditorAutoCloseChar(ui->chkAutoCloseChar->isChecked());
        set.setEditorAutoIden(ui->chkAutoIden->isChecked());
    }
    set.save(m_isConsole ? ScriptSettings::CONSOLE : ScriptSettings::EDITOR);
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
    auto &set = ScriptSettings::instance();
    set.reset(m_isConsole ? ScriptSettings::CONSOLE : ScriptSettings::EDITOR);
    reload();
}

void QEditConfig::reload() {
    auto &set = ScriptSettings::instance();

    auto e = QMetaEnum::fromType<WingCodeEdit::IndentationMode>();

    // reload the current config
    if (m_isConsole) {
        auto theme = set.consoleTheme();
        if (theme.isEmpty()) {
            ui->cbTheme->setCurrentIndex(0);
        } else {
            ui->cbTheme->setCurrentText(set.consoleTheme());
        }

        auto dfont = QFont(set.consoleFontFamily());
        ui->cbFont->setCurrentFont(dfont);
        ui->spnFontSize->setValue(set.consoleFontSize());
        ui->spnTabWidth->setValue(set.consoleTabWidth());

        ui->cbIndentation->setCurrentText(e.valueToKey(set.consoleInden()));
        ui->chkMatchBraces->setChecked(set.consoleMatchBraces());
        ui->chkShowWhitespace->setChecked(set.consoleShowWhiteSpace());
        ui->chkAutoCloseChar->setChecked(set.consoleAutoCloseChar());
    } else {
        auto theme = set.editorTheme();
        if (theme.isEmpty()) {
            ui->cbTheme->setCurrentIndex(0);
        } else {
            ui->cbTheme->setCurrentText(set.consoleTheme());
        }

        auto dfont = QFont(set.editorFontFamily());
        ui->cbFont->setCurrentFont(dfont);
        ui->spnFontSize->setValue(set.editorFontSize());
        ui->spnTabWidth->setValue(set.editorTabWidth());

        ui->cbIndentation->setCurrentText(e.valueToKey(set.editorInden()));
        ui->chkShowLineNumber->setChecked(set.editorShowLineNumber());
        ui->chkShowFolding->setChecked(set.editorFolding());
        ui->chkShowIndentGuides->setChecked(set.editorShowGuideLine());
        ui->chkWordWrap->setChecked(set.editorWordWrap());
        ui->chkLongLineEdge->setChecked(set.editorShowLineEdges());
        ui->chkMatchBraces->setChecked(set.editorMatchBraces());
        ui->chkShowWhitespace->setChecked(set.editorShowWhiteSpace());
        ui->chkAutoCloseChar->setChecked(set.editorAutoCloseChar());
    }
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
    ui->cbIndentation->addItem(
        tr("IndentSpaces"), int(WingCodeEdit::IndentationMode::IndentSpaces));
    ui->cbIndentation->addItem(tr("IndentTabs"),
                               int(WingCodeEdit::IndentationMode::IndentTabs));
    ui->cbIndentation->addItem(tr("IndentMixed"),
                               int(WingCodeEdit::IndentationMode::IndentMixed));
    ui->cbIndentation->setCurrentIndex(0);
}

QIcon QEditConfig::categoryIcon() const { return _icon; }

QString QEditConfig::name() const { return _name; }

QString QEditConfig::id() const { return _id; }
