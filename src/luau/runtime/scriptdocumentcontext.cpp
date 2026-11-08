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

#include "scriptdocumentcontext.h"

#include <limits>
#include <utility>

#include <QApplication>
#include <QThread>
#include <QUndoCommand>
#include <QUndoStack>

#include "WingPlugin/iwingplugincalls.h"

#include "control/editorview.h"

ScriptDocumentContext::ScriptDocumentContext(
    CurrentView current, OpenView open, CloseView close,
    std::shared_ptr<ScriptDocumentIds> ids, int token)
    : m_current(std::move(current)), m_open(std::move(open)),
      m_close(std::move(close)), m_ids(std::move(ids)), m_token(token) {}

ScriptDocumentContext::~ScriptDocumentContext() {
    while (m_macroDepth > 0) {
        finishMacro();
    }
}

const QObject *ScriptDocumentContext::sender() const { return this; }

int ScriptDocumentContext::token() const { return m_token; }

QSet<int> ScriptDocumentContext::handles() const {
    QSet<int> result;
    for (auto it = m_handles.cbegin(); it != m_handles.cend(); ++it) {
        result.insert(it.key());
    }
    return result;
}

void ScriptDocumentContext::cleanHandles(const QSet<int> &handles) {
    // Release access without closing editors or consulting macro guards.
    m_handles.removeIf(
        [&handles](const QPair<int, QPointer<EditorView>> &entry) {
            return !handles.contains(entry.first);
        });
    if (m_active >= 0 && !m_handles.contains(m_active)) {
        m_active = -1;
    }
}

EditorView *ScriptDocumentContext::view() const {
    auto target = m_active < 0 ? m_current() : m_handles.value(m_active).data();
    if (target && target->isCloneFile()) {
        target = target->cloneParent();
    }
    return target && !target->isClosed() ? target : nullptr;
}

int ScriptDocumentContext::bind(EditorView *target, bool select) {
    if (target && target->isCloneFile()) {
        target = target->cloneParent();
    }
    if (!target || target->isClosed()) {
        return WingHex::ErrFile::NotExist;
    }
    for (auto it = m_handles.cbegin(); it != m_handles.cend(); ++it) {
        if (it.value() == target) {
            if (select) {
                m_active = it.key();
            }
            return it.key();
        }
    }
    if (m_handles.size() >= UCHAR_MAX ||
        m_ids->handle == std::numeric_limits<int>::max()) {
        return WingHex::ErrFile::TooManyOpenedFile;
    }
    const auto handle = m_ids->handle++;
    m_handles.insert(handle, target);
    if (select) {
        m_active = handle;
    }
    return handle;
}

int ScriptDocumentContext::openCurrent() {
    if (!onGuiThread()) {
        return WingHex::ErrFile::NotAllowedInNoneGUIThread;
    }
    return bind(view());
}

int ScriptDocumentContext::openFile(const QUrl &file) {
    if (!onGuiThread()) {
        return WingHex::ErrFile::NotAllowedInNoneGUIThread;
    }
    if (m_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    EditorView *target = nullptr;
    const auto result = m_open(file, &target);
    if (result != WingHex::ErrFile::Success &&
        result != WingHex::ErrFile::AlreadyOpened) {
        return result;
    }
    if (!target) {
        return result == WingHex::ErrFile::AlreadyOpened
                   ? result
                   : WingHex::ErrFile::Error;
    }
    const auto handle = bind(target, false);
    if (handle < 0 && result == WingHex::ErrFile::Success) {
        m_close(target, true);
    }
    return handle;
}

bool ScriptDocumentContext::isFileOpened(int handle) const {
    const auto target = m_handles.value(handle);
    return handle >= 0 && target && !target->isClosed();
}

QList<QUrl> ScriptDocumentContext::openedFiles() const {
    QList<QUrl> files;
    for (const auto &target : m_handles) {
        auto view = target.data();
        if (view == nullptr || view->isClosed() ||
            view->documentType() == WingHex::DocumentType::InValid) {
            continue;
        }
        files.append(view->openedFileUrl());
    }
    return files;
}

WingHex::DocumentInfo ScriptDocumentContext::documentInfo(int handle) const {
    auto target = handle < 0 ? nullptr : m_handles.value(handle).data();
    WingHex::DocumentInfo info;
    if (target && !target->isClosed()) {
        info.fileNameUrl = target->fileNameUrl();
        info.workSpaceName = target->workSpaceName();
        info.documentType = target->documentType();
        info.isSaved = target->isSaved();
        info.isReadOnly = target->hexEditor()->isReadOnly();
    }
    return info;
}

bool ScriptDocumentContext::switchDocument(int handle) {
    if (!onGuiThread() || m_macroDepth > 0) {
        return false;
    }
    auto target = m_handles.value(handle);
    if (handle >= 0 && (!target || target->isClosed())) {
        return false;
    }
    m_active = handle < 0 ? -1 : handle;
    return true;
}

bool ScriptDocumentContext::raiseDocument(int handle) {
    if (!onGuiThread()) {
        return false;
    }
    auto target = handle < 0 ? m_current() : m_handles.value(handle).data();
    if (!target || target->isClosed()) {
        return false;
    }
    target->raise();
    return true;
}

int ScriptDocumentContext::closeFile(int handle, bool force) {
    if (!onGuiThread()) {
        return WingHex::ErrFile::NotAllowedInNoneGUIThread;
    }
    if (m_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    auto target = m_handles.value(handle);
    if (!target || target->isClosed()) {
        return WingHex::ErrFile::NotExist;
    }
    const auto identity = target.data();
    const auto result = m_close(identity, force);
    if (result == WingHex::ErrFile::Success) {
        m_handles.removeIf(
            [identity](const QPair<int, QPointer<EditorView>> &entry) {
                auto iden = entry.second;
                return !iden || iden.data() == identity;
            });
    }
    return result;
}

int ScriptDocumentContext::closeFileHandle(int handle) {
    if (!onGuiThread()) {
        return WingHex::ErrFile::NotAllowedInNoneGUIThread;
    }
    if (m_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    return handle >= 0 && m_handles.remove(handle) ? WingHex::ErrFile::Success
                                                   : WingHex::ErrFile::NotExist;
}

int ScriptDocumentContext::closeCurrent() {
    const auto result = closeFileHandle(m_active);
    if (result == WingHex::ErrFile::Success) {
        m_active = -1;
    }
    return result;
}

bool ScriptDocumentContext::beginMacro(const QString &label) {
    if (!onGuiThread()) {
        return false;
    }
    auto *target = view();
    if (!target) {
        return false;
    }
    auto *document = target->hexEditor()->document().data();
    if (m_macroDepth == std::numeric_limits<int>::max() ||
        (m_macroDepth > 0 && m_macroDocument != document)) {
        return false;
    }
    document->beginMarco(label);
    m_macroDocument = document;
    ++m_macroDepth;
    return true;
}

bool ScriptDocumentContext::endMacro() {
    if (!onGuiThread()) {
        return false;
    }
    return finishMacro();
}

bool ScriptDocumentContext::isMacroEmpty() const {
    if (m_macroDepth == 0 || !m_macroDocument) {
        return true;
    }
    const auto *stack = m_macroDocument->undoStack();
    const auto *command =
        stack->count() ? stack->command(stack->count() - 1) : nullptr;
    return !command || command->childCount() == 0;
}

bool ScriptDocumentContext::resetMacro() {
    if (!onGuiThread()) {
        return false;
    }
    if (m_macroDepth == 0 || !m_macroDocument) {
        return false;
    }
    const auto document = m_macroDocument;
    const bool empty = isMacroEmpty();
    while (m_macroDepth > 0) {
        finishMacro();
    }
    if (document && !empty) {
        document->undoStack()->undo();
    }
    return true;
}

bool ScriptDocumentContext::finishMacro() {
    if (m_macroDepth == 0) {
        return false;
    }
    const auto document = m_macroDocument;
    if (--m_macroDepth == 0) {
        m_macroDocument.clear();
    }
    if (document) {
        document->endMarco();
    }
    return true;
}

bool ScriptDocumentContext::onGuiThread() {
    return QThread::currentThread() == qApp->thread();
}
