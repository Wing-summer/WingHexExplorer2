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
#include "class/scriptsettings.h"
#include "scriptaddon/scriptdatetime.h"
#include "scriptaddon/scriptqdictionary.h"

#include <QFont>
#include <QFontMetrics>
#include <QString>
#include <string>

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

template <typename T>
class CScriptObjectView {
public:
    inline explicit CScriptObjectView(const T *data) noexcept : data_(data) {
        Q_ASSERT(data);
    }

    inline ~CScriptObjectView() noexcept { data_ = nullptr; }

    inline const T *data() const noexcept { return data_; }

private:
    const T *data_ = nullptr;
};

using CScriptArrayView = CScriptObjectView<CScriptArray>;
using CScriptDictionaryView = CScriptObjectView<CScriptDictionary>;

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
format_element_with_spec(const void *elemPtr, int typeId,
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
        store.push_back(*static_cast<const bool *>(elemPtr));
        break;
    case asTYPEID_INT8:
        store.push_back(*static_cast<const qint8 *>(elemPtr));
        break;
    case asTYPEID_INT16:
        store.push_back(*static_cast<const qint16 *>(elemPtr));
        break;
    case asTYPEID_INT32:
        store.push_back(*static_cast<const qint32 *>(elemPtr));
        break;
    case asTYPEID_INT64:
        store.push_back(*static_cast<const qint64 *>(elemPtr));
        break;
    case asTYPEID_UINT8:
        store.push_back(*static_cast<const quint8 *>(elemPtr));
        break;
    case asTYPEID_UINT16:
        store.push_back(*static_cast<const quint16 *>(elemPtr));
        break;
    case asTYPEID_UINT32:
        store.push_back(*static_cast<const quint32 *>(elemPtr));
        break;
    case asTYPEID_UINT64:
        store.push_back(*static_cast<const quint64 *>(elemPtr));
        break;
    case asTYPEID_FLOAT:
        store.push_back(*static_cast<const float *>(elemPtr));
        break;
    case asTYPEID_DOUBLE:
        store.push_back(*static_cast<const double *>(elemPtr));
        break;
    default: {
        if (m.isAngelChar(typeId)) {
            store.push_back(
                escapeNonPrintable(*static_cast<const QChar *>(elemPtr))
                    .prepend('\'')
                    .append('\''));
        } else if (m.isAngelString(typeId)) {
            auto str =
                escapeNonPrintable(*static_cast<const QString *>(elemPtr));
            store.push_back(str.prepend('"').append('"'));
        } else if (m.isAngelArray(typeId)) {
            store.push_back(
                CScriptArrayView(static_cast<const CScriptArray *>(elemPtr)));
        } else if (m.isAngelDictionary(typeId)) {
            store.push_back(CScriptDictionaryView(
                static_cast<const CScriptDictionary *>(elemPtr)));
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

    // parse: collect entire inner-spec up to '}', then extract flags anywhere
    // and leave the rest as underlying_spec_
    auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        // find end of this format spec (position of closing '}', or end)
        auto spec_end = it;
        while (spec_end != end && *spec_end != '}')
            ++spec_end;

        std::string spec(it, spec_end); // full spec substring

        // find and remove "?s" first
        auto pos = spec.find("?s");
        if (pos != std::string::npos) {
            range_type_ = AS_DEBUG_STRING;
            spec.erase(pos, 2);
        } else {
            // find 's'
            pos = spec.find('s');
            if (pos != std::string::npos) {
                range_type_ = AS_STRING;
                spec.erase(pos, 1);
            }
        }

        // find 'n'
        pos = spec.find('n');
        if (pos != std::string::npos) {
            no_brackets_ = true;
            spec.erase(pos, 1);
        }

        // If there's a leading ':' leftover (some users might write ":x"),
        // strip it.
        if (!spec.empty() && spec.front() == ':')
            spec.erase(0, 1);

        // Remaining spec is the underlying_spec (may be empty)
        underlying_spec_ = spec;

        // return iterator to the closing brace (fmt expects iterator pointing
        // to the end of what we parsed)
        return spec_end;
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
            // pass through underlying_spec_ to element formatting
            out = format_element_with_spec(const_cast<void *>(elemPtr), typeId,
                                           underlying_spec_, ctx);
        }

        if (!no_brackets_) {
            out = fmt::format_to(out, "]");
        }
        return out;
    }
};

inline static thread_local int g_dict_pretty_level = 0;
inline static constexpr int g_dict_indent_width = 4;

template <>
struct formatter<CScriptDictionaryView> {
    bool no_brackets_ = false;
    bool pretty_align_ = false;
    std::string underlying_spec_;

    auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        auto end = ctx.end();

        auto spec_end = it;
        while (spec_end != end && *spec_end != '}')
            ++spec_end;

        std::string spec(it, spec_end);

        // detect 'h' -> pretty align
        auto pos = spec.find('h');
        if (pos != std::string::npos) {
            pretty_align_ = true;
            spec.erase(pos, 1);
        }

        // detect 'n' -> remove outer braces
        pos = spec.find('n');
        if (pos != std::string::npos) {
            no_brackets_ = true;
            spec.erase(pos, 1);
        }

        // strip leading ':' if user wrote something like ":x"
        if (!spec.empty() && spec.front() == ':')
            spec.erase(0, 1);

        underlying_spec_ = spec;
        return spec_end;
    }

    template <typename FormatContext>
    auto format(const CScriptDictionaryView &dicv, FormatContext &ctx) const
        -> decltype(ctx.out()) {
        auto out = ctx.out();
        auto dict = dicv.data();
        auto &m = ScriptMachine::instance();

        // collect entries using your iterator API
        struct E {
            std::string key_disp; // formatted key display like ["..."]
            void *val_ptr;
            int val_type;
        };
        std::vector<E> entries;
        entries.reserve(dict->GetSize());

        for (auto it = dict->begin(); it != dict->end(); ++it) {
            auto key = it.GetKey();
            void *vptr = const_cast<void *>(it.GetAddressOfValue());
            int vtype = it.GetTypeId();

            std::string key_disp = fmt::format(R"(["{}"])", key);
            entries.push_back({std::move(key_disp), vptr, vtype});
        }

        // --- pretty aligned multi-line printing ---
        if (pretty_align_) {
            ++g_dict_pretty_level;
            const int indent = g_dict_pretty_level * g_dict_indent_width;
            const int item_indent = indent;

            auto &set = ScriptSettings::instance();
            auto dfont = QFont(set.consoleFontFamily());
            dfont.setPointSize(set.consoleFontSize());
            auto metrics = QFontMetrics(dfont);
            auto spacew = metrics.horizontalAdvance(' ');

            std::vector<int> key_widths;
            key_widths.reserve(entries.size());
            int max_key_width = 0;
            for (auto &e : entries) {
                int w = metrics.horizontalAdvance(
                            QString::fromStdString(e.key_disp)) /
                        spacew;
                key_widths.push_back(w);
                if (w > max_key_width)
                    max_key_width = w;
            }

            if (!no_brackets_) {
                out = fmt::format_to(out, "{{\n");
            }

            for (size_t i = 0; i < entries.size(); ++i) {
                const auto &e = entries[i];
                int kw = key_widths[i];

                out = fmt::format_to(out, "{}", std::string(item_indent, ' '));
                out = fmt::format_to(out, "{}", e.key_disp);

                int pad = max_key_width - kw;
                if (pad > 0) {
                    out = fmt::format_to(out, "{}", std::string(pad, ' '));
                }

                out = fmt::format_to(out, " = ");

                if (m.isAngelDictionary(e.val_type)) {
                    std::string child_spec;
                    if (!underlying_spec_.empty()) {
                        child_spec = "{:" + underlying_spec_ + "h}";
                    } else {
                        child_spec = "{:h}";
                    }
                    auto childView = CScriptDictionaryView(
                        static_cast<CScriptDictionary *>(e.val_ptr));
                    out = fmt::format_to(out, child_spec, childView);
                } else {
                    out = format_element_with_spec(e.val_ptr, e.val_type,
                                                   underlying_spec_, ctx);
                }

                if (i + 1 < entries.size())
                    out = fmt::format_to(out, "\n");
            }

            if (!no_brackets_) {
                out = fmt::format_to(
                    out, "\n{}}}",
                    std::string((g_dict_pretty_level - 1) * g_dict_indent_width,
                                ' '));
            }

            --g_dict_pretty_level;
        } else {
            if (!no_brackets_)
                out = fmt::format_to(out, "{{");

            for (size_t i = 0; i < entries.size(); ++i) {
                const auto &e = entries[i];
                if (i)
                    out = fmt::format_to(out, ", ");

                out = fmt::format_to(out, "{}", e.key_disp);
                out = fmt::format_to(out, " = ");
                out = format_element_with_spec(e.val_ptr, e.val_type,
                                               underlying_spec_, ctx);
            }

            if (!no_brackets_)
                out = fmt::format_to(out, "}}");
        }

        return out;
    }
};

} // namespace fmt

#endif // FMTLIBEXT_H
