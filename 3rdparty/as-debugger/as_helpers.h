// MIT Licensed
// see https://github.com/Paril/angelscript-debugger
#ifndef AS_HELPER_H
#define AS_HELPER_H

#include <angelscript.h>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/format.h>

// simple std::expected-like
template <typename T>
struct asIDBExpected {
private:
    std::variant<std::string_view, T> data;

public:
    constexpr asIDBExpected() : data("") {}

    constexpr asIDBExpected(const std::string_view v)
        : data(std::in_place_index<0>, v) {}

    constexpr asIDBExpected(T &&v)
        : data(std::in_place_index<1>, std::move(v)) {}

    constexpr asIDBExpected(const T &v) : data(std::in_place_index<1>, v) {}

    constexpr asIDBExpected(asIDBExpected<void> &&v);

    asIDBExpected(const asIDBExpected<T> &) = default;
    asIDBExpected(asIDBExpected<T> &&) = default;
    asIDBExpected &operator=(const asIDBExpected<T> &) = default;
    asIDBExpected &operator=(asIDBExpected<T> &&) = default;

    constexpr asIDBExpected &operator=(const T &v) {
        data = v;
        return *this;
    }

    constexpr asIDBExpected &operator=(T &&v) {
        data = std::move(v);
        return *this;
    }

    constexpr bool has_value() const { return data.index() == 1; }
    constexpr explicit operator bool() const { return has_value(); }

    constexpr const std::string_view &error() const {
        return std::get<0>(data);
    }
    constexpr const T &value() const { return std::get<1>(data); }
    constexpr T &value() { return std::get<1>(data); }
};

template <>
struct asIDBExpected<void> {
private:
    std::string_view err;

public:
    constexpr asIDBExpected() : err("unknown error") {}

    constexpr asIDBExpected(const std::string_view v) : err(v) {}

    constexpr const std::string_view &error() const { return err; }
};

template <typename T>
constexpr asIDBExpected<T>::asIDBExpected(asIDBExpected<void> &&v)
    : data(std::in_place_index<0>, v.error()) {}

template <class E>
asIDBExpected(E) -> asIDBExpected<void>;

// helper class that is similar to an any,
// storing a value of any type returned by AS
// and managing the ref count.
struct asIDBValue {
public:
    asIScriptEngine *engine = nullptr;
    int typeId = 0;
    asITypeInfo *type = nullptr;

    union {
        asBYTE u8;
        asWORD u16;
        asDWORD u32;
        asQWORD u64;
        float flt;
        double dbl;
        void *obj;
    } value{};

    asIDBValue() = default;
    asIDBValue(asIScriptEngine *engine, void *ptr, int typeId,
               bool reference = false);
    asIDBValue(const asIDBValue &other);
    asIDBValue(asIDBValue &&other) noexcept;

    asIDBValue &operator=(const asIDBValue &other);
    asIDBValue &operator=(asIDBValue &&other) noexcept;

    ~asIDBValue();
    void Release();

    bool IsValid() const { return typeId != 0; }

    template <typename T>
    T *GetPointer(bool as_reference = false) const {
        if (typeId == 0)
            return nullptr;
        else if (typeId & asTYPEID_MASK_OBJECT) {
            if ((typeId & asTYPEID_OBJHANDLE) && as_reference)
                return reinterpret_cast<T *>(const_cast<void **>(&value.obj));
            return reinterpret_cast<T *>(value.obj);
        }
        return reinterpret_cast<T *>(const_cast<asQWORD *>(&value.u64));
    }

    size_t GetSize() const {
        if (typeId == 0)
            return 0;
        else if (type)
            return type->GetSize();
        else
            return engine->GetSizeOfPrimitiveType(typeId);
    }

    void SetArgument(asIScriptContext *ctx, asUINT index) const;
};

// helper class to deal with foreach iteration.
class asIDBObjectIteratorHelper {
public:
    asITypeInfo *type;
    void *obj;
    asIScriptFunction *opForBegin, *opForEnd, *opForNext;
    std::vector<asIScriptFunction *> opForValues;

    asITypeInfo *iteratorType = nullptr;
    int iteratorTypeId = 0;

    std::string_view error;

    asIDBObjectIteratorHelper(asITypeInfo *type, void *obj);

    constexpr bool IsValid() const { return opForBegin != nullptr; }
    constexpr explicit operator bool() const { return IsValid(); }

    // individual access
    asIDBValue Begin(asIScriptContext *ctx) const;
    void Value(asIScriptContext *ctx, const asIDBValue &val,
               size_t index) const;
    asIDBValue Next(asIScriptContext *ctx, const asIDBValue &val) const;
    bool End(asIScriptContext *ctx, const asIDBValue &val) const;

    // O(n) helper for length
    size_t CalculateLength(asIScriptContext *ctx) const;

private:
    bool Validate();
};

/* -*- mode: c; c-file-style: "k&r" -*-

  strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
  Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
template <bool case_sensitive = true>
struct asIDBNatCmp {
    static constexpr bool nat_isdigit(char a) { return a >= '0' && a <= '9'; }

    static constexpr bool nat_isspace(char a) {
        return a == ' ' || a == '\n' || a == '\r' || a == '\t';
    }

    static constexpr char nat_toupper(char a) {
        if (a >= 'a' && a <= 'z')
            return a - ('a' - 'A');
        return a;
    }

    static constexpr int compare_right(const char *a, const char *b) {
        int bias = 0;

        /* The longest run of digits wins.  That aside, the greatest
       value wins, but we can't know that it will until we've scanned
       both numbers to know that they have the same magnitude, so we
       remember it in BIAS. */
        for (;; a++, b++) {
            if (!nat_isdigit(*a) && !nat_isdigit(*b))
                return bias;
            else if (!nat_isdigit(*a))
                return -1;
            else if (!nat_isdigit(*b))
                return +1;
            else if (*a < *b) {
                if (!bias)
                    bias = -1;
            } else if (*a > *b) {
                if (!bias)
                    bias = +1;
            } else if (!*a && !*b)
                return bias;
        }

        return 0;
    }

    static constexpr int compare_left(const char *a, const char *b) {
        /* Compare two left-aligned numbers: the first to have a
           different value wins. */
        for (;; a++, b++) {
            if (!nat_isdigit(*a) && !nat_isdigit(*b))
                return 0;
            else if (!nat_isdigit(*a))
                return -1;
            else if (!nat_isdigit(*b))
                return +1;
            else if (*a < *b)
                return -1;
            else if (*a > *b)
                return +1;
        }

        return 0;
    }

    constexpr inline int operator()(const char *a, const char *b) const {
        int ai = 0, bi = 0;
        char ca = 0, cb = 0;
        int fractional = 0, result = 0;

        while (1) {
            ca = a[ai];
            cb = b[bi];

            /* skip over leading spaces or zeros */
            while (nat_isspace(ca))
                ca = a[++ai];

            while (nat_isspace(cb))
                cb = b[++bi];

            /* process run of digits */
            if (nat_isdigit(ca) && nat_isdigit(cb)) {
                fractional = (ca == '0' || cb == '0');

                if (fractional) {
                    if ((result = compare_left(a + ai, b + bi)) != 0)
                        return result;
                } else {
                    if ((result = compare_right(a + ai, b + bi)) != 0)
                        return result;
                }
            }

            if (!ca && !cb) {
                /* The strings compare the same.  Perhaps the caller
                       will want to call strcmp to break the tie. */
                return 0;
            }

            if constexpr (!case_sensitive) {
                ca = nat_toupper(ca);
                cb = nat_toupper(cb);
            }

            if (ca < cb)
                return -1;
            else if (ca > cb)
                return +1;

            ++ai;
            ++bi;
        }
    }

    constexpr inline int operator()(const std::string_view &a,
                                    const std::string_view &b) const {
        return asIDBNatCmp<case_sensitive>()(a.data(), b.data()) < 0;
    }

    constexpr inline int operator()(const std::string &a,
                                    const std::string &b) const {
        return asIDBNatCmp<case_sensitive>()(a.c_str(), b.c_str()) < 0;
    }
};

using asIDBNatICmp = asIDBNatCmp<false>;

template <bool case_sensitive = true>
struct asIDBNatLess {
    constexpr inline bool operator()(const char *a, const char *b) const {
        return asIDBNatCmp<case_sensitive>()(a, b) < 0;
    }

    constexpr inline bool operator()(const std::string_view &a,
                                     const std::string_view &b) const {
        return asIDBNatCmp<case_sensitive>()(a.data(), b.data()) < 0;
    }

    constexpr inline bool operator()(const std::string &a,
                                     const std::string &b) const {
        return asIDBNatCmp<case_sensitive>()(a.c_str(), b.c_str()) < 0;
    }
};

using asIDBNatILess = asIDBNatLess<false>;

/*
MIT License

Copyright (c) 2019 Tobias Locker

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

https://github.com/tobiaslocker/base64
*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace base64 {

namespace detail {

inline constexpr char padding_char{'='};

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ||     \
    (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) ||                 \
    (defined(_BYTE_ORDER) && _BYTE_ORDER == _BIG_ENDIAN) ||                    \
    (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) ||                       \
    (defined(__sun) && defined(__SVR4) && defined(_BIG_ENDIAN)) ||             \
    defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) ||    \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__) ||            \
    defined(_M_PPC)
#define __BIG_ENDIAN__
#elif (defined(__BYTE_ORDER__) &&                                              \
       __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || /* gcc */                 \
    (defined(__BYTE_ORDER) &&                                                  \
     __BYTE_ORDER == __LITTLE_ENDIAN) /* linux header */                       \
    || (defined(_BYTE_ORDER) && _BYTE_ORDER == _LITTLE_ENDIAN) ||              \
    (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) /* mingw header */ || \
    (defined(__sun) && defined(__SVR4) &&                                      \
     defined(_LITTLE_ENDIAN)) || /* solaris */                                 \
    defined(__ARMEL__) ||                                                      \
    defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL) ||      \
    defined(__MIPSEL) || defined(__MIPSEL__) || defined(_M_IX86) ||            \
    defined(_M_X64) || defined(_M_IA64) || /* msvc for intel processors */     \
    defined(_M_ARM) /* msvc code on arm executes in little endian mode */
#define __LITTLE_ENDIAN__
#endif
#endif

#if !defined(__LITTLE_ENDIAN__) & !defined(__BIG_ENDIAN__)
#error "UNKNOWN Platform / endianness. Configure endianness explicitly."
#endif

std::array<char, 256> constexpr encode_table_0 = {
    'A', 'A', 'A', 'A', 'B', 'B', 'B', 'B', 'C', 'C', 'C', 'C', 'D', 'D', 'D',
    'D', 'E', 'E', 'E', 'E', 'F', 'F', 'F', 'F', 'G', 'G', 'G', 'G', 'H', 'H',
    'H', 'H', 'I', 'I', 'I', 'I', 'J', 'J', 'J', 'J', 'K', 'K', 'K', 'K', 'L',
    'L', 'L', 'L', 'M', 'M', 'M', 'M', 'N', 'N', 'N', 'N', 'O', 'O', 'O', 'O',
    'P', 'P', 'P', 'P', 'Q', 'Q', 'Q', 'Q', 'R', 'R', 'R', 'R', 'S', 'S', 'S',
    'S', 'T', 'T', 'T', 'T', 'U', 'U', 'U', 'U', 'V', 'V', 'V', 'V', 'W', 'W',
    'W', 'W', 'X', 'X', 'X', 'X', 'Y', 'Y', 'Y', 'Y', 'Z', 'Z', 'Z', 'Z', 'a',
    'a', 'a', 'a', 'b', 'b', 'b', 'b', 'c', 'c', 'c', 'c', 'd', 'd', 'd', 'd',
    'e', 'e', 'e', 'e', 'f', 'f', 'f', 'f', 'g', 'g', 'g', 'g', 'h', 'h', 'h',
    'h', 'i', 'i', 'i', 'i', 'j', 'j', 'j', 'j', 'k', 'k', 'k', 'k', 'l', 'l',
    'l', 'l', 'm', 'm', 'm', 'm', 'n', 'n', 'n', 'n', 'o', 'o', 'o', 'o', 'p',
    'p', 'p', 'p', 'q', 'q', 'q', 'q', 'r', 'r', 'r', 'r', 's', 's', 's', 's',
    't', 't', 't', 't', 'u', 'u', 'u', 'u', 'v', 'v', 'v', 'v', 'w', 'w', 'w',
    'w', 'x', 'x', 'x', 'x', 'y', 'y', 'y', 'y', 'z', 'z', 'z', 'z', '0', '0',
    '0', '0', '1', '1', '1', '1', '2', '2', '2', '2', '3', '3', '3', '3', '4',
    '4', '4', '4', '5', '5', '5', '5', '6', '6', '6', '6', '7', '7', '7', '7',
    '8', '8', '8', '8', '9', '9', '9', '9', '+', '+', '+', '+', '/', '/', '/',
    '/'};

std::array<char, 256> constexpr encode_table_1 = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', '+', '/', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k',
    'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', 'A', 'B', 'C',
    'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
    'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+',
    '/'};

} // namespace detail

template <class OutputBuffer, class InputIterator>
inline OutputBuffer encode_into(InputIterator begin, InputIterator end) {
    typedef std::decay_t<decltype(*begin)> input_value_type;
    static_assert(std::is_same_v<input_value_type, char> ||
                  std::is_same_v<input_value_type, signed char> ||
                  std::is_same_v<input_value_type, unsigned char> ||
                  std::is_same_v<input_value_type, std::byte>);
    typedef typename OutputBuffer::value_type output_value_type;
    static_assert(std::is_same_v<output_value_type, char> ||
                  std::is_same_v<output_value_type, signed char> ||
                  std::is_same_v<output_value_type, unsigned char> ||
                  std::is_same_v<output_value_type, std::byte>);
    const size_t binarytextsize = end - begin;
    const size_t encodedsize = (binarytextsize / 3 + (binarytextsize % 3 > 0))
                               << 2;
    OutputBuffer encoded(encodedsize, detail::padding_char);

    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&*begin);
    char *currEncoding = reinterpret_cast<char *>(&encoded[0]);

    for (size_t i = binarytextsize / 3; i; --i) {
        const uint8_t t1 = *bytes++;
        const uint8_t t2 = *bytes++;
        const uint8_t t3 = *bytes++;
        *currEncoding++ = detail::encode_table_0[t1];
        *currEncoding++ =
            detail::encode_table_1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *currEncoding++ =
            detail::encode_table_1[((t2 & 0x0F) << 2) | ((t3 >> 6) & 0x03)];
        *currEncoding++ = detail::encode_table_1[t3];
    }

    switch (binarytextsize % 3) {
    case 0: {
        break;
    }
    case 1: {
        const uint8_t t1 = bytes[0];
        *currEncoding++ = detail::encode_table_0[t1];
        *currEncoding++ = detail::encode_table_1[(t1 & 0x03) << 4];
        // *currEncoding++ = detail::padding_char;
        // *currEncoding++ = detail::padding_char;
        break;
    }
    case 2: {
        const uint8_t t1 = bytes[0];
        const uint8_t t2 = bytes[1];
        *currEncoding++ = detail::encode_table_0[t1];
        *currEncoding++ =
            detail::encode_table_1[((t1 & 0x03) << 4) | ((t2 >> 4) & 0x0F)];
        *currEncoding++ = detail::encode_table_1[(t2 & 0x0F) << 2];
        // *currEncoding++ = detail::padding_char;
        break;
    }
    default: {
        throw std::runtime_error{"Invalid base64 encoded data"};
    }
    }

    return encoded;
}

template <class OutputBuffer>
inline OutputBuffer encode_into(std::string_view data) {
    return encode_into<OutputBuffer>(std::begin(data), std::end(data));
}

inline std::string to_base64(std::string_view data) {
    return encode_into<std::string>(std::begin(data), std::end(data));
}
} // namespace base64

#endif
