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

#include "scriptinfobroswer.h"

ScriptInfoBroswer::ScriptInfoBroswer(QWidget *parent) : QTextBrowser(parent) {
    setUndoRedoEnabled(false);
    setOpenLinks(true);
    setOpenExternalLinks(true);
    setReadOnly(true);
    setAcceptRichText(true);
}

void ScriptInfoBroswer::updateData(const ScriptManager::ScriptDirMeta &meta) {
    this->clear();
    _icon = meta.icon;
    _isSys = meta.isSys;

    this->insertHtml(
        QStringLiteral(R"(<p align="center"><img src="scicon://%1"/></p><p />)")
            .arg(meta.rawName));
    this->insertPlainText(tr("RawName:") + meta.rawName + '\n');
    this->insertPlainText(tr("Name:") + meta.name + '\n');
    this->insertPlainText(tr("Author:") + meta.author + '\n');
    this->insertPlainText(tr("License:") + meta.license + '\n');
    this->insertPlainText(tr("ContextMenu:") +
                          (meta.isContextMenu ? QStringLiteral("true")
                                              : QStringLiteral("false")) +
                          '\n');
    this->insertPlainText(tr("HomePage:"));
    this->insertHtml(
        QStringLiteral(R"(<a href="%1">%1</a>)").arg(meta.homepage));
    auto cursor = this->textCursor();
    cursor.insertBlock();
    cursor.setCharFormat({});
    setTextCursor(cursor);
    this->insertPlainText(tr("Comment:"));
    this->insertHtml(meta.comment);
}

QVariant ScriptInfoBroswer::loadResource(int type, const QUrl &name) {
    if (type == QTextDocument::ImageResource) {
        if (QStringLiteral("scicon").compare(name.scheme())) {
            return {};
        }

        constexpr auto wh = 48;
        if (_icon.isNull()) {
            return QPixmap(NAMEICONRES(_isSys
                                           ? QStringLiteral("scriptfolder")
                                           : QStringLiteral("scriptfolderusr")))
                .scaled(wh, wh, Qt::KeepAspectRatio);
        } else {
            auto av = _icon.availableSizes();
            return _icon
                .pixmap(*std::max_element(
                    av.begin(), av.end(),
                    [](const QSize &a, const QSize &b) -> bool {
                        return a.width() < b.width();
                    }))
                .scaled(wh, wh, Qt::KeepAspectRatio);
        }
    }

    return QTextBrowser::loadResource(type, name);
}
