/*==============================================================================
** Copyright (C) 2024-2027 WingSummer
**
** You can redistribute this file and/or modify it under the terms of the
** BSD 3-Clause.
**
** THIS FILE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** =============================================================================
*/

#ifndef WINGCORE_H
#define WINGCORE_H

#include <QDebug>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QString>

constexpr auto CALL_TABLE_PROPERTY = "__CALL_TABLE__";
constexpr auto CALL_POINTER_PROPERTY = "__CALL_POINTER__";

template <class Func>
inline QByteArray getFunctionSig(Func &&, const char *fn) {
    typedef QtPrivate::FunctionPointer<std::decay_t<Func>> FnPointerType;
    const int *types =
        QtPrivate::ConnectionTypes<typename FnPointerType::Arguments>::types();
    if constexpr (FnPointerType::ArgumentCount > 0) {
        QStringList args;
        Q_ASSERT(types);
        for (int i = 0; i < FnPointerType::ArgumentCount; ++i) {
            QMetaType type(types[i]);
            if (type.isValid()) {
                args.append(type.name());
            }
        }
        return QByteArray(fn) + '(' + args.join(',').toLatin1() + ')';
    } else {
        return QByteArray(fn) + QByteArray("()");
    }
}

#define SETUP_CALL_CONTEXT(FN)                                                 \
    static QMutex __mutex__;                                                   \
    static QMetaMethod m;                                                      \
    do {                                                                       \
        QMutexLocker locker(&__mutex__);                                       \
        static auto CALLNAME = getFunctionSig(FN, __func__);                   \
        if (!m.isValid()) {                                                    \
            auto fnMap = callTable();                                          \
            if (fnMap.contains(CALLNAME)) {                                    \
                m = fnMap.value(CALLNAME);                                     \
                Q_ASSERT(m.isValid());                                         \
            } else {                                                           \
                qDebug("[InvokeCall] '%s' is not found in call table.",        \
                       CALLNAME.constData());                                  \
            }                                                                  \
        }                                                                      \
    } while (0)

#endif // WINGCORE_H
