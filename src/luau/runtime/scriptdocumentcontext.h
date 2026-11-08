/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef SCRIPTDOCUMENTCONTEXT_H
#define SCRIPTDOCUMENTCONTEXT_H

#include <functional>
#include <memory>

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QUrl>

class EditorView;
class QHexDocument;

namespace WingHex {
struct DocumentInfo;
}

struct ScriptDocumentIds {
    int handle = 0;
    int context = 0;
};

// Owned by clearLuaContext(), not by the Lua document tables or the editor.
class ScriptDocumentContext final : public QObject {
public:
    using CurrentView = std::function<EditorView *()>;
    using OpenView = std::function<int(const QUrl &, EditorView **)>;
    using CloseView = std::function<int(EditorView *, bool)>;

public:
    ScriptDocumentContext(CurrentView current, OpenView open, CloseView close,
                          std::shared_ptr<ScriptDocumentIds> ids, int token);
    virtual ~ScriptDocumentContext() override;

public:
    const QObject *sender() const;
    int token() const;
    QSet<int> handles() const;
    void cleanHandles(const QSet<int> &handles);

public:
    EditorView *view() const;
    int bind(EditorView *target, bool select = true);
    int openCurrent();
    int openFile(const QUrl &file);
    bool isFileOpened(int handle) const;
    QList<QUrl> openedFiles() const;
    WingHex::DocumentInfo documentInfo(int handle) const;
    bool switchDocument(int handle);
    bool raiseDocument(int handle);
    int closeFile(int handle, bool force);
    int closeFileHandle(int handle);
    int closeCurrent();
    bool beginMacro(const QString &label);
    bool endMacro();
    bool isMacroEmpty() const;
    bool resetMacro();

private:
    bool finishMacro();
    static bool onGuiThread();
    CurrentView m_current;
    OpenView m_open;
    CloseView m_close;
    std::shared_ptr<ScriptDocumentIds> m_ids;
    QHash<int, QPointer<EditorView>> m_handles;
    QPointer<QHexDocument> m_macroDocument;
    int m_macroDepth = 0;
    int m_token;
    int m_active = -1;
};

#endif // SCRIPTDOCUMENTCONTEXT_H
