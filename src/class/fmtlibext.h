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

#ifndef FMTLIBEXT_H
#define FMTLIBEXT_H

#include <fmt/args.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "class/scriptmachine.h"
#include "scriptaddon/scriptdatetime.h"

#include <QString>

inline QString escapeNonPrintable(const QChar &ch) {
    const ushort code = ch.unicode();

    switch (code) {
    case '\\':
        return QStringLiteral("\\\\");
    case '\"':
        return QStringLiteral("\\\"");
    case '\'':
        return QStringLiteral("\\'");
    case '\0':
        return QStringLiteral("\\0");
    case '\n':
        return QStringLiteral("\\n");
    case '\r':
        return QStringLiteral("\\r");
    case '\t':
        return QStringLiteral("\\t");
    case '\b':
        return QStringLiteral("\\b");
    case '\f':
        return QStringLiteral("\\f");
    case '\v':
        return QStringLiteral("\\v");
    case '\a':
        return QStringLiteral("\\a");
    }

    if (ch.isPrint()) {
        return QString(ch);
    }

    if (code <= 0xFF) {
        return QStringLiteral("\\x%1").arg(code, 2, 16, QLatin1Char('0'));
    } else {
        return QStringLiteral("\\u%1").arg(code, 4, 16, QLatin1Char('0'));
    }
}

inline QString escapeNonPrintable(const QString &input) {
    QString out;
    out.reserve(input.size() * 2);
    for (auto &ch : input) {
        out.append(escapeNonPrintable(ch));
    }
    return out;
}

class CScriptArrayView {
public:
    inline explicit CScriptArrayView(CScriptArray *arr) noexcept : arr_(arr) {
        Q_ASSERT(arr);
        arr->AddRef();
    }

    inline ~CScriptArrayView() {
        arr_->Release();
        arr_ = nullptr;
    }

    inline CScriptArray *data() const { return arr_; }

private:
    CScriptArray *arr_ = nullptr;
};

namespace fmt {

template <>
struct formatter<QString> : formatter<const char *> {
    auto format(const QString &qstr, format_context &ctx) const {
        auto utf8 = qstr.toUtf8();
        return fmt::formatter<const char *>::format(utf8.data(), ctx);
    }
};

template <>
struct formatter<QChar, char> {
    std::string spec_; // e.g. "", "04x", ">4", etc.
    // true if user wrote leading '?', e.g. "{:?}" or "{:?04x}"
    bool debug_ = false;

    // parse MUST be non-const
    auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        spec_.clear();
        debug_ = false;

        if (it != end && *it != '}') {
            // Detect leading '?'
            if (*it == '?') {
                debug_ = true;
                ++it;
            }
            // copy rest until '}' into spec_
            auto b = it;
            while (it != end && *it != '}')
                ++it;
            spec_.assign(b, it);
        }
        return it;
    }

    // format SHOULD be const for compatibility with many fmt versions (v12)
    template <typename FormatContext>
    auto format(const QChar &qc, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        auto out = ctx.out();

        // Convert QChar -> UTF-8 std::string (single code unit -> 1..3 bytes)
        QString qs(qc);
        QByteArray ba = qs.toUtf8();
        std::string utf8(ba.constData(), ba.size());

        if (debug_) {
            // use fmt's debug formatting for strings (adds quotes and escapes)
            return fmt::format_to(out, "{}", escapeNonPrintable(qs));
        }

        if (spec_.empty()) {
            return fmt::format_to(out, "{}", fmt::string_view(utf8));
        }

        char type_char = 0;
        for (auto i = spec_.size() - 1; i >= 0; --i) {
            char c = spec_[i];
            if (std::isalpha(static_cast<unsigned char>(c))) {
                type_char = c;
                break;
            }
        }

        if (type_char) {
            const char *intTypes = "diuoxXbB";
            if (std::strchr(intTypes, type_char)) {
                // Treat as integer formatting of code point
                std::string fmtstr = "{:";
                fmtstr += spec_;
                fmtstr += "}";
                fmt::dynamic_format_arg_store<FormatContext> store;
                uint32_t code = static_cast<uint32_t>(qc.unicode());
                store.push_back(code);
                return fmt::vformat_to(out, fmt::string_view(fmtstr), store);
            }
        }

        // Else forward the spec to string formatting (support
        // width/align/fill/precision)
        {
            std::string fmtstr = "{:";
            fmtstr += spec_;
            fmtstr += "}";
            fmt::dynamic_format_arg_store<FormatContext> store;
            store.push_back(utf8);
            return fmt::vformat_to(out, fmt::string_view(fmtstr), store);
        }
    }
};

template <typename Char>
struct formatter<CDateTime, Char> {
    formatter<std::chrono::system_clock::time_point, Char> tp_fmt;

    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return tp_fmt.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const CDateTime &cd, FormatContext &ctx) const {
        return tp_fmt.format(cd.time_point(), ctx);
    }
};

static format_context::iterator
format_element_with_spec(void *elemPtr, int typeId,
                         const std::string &underlying_spec,
                         format_context &ctx) {
    std::string fmt_str = "{";
    if (!underlying_spec.empty()) {
        fmt_str.push_back(':');
        fmt_str += underlying_spec;
    }
    fmt_str.push_back('}');

    fmt::dynamic_format_arg_store<fmt::format_context> store;

    auto &m = ScriptMachine::instance();

    switch (typeId) {
    case asTYPEID_BOOL:
        store.push_back(*static_cast<bool *>(elemPtr));
        break;
    case asTYPEID_INT8:
        store.push_back(*static_cast<qint8 *>(elemPtr));
        break;
    case asTYPEID_INT16:
        store.push_back(*static_cast<qint16 *>(elemPtr));
        break;
    case asTYPEID_INT32:
        store.push_back(*static_cast<qint32 *>(elemPtr));
        break;
    case asTYPEID_INT64:
        store.push_back(*static_cast<qint64 *>(elemPtr));
        break;
    case asTYPEID_UINT8:
        store.push_back(*static_cast<quint8 *>(elemPtr));
        break;
    case asTYPEID_UINT16:
        store.push_back(*static_cast<quint16 *>(elemPtr));
        break;
    case asTYPEID_UINT32:
        store.push_back(*static_cast<quint32 *>(elemPtr));
        break;
    case asTYPEID_UINT64:
        store.push_back(*static_cast<quint64 *>(elemPtr));
        break;
    case asTYPEID_FLOAT:
        store.push_back(*static_cast<float *>(elemPtr));
        break;
    case asTYPEID_DOUBLE:
        store.push_back(*static_cast<double *>(elemPtr));
        break;
    default: {
        if (m.isAngelChar(typeId)) {
            QString r(3, '\'');
            r[1] = *static_cast<QChar *>(elemPtr);
            store.push_back(r);
        } else if (m.isAngelString(typeId)) {
            auto str = *static_cast<QString *>(elemPtr);
            store.push_back(str.prepend('"').append('"'));
        } else if (m.isAngelArray(typeId)) {
            store.push_back(
                CScriptArrayView(static_cast<CScriptArray *>(elemPtr)));
        } else {
            store.push_back(m.stringify_helper(elemPtr, typeId));
        }
    } break;
    }

    return fmt::vformat_to(ctx.out(), fmt::string_view(fmt_str), store);
}

template <>
struct formatter<CScriptArrayView> {
    bool no_brackets_ = false; // 'n' flag
    enum RangeType {
        NONE = 0,
        AS_STRING = 1,
        AS_DEBUG_STRING = 2
    } range_type_ = NONE;
    std::string underlying_spec_;

    // ["n"][range_type][":" underlying_spec]
    constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        // optional 'n'
        if (it != end && *it == 'n') {
            no_brackets_ = true;
            ++it;
        }

        // optional range_type: 's' or '?s'
        if (it != end) {
            if (*it == '?' && std::next(it) != end && *std::next(it) == 's') {
                range_type_ = AS_DEBUG_STRING;
                it += 2;
            } else if (*it == 's') {
                range_type_ = AS_STRING;
                ++it;
            }
        }

        // optional ':' underlying_spec (we capture until '}' or end of spec)
        if (it != end && *it == ':') {
            ++it; // skip ':'
            auto spec_begin = it;
            // read until closing '}' or end
            while (it != end && *it != '}')
                ++it;
            underlying_spec_.assign(spec_begin, it);
        }

        // return iterator to the end of parse (fmt expects it)
        return it;
    }

    template <typename FormatContext>
    auto format(const CScriptArrayView &arrv, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        auto out = ctx.out();
        auto arr = arrv.data();
        asUINT n = arr->GetSize();
        int typeId = arr->GetElementTypeId();

        // handle range_type as string if requested and element is char-like:
        if (range_type_ != NONE) {
            auto &m = ScriptMachine::instance();
            if (m.isAngelChar(typeId)) {
                QString data;
                data.reserve(n);

                for (asUINT i = 0; i < n; ++i) {
                    const void *elemPtr = arr->At(i);
                    data.append(*static_cast<const QChar *>(elemPtr));
                }

                return fmt::format_to(
                    out, range_type_ == AS_DEBUG_STRING ? "{:?}" : "{}", data);
            }
        }

        if (!no_brackets_)
            out = fmt::format_to(out, "[");

        for (asUINT i = 0; i < n; ++i) {
            if (i) {
                out = fmt::format_to(out, ", ");
            }
            const void *elemPtr = arr->At(i);
            out = format_element_with_spec(const_cast<void *>(elemPtr), typeId,
                                           underlying_spec_, ctx);
        }

        if (!no_brackets_) {
            out = fmt::format_to(out, "]");
        }
        return out;
    }
};

} // namespace fmt

#endif // FMTLIBEXT_H
