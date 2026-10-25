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

#ifndef QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTDOCUMENTBINDINGS_H
#define QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTDOCUMENTBINDINGS_H

#include "class/plugin/pluginsystem.h"
#include "control/editorview.h"
#include "generated_bindings.hpp"
#include <QApplication>
#include <QPointer>
#include <QThread>
#include <QUndoCommand>
#include <QUndoStack>
#include <limits>

// One VM owns its document handles. Existing plugin maps, IDs and validation
// are untouched; calls reuse the application's EditorView implementation.
class ScriptDocumentContext {
public:
    using CurrentView = std::function<EditorView *()>;
    using FindView = std::function<EditorView *(const QUrl &)>;
    ScriptDocumentContext(CurrentView current, FindView find)
        : _current(std::move(current)), _find(std::move(find)) {}
    ~ScriptDocumentContext() {
        while (!_macros.isEmpty()) {
            endMacro();
        }
    }
    const QObject *sender() const { return &_sender; }
    EditorView *view() const {
        if (!qApp || QThread::currentThread() != qApp->thread()) {
            throw std::runtime_error(
                "Script document access requires the GUI thread");
        }
        return _active < 0 ? _current() : _handles.value(_active).data();
    }
    int bind(EditorView *view) {
        if (!view) {
            return int(WingHex::ErrFile::NotExist);
        }
        for (auto it = _handles.begin(); it != _handles.end();) {
            if (it.value().isNull()) {
                it = _handles.erase(it);
            } else {
                if (it.value() == view) {
                    _active = it.key();
                    return _active;
                }
                ++it;
            }
        }
        if (_handles.size() >= 256) {
            return int(WingHex::ErrFile::TooManyOpenedFile);
        }
        while (_handles.contains(_next)) {
            _next = _next == std::numeric_limits<int>::max() ? 0 : _next + 1;
        }
        _active = _next;
        _handles.insert(_active, view);
        return _active;
    }
    int openCurrent() { return bind(view()); }
    int openFile(const QUrl &file) { return bind(_find(file)); }
    bool switchDocument(int handle) {
        if (!_macros.isEmpty() ||
            (handle >= 0 && _handles.value(handle).isNull())) {
            return false;
        }
        _active = handle < 0 ? -1 : handle;
        return true;
    }
    bool raiseDocument(int handle) {
        auto *target = handle < 0 ? _current() : _handles.value(handle).data();
        if (!target) {
            return false;
        }
        target->raise();
        return true;
    }
    WingHex::ErrFile closeFile(int handle) {
        if (!_macros.isEmpty()) {
            return WingHex::ErrFile::Error;
        }
        if (handle < 0 || !_handles.remove(handle)) {
            return WingHex::ErrFile::NotExist;
        }
        if (_active == handle) {
            _active = -1;
        }
        return WingHex::ErrFile::Success;
    }
    WingHex::ErrFile closeCurrent() { return closeFile(_active); }
    bool beginMacro(const QString &label) {
        auto *target = view();
        if (!target) {
            return false;
        }
        auto *document = target->hexEditor()->document().data();
        if (!_macros.isEmpty() && _macros.last() != document) {
            return false;
        }
        document->beginMarco(label);
        _macros.append(document);
        return true;
    }
    bool endMacro() {
        if (_macros.isEmpty()) {
            return false;
        }
        const auto document = _macros.takeLast();
        if (document) {
            document->endMarco();
        }
        return true;
    }
    bool isMacroEmpty() const {
        if (_macros.isEmpty() || !_macros.last()) {
            return true;
        }
        const auto *stack = _macros.last()->undoStack();
        const auto *command =
            stack->count() ? stack->command(stack->count() - 1) : nullptr;
        return !command || command->childCount() == 0;
    }
    bool resetMacro() {
        if (_macros.isEmpty() || !_macros.last()) {
            return false;
        }
        const auto document = _macros.last();
        const bool empty = isMacroEmpty();
        while (!_macros.isEmpty()) {
            endMacro();
        }
        if (document && !empty) {
            document->undoStack()->undo();
        }
        return true;
    }

private:
    QObject _sender;
    CurrentView _current;
    FindView _find;
    QHash<int, QPointer<EditorView>> _handles;
    QList<QPointer<QHexDocument>> _macros;
    int _active = -1;
    int _next = 0;
};

#endif // QTLUAU_INTEGRATIONS_WINGHEX_SCRIPTDOCUMENTBINDINGS_H
