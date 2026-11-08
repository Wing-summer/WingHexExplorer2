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

#include "WingPlugin/iwingplugincalls.h"
#include "control/editorview.h"
#include <QApplication>
#include <QThread>
#include <QUndoCommand>
#include <QUndoStack>
#include <limits>
#include <utility>

ScriptDocumentContext::ScriptDocumentContext(
    CurrentView current, OpenView open, CloseView close,
    std::shared_ptr<ScriptDocumentIds> ids, int token)
    : _current(std::move(current)), _open(std::move(open)),
      _close(std::move(close)), _ids(std::move(ids)), _token(token) {}

ScriptDocumentContext::~ScriptDocumentContext() {
    while (_macroDepth > 0) {
        finishMacro();
    }
}

const QObject *ScriptDocumentContext::sender() const { return this; }

int ScriptDocumentContext::token() const { return _token; }

QSet<int> ScriptDocumentContext::handles() const {
    QSet<int> result;
    for (auto it = _handles.cbegin(); it != _handles.cend(); ++it) {
        result.insert(it.key());
    }
    return result;
}

void ScriptDocumentContext::cleanHandles(const QSet<int> &handles) {
    // Release access without closing editors or consulting macro guards.
    _handles.removeIf(
        [&handles](const QPair<int, QPointer<EditorView>> &entry) {
            return !handles.contains(entry.first);
        });
    if (_active >= 0 && !_handles.contains(_active)) {
        _active = -1;
    }
}

EditorView *ScriptDocumentContext::view() const {
    auto target = _active < 0 ? _current() : _handles.value(_active).data();
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
    for (auto it = _handles.cbegin(); it != _handles.cend(); ++it) {
        if (it.value() == target) {
            if (select) {
                _active = it.key();
            }
            return it.key();
        }
    }
    if (_handles.size() >= UCHAR_MAX ||
        _ids->handle == std::numeric_limits<int>::max()) {
        return WingHex::ErrFile::TooManyOpenedFile;
    }
    const auto handle = _ids->handle++;
    _handles.insert(handle, target);
    if (select) {
        _active = handle;
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
    if (_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    EditorView *target = nullptr;
    const auto result = _open(file, &target);
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
        _close(target, true);
    }
    return handle;
}

bool ScriptDocumentContext::isFileOpened(int handle) const {
    const auto target = _handles.value(handle);
    return handle >= 0 && target && !target->isClosed();
}

QList<QUrl> ScriptDocumentContext::openedFiles() const {
    QList<QUrl> files;
    for (const auto &target : _handles) {
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
    auto target = handle < 0 ? nullptr : _handles.value(handle).data();
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
    if (!onGuiThread() || _macroDepth > 0) {
        return false;
    }
    auto target = _handles.value(handle);
    if (handle >= 0 && (!target || target->isClosed())) {
        return false;
    }
    _active = handle < 0 ? -1 : handle;
    return true;
}

bool ScriptDocumentContext::raiseDocument(int handle) {
    if (!onGuiThread()) {
        return false;
    }
    auto target = handle < 0 ? _current() : _handles.value(handle).data();
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
    if (_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    auto target = _handles.value(handle);
    if (!target || target->isClosed()) {
        return WingHex::ErrFile::NotExist;
    }
    const auto identity = target.data();
    const auto result = _close(identity, force);
    if (result == WingHex::ErrFile::Success) {
        _handles.removeIf(
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
    if (_macroDepth > 0) {
        return WingHex::ErrFile::Error;
    }
    return handle >= 0 && _handles.remove(handle) ? WingHex::ErrFile::Success
                                                  : WingHex::ErrFile::NotExist;
}

int ScriptDocumentContext::closeCurrent() {
    const auto result = closeFileHandle(_active);
    if (result == WingHex::ErrFile::Success) {
        _active = -1;
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
    if (_macroDepth == std::numeric_limits<int>::max() ||
        (_macroDepth > 0 && _macroDocument != document)) {
        return false;
    }
    document->beginMarco(label);
    _macroDocument = document;
    ++_macroDepth;
    return true;
}

bool ScriptDocumentContext::endMacro() {
    if (!onGuiThread()) {
        return false;
    }
    return finishMacro();
}

bool ScriptDocumentContext::isMacroEmpty() const {
    if (_macroDepth == 0 || !_macroDocument) {
        return true;
    }
    const auto *stack = _macroDocument->undoStack();
    const auto *command =
        stack->count() ? stack->command(stack->count() - 1) : nullptr;
    return !command || command->childCount() == 0;
}

bool ScriptDocumentContext::resetMacro() {
    if (!onGuiThread()) {
        return false;
    }
    if (_macroDepth == 0 || !_macroDocument) {
        return false;
    }
    const auto document = _macroDocument;
    const bool empty = isMacroEmpty();
    while (_macroDepth > 0) {
        finishMacro();
    }
    if (document && !empty) {
        document->undoStack()->undo();
    }
    return true;
}

bool ScriptDocumentContext::finishMacro() {
    if (_macroDepth == 0) {
        return false;
    }
    const auto document = _macroDocument;
    if (--_macroDepth == 0) {
        _macroDocument.clear();
    }
    if (document) {
        document->endMarco();
    }
    return true;
}

bool ScriptDocumentContext::onGuiThread() {
    return QThread::currentThread() == qApp->thread();
}
