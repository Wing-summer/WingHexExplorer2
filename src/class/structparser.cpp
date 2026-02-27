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

#include "structparser.h"

#include "class/wingangelapi.h"
#include "class/wingcstruct.h"
#include "define.h"

StructParser::StructParser(WingCStruct *api) : _api(api) {
    _parser = new CTypeParser([this](const MsgInfo &info) {
        QString msg = QStringLiteral("[%1, %2] ")
                          .arg(info.line)
                          .arg(info.charPositionInLine) +
                      info.info;
        switch (info.type) {
        case MsgType::Error:
            _errors.append(msg);
            break;
        case MsgType::Warn:
            _warns.append(msg);
            break;
        }
    });
}

StructParser::~StructParser() { delete _parser; }

bool StructParser::parseFromSource(const QString &header) {
    _errors.clear();
    _warns.clear();
    return _parser->parseFromSource(header.toUtf8());
}

bool StructParser::parse(const QString &fileName) {
    _errors.clear();
    _warns.clear();
    return _parser->parse(fileName);
}

void StructParser::reset() {
    _errors.clear();
    _warns.clear();
    _parser->clear();
}

bool StructParser::isLittleEndian() const { return m_islittle; }

void StructParser::setIsLittleEndian(bool newIslittle) {
    m_islittle = newIslittle;
}

bool StructParser::setPadAlignment(int padding) {
    return _parser->setPadAlignment(padding);
}

int StructParser::padAlignment() const { return _parser->padAlignment(); }

QStringList StructParser::structTypeDefs() { return _parser->structTypeDefs(); }

QStringList StructParser::unionTypeDefs() { return _parser->unionTypeDefs(); }

QStringList StructParser::typedefTypeDefs() {
    return _parser->typedefTypeDefs();
}

QStringList StructParser::enumTypeDefs() { return _parser->enumTypeDefs(); }

QStringList StructParser::constVarDefs() { return _parser->constVarDefs(); }

qint64 StructParser::sizeOf(const QString &type) {
    auto r = _parser->getTypeSize(type);
    if (r) {
        return r.value();
    }
    return 0;
}

bool StructParser::containsType(const QString &name) {
    return _parser->containsType(name);
}

bool StructParser::isBasicType(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::BasicType;
}

bool StructParser::isUnsignedBasicType(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->isUnsignedBasicType(rname);
}

bool StructParser::containsEnum(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Enum;
}

bool StructParser::containsStruct(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Struct;
}

bool StructParser::containsUnion(const QString &name) {
    return _parser->resolveType(name) == CTypeParser::CType::Union;
}

bool StructParser::containsTypeDef(const QString &name) {
    return _parser->containsTypeDef(name);
}

bool StructParser::containsConstVar(const QString &name) {
    return _parser->containsConstVar(name);
}

bool StructParser::isCompletedType(const QString &name) {
    return _parser->isCompletedType(name);
}

QStringList StructParser::enumValueNames(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->enumMembers(rname);
}

qint64 StructParser::constVarValueInt(const QString &name, bool *ok) {
    auto v = _parser->constVarValue(name);
    if (std::holds_alternative<qint64>(v)) {
        if (ok) {
            *ok = true;
        }
        return std::get<qint64>(v);
    } else {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
}

quint64 StructParser::constVarValueUInt(const QString &name, bool *ok) {
    auto v = _parser->constVarValue(name);
    if (std::holds_alternative<quint64>(v)) {
        if (ok) {
            *ok = true;
        }
        return std::get<quint64>(v);
    } else {
        if (ok) {
            *ok = false;
        }
        return 0;
    }
}

CINT_TYPE StructParser::constVarValue(const QString &name) const {
    return _parser->constVarValue(name);
}

bool StructParser::isCompletedStruct(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->isCompletedStruct(rname);
}

bool StructParser::isCompletedUnion(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->isCompletedUnion(rname);
}

QStringList StructParser::getMissingDependencise(const QString &name) {
    auto rname = _parser->resolveTypeName(name);
    return _parser->getMissingDependencise(rname);
}

QString StructParser::dumpAllTypes() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpAllTypes(buffer);
    return ret;
}

QString StructParser::dumpTypeDefines() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpTypeDefines(buffer);
    return ret;
}

QString StructParser::dumpConstants() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpConstants(buffer);
    return ret;
}

QString StructParser::dumpStructs() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpStructs(buffer);
    return ret;
}

QString StructParser::dumpUnions() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpUnions(buffer);
    return ret;
}

QString StructParser::dumpEnums() const {
    QString ret;
    QTextStream buffer(&ret);
    _parser->dumpEnums(buffer);
    return ret;
}

QStringList StructParser::getParsedErrors() const { return _errors; }

QStringList StructParser::getParsedWarns() const { return _warns; }

QStringList StructParser::structOrUnionMemberNames(const QString &type) const {
    QStringList buffer;
    auto t = _parser->resolveTypeName(type);
    if (_parser->containsStruct(t)) {
        auto r = _parser->structMembers(t);
        for (auto &m : r) {
            buffer.append(m.var_name);
        }
    } else if (_parser->containsUnion(t)) {
        auto r = _parser->unionMembers(t);
        for (auto &m : r) {
            buffer.append(m.var_name);
        }
    }

    return buffer;
}

QStringList
StructParser::structOrUnionMemberDataTypes(const QString &type) const {
    QStringList buffer;
    auto t = _parser->resolveTypeName(type);
    if (_parser->containsStruct(t)) {
        auto r = _parser->structMembers(t);
        for (auto &m : r) {
            buffer.append(m.data_type);
        }
    } else if (_parser->containsUnion(t)) {
        auto r = _parser->unionMembers(t);
        for (auto &m : r) {
            buffer.append(m.data_type);
        }
    }
    return buffer;
}

QStringList StructParser::structOrUnionMemberDecls(const QString &type) const {
    QStringList buffer;
    static QString padding(4, ' ');
    auto t = _parser->resolveTypeName(type);

    auto op = [](const VariableDeclaration &m) -> QString {
        QString buffer;
        QTextStream output(&buffer);
        output << m.data_type;

        if (m.is_pointer)
            output << QStringLiteral("* ");

        output << padding;
        if (m.var_name.isEmpty()) {
            output << '?';
        } else {
            output << m.var_name;
        }

        for (auto &dim : m.array_dims) {
            output << '[' << dim << ']';
        }

        if (m.bit_size) {
            output << QStringLiteral(" : ") << m.bit_size;
        }
        return buffer;
    };

    if (_parser->containsStruct(t)) {
        auto r = _parser->structMembers(t);
        for (auto &m : r) {
            buffer.append(op(m));
        }
    } else if (_parser->containsUnion(t)) {
        auto r = _parser->unionMembers(t);
        for (auto &m : r) {
            buffer.append(op(m));
        }
    }
    return buffer;
}

QStringList
StructParser::structOrUnionMemberDeclWithoutNames(const QString &type) const {
    QStringList buffer;
    static QString padding(4, ' ');
    auto t = _parser->resolveTypeName(type);

    auto op = [](const VariableDeclaration &m) -> QString {
        QString buffer;
        QTextStream output(&buffer);
        output << m.data_type;

        if (m.is_pointer)
            output << QStringLiteral("* ");

        output << padding;

        for (auto &dim : m.array_dims) {
            output << '[' << dim << ']';
        }

        if (m.bit_size) {
            output << QStringLiteral(" : ") << m.bit_size;
        }
        return buffer;
    };

    if (_parser->containsStruct(t)) {
        auto r = _parser->structMembers(t);
        for (auto &m : r) {
            buffer.append(op(m));
        }
    } else if (_parser->containsUnion(t)) {
        auto r = _parser->unionMembers(t);
        for (auto &m : r) {
            buffer.append(op(m));
        }
    }
    return buffer;
}

QString StructParser::resolveTypeName(const QString &name) const {
    return _parser->resolveTypeName(name);
}

quint64 StructParser::getShiftAndMaskedPtr(const QByteArray &buffer) {
    if (buffer.size() == sizeof(quint32)) {
        return Utilities::processEndian(
            *reinterpret_cast<const quint32 *>(buffer.data()), m_islittle);
    } else {
        return Utilities::processEndian(
            *reinterpret_cast<const quint64 *>(buffer.data()), m_islittle);
    }
}

QVariant StructParser::getData(const char *ptr, const char *end,
                               QMetaType::Type type, qint64 shift, qint64 mask,
                               qsizetype size) {
    if (ptr + size > end) {
        return {};
    }

    QByteArray buffer(ptr, size);
    switch (type) {
    case QMetaType::Bool:
        return QVariant::fromValue(
            getShiftAndMasked<bool>(buffer, shift, mask));
    case QMetaType::Int:
        return QVariant::fromValue(getShiftAndMasked<int>(buffer, shift, mask));
    case QMetaType::UInt:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned int>(buffer, shift, mask));
    case QMetaType::LongLong:
        return QVariant::fromValue(
            getShiftAndMasked<long long>(buffer, shift, mask));
    case QMetaType::ULongLong:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned long long>(buffer, shift, mask));
    case QMetaType::Double:
        return QVariant::fromValue(
            getShiftAndMasked<double>(buffer, shift, mask));
    case QMetaType::Long:
        return QVariant::fromValue(
            getShiftAndMasked<long>(buffer, shift, mask));
    case QMetaType::Short:
        return QVariant::fromValue(
            getShiftAndMasked<short>(buffer, shift, mask));
    case QMetaType::Char:
        return QVariant::fromValue(
            getShiftAndMasked<char>(buffer, shift, mask));
    case QMetaType::Char16:
        return QVariant::fromValue(
            getShiftAndMasked<char16_t>(buffer, shift, mask));
    case QMetaType::Char32:
        return QVariant::fromValue(
            getShiftAndMasked<char32_t>(buffer, shift, mask));
    case QMetaType::ULong:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned long>(buffer, shift, mask));
    case QMetaType::UShort:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned short>(buffer, shift, mask));
    case QMetaType::UChar:
        return QVariant::fromValue(
            getShiftAndMasked<unsigned char>(buffer, shift, mask));
    case QMetaType::Float:
        return QVariant::fromValue(
            getShiftAndMasked<float>(buffer, shift, mask));
    case QMetaType::SChar:
        return QVariant::fromValue(
            getShiftAndMasked<signed char>(buffer, shift, mask));
    case QMetaType::VoidStar:
        return QVariant::fromValue(getShiftAndMaskedPtr(buffer));
    default:
        return {};
    }
}

QVariantHash StructParser::__read(qsizetype offset, const QString &type,
                                  bool efmtInVariantList) {
    if (!_api->isCurrentDocEditing()) {
        return {};
    }

    auto rtype = resolveTypeName(type);
    auto len = sizeOf(rtype);
    if (len <= 0) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto doclen = _api->documentBytes();
    if (doclen < 0 || offset + len > doclen) {
        return {};
    }

    // first read all bytes
    auto raw = _api->readBytes(offset, len);

    // then slice and parse
    const auto *pdata = raw.data();
    const auto *pend = pdata + raw.length();

    return readStruct(pdata, pend, rtype, efmtInVariantList);
}

QVariant StructParser::__readMember(qsizetype offset, const QString &type,
                                    const QString &member,
                                    bool efmtInVariantList) {
    if (!_api->isCurrentDocEditing()) {
        return {};
    }

    auto rtype = resolveTypeName(type);
    auto len = sizeOf(rtype);
    if (len <= 0) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto doclen = _api->documentBytes();
    if (doclen < 0 || offset + len > doclen) {
        return {};
    }

    // first read all bytes
    auto raw = _api->readBytes(offset, len);

    // then slice and parse
    const auto *pdata = raw.data();
    const auto *pend = pdata + raw.length();

    return readStructMember(pdata, pend, rtype, member, efmtInVariantList);
}

QVariantHash
StructParser::__readMembers(qsizetype offset, const QString &type,
                            const std::function<QString(uint)> &members,
                            uint total, bool efmtInVariantList) {
    if (!_api->isCurrentDocEditing()) {
        return {};
    }

    auto rtype = resolveTypeName(type);
    auto len = sizeOf(rtype);

    if (offset < 0) {
        return {};
    }

    auto doclen = _api->documentBytes();
    if (doclen < 0 || offset + len > doclen) {
        return {};
    }

    // first read all bytes
    auto raw = _api->readBytes(offset, len);

    // then slice and parse
    const auto *pdata = raw.data();
    const auto *pend = pdata + raw.length();

    return readStructMembers(pdata, pend, rtype, members, total,
                             efmtInVariantList);
}

QByteArray StructParser::__readRaw(qsizetype offset, const QString &type) {
    if (!_api->isCurrentDocEditing()) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto rtype = resolveTypeName(type);
    auto len = sizeOf(rtype);
    if (len <= 0) {
        return {};
    }

    if (!_api->isCurrentDocEditing()) {
        return {};
    }

    if (offset < 0) {
        return {};
    }

    auto doclen = _api->documentBytes();
    if (doclen < 0 || offset + len > doclen) {
        return {};
    }

    return _api->readBytes(offset, len);
}

QVariantHash StructParser::readStruct(const char *ptr, const char *end,
                                      const QString &type,
                                      bool efmtInVariantList) {
    if (!_parser->isCompletedStruct(type)) {
        return {};
    }
    auto struc = _parser->structMembers(type);
    QVariantHash content;
    // then slice and parse
    for (auto &m : struc) {
        content.insert(m.var_name,
                       readContent(ptr + m.offset, end, m, efmtInVariantList));
    }
    return content;
}

QVariant StructParser::readStructMember(const char *ptr, const char *end,
                                        const QString &type,
                                        const QString &member,
                                        bool efmtInVariantList) {
    if (!_parser->isCompletedStruct(type)) {
        return {};
    }
    auto struc = _parser->structMembers(type);
    auto r = std::find_if(struc.begin(), struc.end(),
                          [member](const VariableDeclaration &decl) {
                              return decl.var_name == member;
                          });
    if (r != struc.end()) {
        return readContent(ptr + r->offset, end, *r, efmtInVariantList);
    }
    return {};
}

QVariantHash
StructParser::readStructMembers(const char *ptr, const char *end,
                                const QString &type,
                                const std::function<QString(uint)> &members,
                                uint total, bool efmtInVariantList) {
    if (!_parser->isCompletedStruct(type)) {
        return {};
    }
    auto struc = _parser->structMembers(type);

    QSet<QString> mems;
    for (uint i = 0; i < total; ++i) {
        mems.insert(members(i));
    }

    QVariantHash content;

    for (auto &m : struc) {
        if (mems.contains(m.var_name)) {
            content.insert(m.var_name, readContent(ptr + m.offset, end, m,
                                                   efmtInVariantList));
        }
    }

    return content;
}

QVariant StructParser::readContent(const char *ptr, const char *end,
                                   const VariableDeclaration &m,
                                   bool efmtInVariantList) {

    bool retry = false;
    auto dt = m.data_type;
    do {
        auto t = _parser->type(dt);
        switch (t) {
        case CTypeParser::CType::Unknown:
        case CTypeParser::CType::ConstVar:
            Q_ASSERT(false);
            break;
        case CTypeParser::CType::BasicType: {
            auto meta = _parser->metaType(dt);
            if (m.element_count > 1) {
                // array with bit field is not allowed
                auto size = _parser->getTypeSize(dt);
                Q_ASSERT(size);
                auto size_v = size.value();

                QVariantList l;
                auto begin = ptr;
                for (qsizetype i = 0; i < m.element_count; ++i) {
                    auto data = getData(begin, end, meta, m.op.shift, m.op.mask,
                                        size_v);
                    if (data.isNull()) {
                        return {};
                    }
                    l.append(data);
                    begin += size_v;
                }
                return l;
            } else {
                qint64 size;
                if (m.bit_size) {
                    auto div = std::div(m.bit_size, qint64(8));
                    size = div.quot;
                    if (div.rem) {
                        size++;
                    }
                } else {
                    size = m.var_size;
                }
                auto data =
                    getData(ptr, end, meta, m.op.shift, m.op.mask, size);
                if (data.isNull()) {
                    return {};
                }

                if (!m.fmt_type.isEmpty()) {
                    // format will take effects if basic type
                    // is not double or float
                    auto typeID = data.typeId();

                    if (typeID == QMetaType::Float ||
                        typeID == QMetaType::Double ||
                        typeID == QMetaType::Float16) {
                        // warn and ignore format
                        qWarning("[StructParser::readContent] Invalid format "
                                 "type on float number");
                    }

                    QString v;

                    if (efmtInVariantList) {
                        // [0] = int_value, [1] = decoded_enum
                        QVariantList rlist;
                        bool ok;
                        auto var = data.toLongLong(&ok);
                        auto evs = _parser->enumMembers(m.fmt_type);
                        for (auto &e : evs) {
                            auto ev = _parser->constVarValue(e);
                            if (std::holds_alternative<qint64>(ev)) {
                                auto rv = std::get<qint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            } else {
                                auto rv = std::get<quint64>(ev);
                                if (rv == var) {
                                    v = e;
                                    break;
                                }
                            }
                        }

                        if (!ok) {
                            auto var = data.toULongLong();

                            auto evs = _parser->enumMembers(m.fmt_type);

                            for (auto &e : evs) {
                                auto ev = _parser->constVarValue(e);
                                if (std::holds_alternative<qint64>(ev)) {
                                    auto rv = std::get<qint64>(ev);
                                    if (rv == var) {
                                        v = e;
                                        break;
                                    }
                                } else {
                                    auto rv = std::get<quint64>(ev);
                                    if (rv == var) {
                                        v = e;
                                        break;
                                    }
                                }
                            }
                        }

                        rlist.append(data);
                        if (v.isEmpty()) {
                            rlist.append(QStringLiteral("<?>"));
                        } else {
                            rlist.append(
                                QString(m.fmt_type + QStringLiteral("::") + v));
                        }
                        data = rlist;
                    } else {
                        CEnumValue rlist;
                        bool ok;
                        auto var = data.toLongLong(&ok);
                        auto evs = _parser->enumMembers(m.fmt_type);

                        for (auto &e : evs) {
                            auto ev = _parser->constVarValue(e);
                            if (std::holds_alternative<qint64>(ev)) {
                                auto rv = std::get<qint64>(ev);
                                if (rv == var) {
                                    rlist.first = rv;
                                    v = e;
                                    break;
                                }
                            } else {
                                auto rv = std::get<quint64>(ev);
                                if (rv == var) {
                                    rlist.first = rv;
                                    v = e;
                                    break;
                                }
                            }
                        }
                        if (!ok) {
                            auto var = data.toULongLong();
                            auto evs = _parser->enumMembers(m.fmt_type);

                            for (auto &e : evs) {
                                auto ev = _parser->constVarValue(e);
                                if (std::holds_alternative<qint64>(ev)) {
                                    auto rv = std::get<qint64>(ev);
                                    if (rv == var) {
                                        rlist.first = rv;
                                        v = e;
                                        break;
                                    }
                                } else {
                                    auto rv = std::get<quint64>(ev);
                                    if (rv == var) {
                                        rlist.first = rv;
                                        v = e;
                                        break;
                                    }
                                }
                            }
                        }

                        if (v.isEmpty()) {
                            rlist.second = QStringLiteral("<?>");
                        } else {
                            rlist.second =
                                m.fmt_type + QStringLiteral("::") + v;
                        }

                        data = QVariant::fromValue(rlist);
                    }
                }
                return data;
            }
        } break;
        case CTypeParser::CType::Enum:
            // should not go there
            Q_ASSERT(false);
            break;
        case CTypeParser::CType::Struct:
            return readStruct(ptr, end, dt, efmtInVariantList);
        case CTypeParser::CType::Union: {
            auto size = _parser->getTypeSize(dt);
            Q_ASSERT(size);
            auto size_v = size.value();
            auto ptrend = ptr + size_v;
            if (ptrend <= end) {
                auto ms = _parser->unionMembers(dt);
                QVariantHash ret;
                for (auto &m : ms) {
                    auto r = readContent(ptr, ptrend, m, efmtInVariantList);
                    ret.insert(m.var_name, r);
                }
                return ret;
            } else {
                return {};
            }
        } break;
        case CTypeParser::CType::TypeDef: {
            dt = _parser->resolveTypeName(dt);
            retry = true;
        } break;
        case CTypeParser::CType::Pointer: {
            if (_parser->pointerMode() == PointerMode::X86) {
                auto data = getData(ptr, end, QMetaType::VoidStar, m.op.shift,
                                    m.op.mask, sizeof(quint32));
                return data;
            } else {
                auto data = getData(ptr, end, QMetaType::VoidStar, m.op.shift,
                                    m.op.mask, sizeof(quint64));
                return data;
            }
        } break;
        }
    } while (retry);

    return {};
}

bool StructParser::isValidCStructMetaType(QMetaType::Type type) {
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Double:
    case QMetaType::Long:
    case QMetaType::Short:
    case QMetaType::Char:
    case QMetaType::Char16:
    case QMetaType::Char32:
    case QMetaType::ULong:
    case QMetaType::UShort:
    case QMetaType::UChar:
    case QMetaType::Float:
    case QMetaType::SChar:
        return true;
    default:
        if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
            return true;
        }
        return false;
    }
}

CScriptDictionary *
StructParser::convert2AsDictionary(const QVariantHash &hash) {
    auto ctx = asGetActiveContext();
    if (ctx == nullptr) {
        return nullptr;
    }

    auto engine = ctx->GetEngine();
    Q_ASSERT(engine);

    auto dic = CScriptDictionary::Create(engine);
    for (auto p = hash.constKeyValueBegin(); p != hash.constKeyValueEnd();
         ++p) {
        auto var = p->second;
        auto type = var.isNull() ? QMetaType::Type::Void
                                 : QMetaType::Type(var.typeId());
        switch (type) {
        case QMetaType::Bool: {
            auto v = var.toBool();
            dic->Set(p->first, &v, asTYPEID_BOOL);
            break;
        }
        case QMetaType::UChar: {
            auto v = var.value<uchar>();
            dic->Set(p->first, &v, asTYPEID_UINT8);
            break;
        }
        case QMetaType::SChar: {
            auto v = var.value<uchar>();
            dic->Set(p->first, &v, asTYPEID_INT8);
            break;
        }
        case QMetaType::Short: {
            auto v = var.value<short>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            dic->Set(p->first, &v, asTYPEID_INT16);
            break;
        }
        case QMetaType::UShort: {
            auto v = var.value<ushort>();
            static_assert(sizeof(short) == 2, "sizeof(short) != 2");
            dic->Set(p->first, &v, asTYPEID_UINT16);
            break;
        }
        case QMetaType::Int:
        case QMetaType::Long: {
            auto v = var.toInt();
            dic->Set(p->first, &v, asTYPEID_INT32);
        }
        case QMetaType::LongLong: {
            auto v = var.toLongLong();
            dic->Set(p->first, &v, asTYPEID_INT64);
            break;
        }
        case QMetaType::UInt:
        case QMetaType::ULong: {
            auto v = var.toUInt();
            dic->Set(p->first, &v, asTYPEID_UINT32);
            break;
        }
        case QMetaType::ULongLong: {
            auto v = var.toULongLong();
            dic->Set(p->first, asQWORD(v));
            break;
        }
        case QMetaType::Double:
            dic->Set(p->first, var.toDouble());
            break;
        case QMetaType::Float: {
            auto v = var.toFloat();
            dic->Set(p->first, &v, asTYPEID_FLOAT);
            break;
        }
        case QMetaType::Char: {
            auto v = var.value<char>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::Char16: {
            auto v = var.value<char16_t>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::Char32: {
            auto v = var.value<char32_t>();
            QChar ch(v);
            auto type = static_cast<asITypeInfo *>(
                engine->GetUserData(AsUserDataType::UserData_CharTypeInfo));
            auto id = type->GetTypeId();
            dic->Set(p->first, &ch, id);
            break;
        }
        case QMetaType::QVariantList: {
            // note: empty list is not allowed!
            // If empty, it will be ignored
            auto v = var.toList();
            if (!v.isEmpty()) {
                // reguard the first element type is the specilization
                auto var = v.first();
                auto type = var.isNull() ? QMetaType::Type::Void
                                         : QMetaType::Type(var.typeId());
                if (type == QMetaType::Type::Void) {
                    // ignore
                    break;
                }
                if (!isValidCStructMetaType(type)) {
                    // ignore
                    break;
                }

                QString idStr;

                if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                    idStr = QStringLiteral("WingCStruct::EnumValue");
                } else if (QMetaType::fromType<CINT_TYPE>().id() == int(type)) {
                    idStr = QStringLiteral("WingCStruct::IntType");
                } else {
                    idStr = WingAngelAPI::qvariantCastASString(type);
                }

                if (idStr.isEmpty()) {
                    // ignore
                    break;
                }
                QString arrType =
                    QStringLiteral("array<") + idStr + QStringLiteral(">");
                auto arrTypeID = engine->GetTypeIdByDecl(arrType.toUtf8());
                if (arrTypeID < 0) {
                    // ignore
                    break;
                }

                dic->Set(p->first, convert2AsArray(v, type, arrTypeID),
                         arrTypeID);
            }
            break;
        }
        case QMetaType::QVariantHash: {
            auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                AsUserDataType::UserData_DictionaryTypeInfo));
            auto id = type->GetTypeId();
            dic->Set(p->first, convert2AsDictionary(var.toHash()), id);
            break;
        }
        default:
            if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                static auto type =
                    engine->GetTypeInfoByName("WingCStruct::EnumValue");
                Q_ASSERT(type);
                if (type) {
                    auto id = type->GetTypeId();
                    Q_ASSERT(id >= 0);
                    auto v = var.value<CEnumValue>();
                    dic->Set(p->first, &v, id);
                }
            } else if (QMetaType::fromType<CINT_TYPE>().id() == int(type)) {
                static auto type =
                    engine->GetTypeInfoByName("WingCStruct::IntType");
                Q_ASSERT(type);
                if (type) {
                    auto id = type->GetTypeId();
                    Q_ASSERT(id >= 0);
                    auto v = var.value<CEnumValue>();
                    dic->Set(p->first, &v, id);
                }
            }
            break;
        }
    }
    return dic;
}

CScriptArray *StructParser::convert2AsArray(const QVariantList &array,
                                            QMetaType::Type type, int id) {
    Q_ASSERT(!array.isEmpty());
    auto ctx = asGetActiveContext();
    if (ctx == nullptr) {
        return nullptr;
    }

    auto engine = ctx->GetEngine();
    Q_ASSERT(engine);

    auto arr = CScriptArray::Create(engine->GetTypeInfoById(id), array.size());

    static asQWORD buffer;
    buffer = 0;

    for (asUINT i = 0; i < arr->GetSize(); ++i) {
        auto var = array.at(i);
        switch (type) {
        case QMetaType::Type::Bool:
            WingAngelAPI::assignTmpBuffer(buffer, var.toBool());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Short:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<short>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::UShort:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<ushort>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Int:
            WingAngelAPI::assignTmpBuffer(buffer, var.toInt());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::UInt:
            WingAngelAPI::assignTmpBuffer(buffer, var.toUInt());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Long:
        case QMetaType::LongLong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toLongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::ULong:
        case QMetaType::ULongLong:
            WingAngelAPI::assignTmpBuffer(buffer, var.toULongLong());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Float:
            WingAngelAPI::assignTmpBuffer(buffer, var.toFloat());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Double:
            WingAngelAPI::assignTmpBuffer(buffer, var.toDouble());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::Char16: {
            QChar ch(var.value<char16_t>());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::Char32: {
            QChar ch(var.value<char32_t>());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::Char: {
            QChar ch(var.toChar());
            arr->SetValue(i, &ch);
        } break;
        case QMetaType::UChar:
        case QMetaType::SChar:
            WingAngelAPI::assignTmpBuffer(buffer, var.value<uchar>());
            arr->SetValue(i, &buffer);
            break;
        case QMetaType::QVariantHash:
            // struct
            arr->SetValue(i, convert2AsDictionary(var.toHash()));
        default:
            if (QMetaType::fromType<CEnumValue>().id() == int(type)) {
                auto v = var.value<CEnumValue>();
                arr->SetValue(i, &v);
            }
            // ignore, but should not go there
            // nested array is not allowed
            break;
        }
    }

    return arr;
}
