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

#ifndef ASTYPE_EVALUATOR_H
#define ASTYPE_EVALUATOR_H

#include "scriptaddon/scriptany.h"

#include "WingPlugin/iwingangel.h"
#include "as-debugger/as_debugger.h"
#include "define.h"
#include "fmtlibext.h"
#include "src/scriptaddon/scriptqdictionary.h"

#include <QChar>
#include <QColor>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QString>

class asIDBArrayTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto arr = var->address.ResolveAs<CScriptArray>();
        var->expandable = !arr->IsEmpty();
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);
        var->value = fmt::format(
            FMT_STRING("array<{}>[{}]"),
            cache.GetTypeNameFromType(arr->GetElementTypeId()), arr->GetSize());
    }
    virtual void Expand(asIDBVariable::Ptr var) const override {
        QueryVariableForEach(var, 0);
    }
};

class asIDBStringTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QString *s = var->address.ResolveAs<const QString>();
        if (s->isEmpty()) {
            var->value = R"("")";
            var->expandable = false;
        } else {
            var->value =
                fmt::format(FMT_STRING(R"("{}")"), escapeNonPrintable(*s));
            var->expandable = true;
        }
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        QueryVariableForEach(var, 0);
    }
};

class asIDBRegExTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QRegularExpression *s =
            var->address.ResolveAs<const QRegularExpression>();
        auto p = s->pattern();
        var->value = fmt::format(FMT_STRING("exp</{}/>"), s->pattern());
    }
};

class asIDBRegMatchTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QRegularExpressionMatch *s =
            var->address.ResolveAs<const QRegularExpressionMatch>();
        auto r = s->regularExpression();
        var->value = fmt::format(FMT_STRING("match</{}/>"), r.pattern());
    }
};

class asIDBCharTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QChar *s = var->address.ResolveAs<const QChar>();
        var->value =
            fmt::format(FMT_STRING("'{}'"), escapeNonPrintable(QString(*s)));
    }
};

class asIDBUrlTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);
        var->expandable = true;

        const QUrl *s = var->address.ResolveAs<const QUrl>();
        var->value = fmt::format(FMT_STRING(R"(url<"{}">)"), s->fileName());
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = static_cast<asITypeInfo *>(ctx->GetEngine()->GetUserData(
            AsUserDataType::UserData_StringTypeInfo));
        if (type == nullptr) {
            return;
        }
        auto typeID = type->GetTypeId();
        const QUrl *s = var->address.ResolveAs<const QUrl>();

        {
            // scheme
            auto child = var->CreateChildVariable(
                "[scheme]", {}, cache.GetTypeNameFromType({typeID, asTM_NONE}));
            child->evaluated = true;
            child->value = fmt::format(FMT_STRING(R"("{}")"), s->scheme());
        }
        {
            // authority
            auto child = var->CreateChildVariable(
                "[authority]", {},
                cache.GetTypeNameFromType({typeID, asTM_NONE}));
            child->evaluated = true;
            child->value = fmt::format(FMT_STRING(R"("{}")"), s->authority());
        }
        {
            // path
            auto child = var->CreateChildVariable(
                "[path]", {}, cache.GetTypeNameFromType({typeID, asTM_NONE}));
            child->evaluated = true;
            child->value = fmt::format(FMT_STRING(R"("{}")"), s->path());
        }
        {
            // fileName
            auto child = var->CreateChildVariable(
                "[fileName]", {},
                cache.GetTypeNameFromType({typeID, asTM_NONE}));
            child->evaluated = true;
            child->value = fmt::format(FMT_STRING(R"("{}")"), s->fileName());
        }
    }
};

class asIDBColorTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QColor *s = var->address.ResolveAs<const QColor>();
        var->value = fmt::format(FMT_STRING("color<{}>"), s->name());
    }
};

class asIDBAnyTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();

        if (v->GetTypeId() == 0) {
            var->value = "any<null>";
            return;
        }

        var->value =
            fmt::format(FMT_STRING("any<{}>"),
                        var->dbg.cache->GetTypeNameFromType(v->GetTypeId()));
        var->expandable = true;
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        const CScriptAny *v = var->address.ResolveAs<const CScriptAny>();
        auto tid = v->GetTypeId();
        asIDBVarAddr id(tid, false, const_cast<void *>(v->GetAddressOfValue()));
        var->CreateChildVariable(
            "value", id, var->dbg.cache->GetTypeNameFromType(v->GetTypeId()));
    }
};

class asIDBDictionaryTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Expand(asIDBVariable::Ptr var) const override {
        const CScriptDictionary *v =
            var->address.ResolveAs<const CScriptDictionary>();

        for (auto &kvp : *v) {
            auto child = var->CreateChildVariable(
                fmt::format(FMT_STRING("[\"{}\"]"), kvp.GetKey()),
                {kvp.GetTypeId(), false,
                 const_cast<void *>(kvp.GetAddressOfValue())},
                var->dbg.cache->GetTypeNameFromType(kvp.GetTypeId()));
            child->Evaluate();
        }
    }
};

class asIDBJsonDocumentTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QJsonDocument *v = var->address.ResolveAs<const QJsonDocument>();
        if (v->isEmpty() || v->isNull()) {
            var->value = "json::document<null>";
        } else {
            if (v->isArray()) {
                auto arr = v->array();
                auto len = arr.size();
                var->value = fmt::format(
                    FMT_STRING("json::document<json::array>[{}]"), len);
                var->expandable = len > 0;
            } else if (v->isObject()) {
                auto obj = v->object();
                auto len = obj.size();
                var->value = fmt::format(
                    FMT_STRING("json::document<json::object>[{}]"), len);
                var->expandable = len > 0;
            }
        }
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        const QJsonDocument *v = var->address.ResolveAs<const QJsonDocument>();

        if (v->isEmpty() || v->isNull()) {
            return;
        }

        auto engine = var->dbg.cache->ctx->GetEngine();
        auto type = static_cast<asITypeInfo *>(
            engine->GetUserData(AsUserDataType::UserData_JsonValueTypeInfo));
        auto typeId = type->GetTypeId();
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        if (v->isArray()) {
            auto arr = v->array();
            auto total = arr.size();
            for (qsizetype i = 0; i < total; i++) {
                QJsonValue temp = arr.at(i);
                auto child = var->CreateChildVariable(
                    fmt::format(FMT_STRING("[{}]"), i),
                    asIDBVarAddr{typeId, false, &temp}, var->typeName);
                child->stackValue =
                    asIDBValue(engine, &temp, typeId, asTM_NONE);
                child->address.address = child->stackValue.GetPointer<void>();
            }
        } else if (v->isObject()) {
            auto obj = v->object();
            for (auto p = obj.begin(); p != obj.end(); p++) {
                auto key = p.key();
                QJsonValue temp = p.value();
                auto child = var->CreateChildVariable(
                    fmt::format(FMT_STRING(R"(["{}"])"), key),
                    asIDBVarAddr{typeId, false, &temp}, var->typeName);
                child->stackValue =
                    asIDBValue(engine, &temp, typeId, asTM_NONE);
                child->address.address = child->stackValue.GetPointer<void>();
            }
        }
    }
};

class asIDBJsonArrayTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Expand(asIDBVariable::Ptr var) const override {
        QueryVariableForEach(var, 0);
    }
};

class asIDBJsonObjectTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Expand(asIDBVariable::Ptr var) const override {
        const QJsonObject *v = var->address.ResolveAs<const QJsonObject>();

        if (v->isEmpty()) {
            return;
        }

        auto engine = var->dbg.cache->ctx->GetEngine();
        auto type = static_cast<asITypeInfo *>(
            engine->GetUserData(AsUserDataType::UserData_JsonValueTypeInfo));
        auto typeId = type->GetTypeId();
        for (auto p = v->begin(); p != v->end(); p++) {
            auto key = p.key();
            QJsonValue temp = p.value();
            auto child = var->CreateChildVariable(
                fmt::format(FMT_STRING(R"(["{}"])"), key),
                asIDBVarAddr{typeId, false, &temp}, var->typeName);
            child->stackValue = asIDBValue(engine, &temp, typeId, asTM_NONE);
            child->address.address = child->stackValue.GetPointer<void>();
        }
    }
};

class asIDBJsonValueTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        const QJsonValue *v = var->address.ResolveAs<const QJsonValue>();
        switch (v->type()) {
        case QJsonValue::Null:
            var->value = "null";
            var->expandable = false;
            break;
        case QJsonValue::Bool:
            var->value = fmt::to_string(v->toBool());
            var->expandable = false;
            break;
        case QJsonValue::Double:
            var->value = fmt::to_string(v->toDouble());
            var->expandable = false;
            break;
        case QJsonValue::String: {
            auto str = v->toString();
            if (str.isEmpty()) {
                var->value = "\"\"";
            } else {
                var->value =
                    fmt::format(FMT_STRING(R"("{}")"), escapeNonPrintable(str));
            }
            var->expandable = false;
        } break;
        case QJsonValue::Array: {
            auto arr = v->toArray();
            auto len = arr.size();
            var->value = fmt::format(FMT_STRING("json::array[{}]"), len);
            var->expandable = len > 0;
        } break;
        case QJsonValue::Object: {
            auto obj = v->toObject();
            auto len = obj.size();
            var->value = fmt::format(FMT_STRING("json::object[{}]"), len);
            var->expandable = len > 0;
        } break;
        case QJsonValue::Undefined:
            var->value = "undefined";
            var->expandable = false;
            break;
        }
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        const QJsonValue *v = var->address.ResolveAs<const QJsonValue>();
        switch (v->type()) {
        case QJsonValue::Null:
        case QJsonValue::Bool:
        case QJsonValue::Double:
        case QJsonValue::String:
        case QJsonValue::Undefined:
            break;
        case QJsonValue::Array: {
            auto arr = v->toArray();
            auto total = arr.size();
            auto engine = var->dbg.cache->ctx->GetEngine();
            auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                AsUserDataType::UserData_JsonValueTypeInfo));
            auto typeId = type->GetTypeId();
            for (qsizetype i = 0; i < total; i++) {
                QJsonValue temp = arr.at(i);
                auto child = var->CreateChildVariable(
                    fmt::format(FMT_STRING("[{}]"), i),
                    asIDBVarAddr{typeId, false, &temp}, var->typeName);
                child->stackValue =
                    asIDBValue(engine, &temp, typeId, asTM_NONE);
                child->address.address = child->stackValue.GetPointer<void>();
            }
        } break;
        case QJsonValue::Object: {
            auto obj = v->toObject();
            auto engine = var->dbg.cache->ctx->GetEngine();
            auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                AsUserDataType::UserData_JsonValueTypeInfo));
            auto typeId = type->GetTypeId();
            for (auto p = obj.begin(); p != obj.end(); p++) {
                auto key = p.key();
                QJsonValue temp = p.value();
                auto child = var->CreateChildVariable(
                    fmt::format(FMT_STRING(R"(["{}"])"), key),
                    asIDBVarAddr{typeId, false, &temp}, var->typeName);
                child->stackValue =
                    asIDBValue(engine, &temp, typeId, asTM_NONE);
                child->address.address = child->stackValue.GetPointer<void>();
            }
        } break;
        }
    }
};

class asIDBCustomTypeEvaluator : public asIDBObjectTypeEvaluator {
public:
    asIDBCustomTypeEvaluator(
        const QHash<std::string_view, WingHex::IWingAngel::Evaluator> &evals)
        : evals(evals) {}

    virtual void Evaluate(asIDBVariable::Ptr var) const override {
        auto &dbg = var->dbg;
        auto &cache = *dbg.cache;
        auto ctx = cache.ctx;
        auto type = ctx->GetEngine()->GetTypeInfoById(var->address.typeId);
        if (type == nullptr) {
            return;
        }
        var->typeName = cache.GetTypeNameFromType(var->address.typeId);

        if (evals.contains(var->typeName)) {
            auto r = evals[var->typeName];
            auto ret =
                r(var->address.address, WingHex::IWingAngel::EvalMode::Eval);
            if (std::holds_alternative<WingHex::IWingAngel::EvalResult>(ret)) {
                auto r = std::get<WingHex::IWingAngel::EvalResult>(ret);
                var->value = r.value.toStdString();
                var->expandable = r.expandable;
            }
        }
    }

    virtual void Expand(asIDBVariable::Ptr var) const override {
        if (evals.contains(var->typeName)) {
            auto r = evals[var->typeName];
            auto ret =
                r(var->address.address, WingHex::IWingAngel::EvalMode::Expand);
            auto engine = var->dbg.cache->ctx->GetEngine();

            if (std::holds_alternative<
                    QVector<WingHex::IWingAngel::ExpandResult>>(ret)) {
                auto rc =
                    std::get<QVector<WingHex::IWingAngel::ExpandResult>>(ret);
                auto total = rc.size();
                for (qsizetype i = 0; i < total; ++i) {
                    auto data = rc.at(i);

                    auto typeId = engine->GetTypeIdByDecl(data.type.toUtf8());
                    auto temp = data.buffer;
                    if (temp) {
                        auto child = var->CreateChildVariable(
                            fmt::format(FMT_STRING("[{}]"), i),
                            asIDBVarAddr{typeId, false, temp}, var->typeName);
                        child->stackValue =
                            asIDBValue(engine, temp, typeId, asTM_NONE);
                        child->address.address =
                            child->stackValue.GetPointer<void>();
                        continue;
                    }

                    auto child = var->CreateChildVariable(
                        fmt::format(FMT_STRING("[{}]"), i),
                        asIDBVarAddr{asTYPEID_VOID, false, nullptr},
                        var->typeName);
                    child->stackValue =
                        asIDBValue(engine, nullptr, asTYPEID_VOID, asTM_NONE);
                    child->address.address =
                        child->stackValue.GetPointer<void>();
                }
            } else if (std::holds_alternative<
                           QHash<QString, WingHex::IWingAngel::ExpandResult>>(
                           ret)) {
                auto rc =
                    std::get<QHash<QString, WingHex::IWingAngel::ExpandResult>>(
                        ret);

                for (auto &&[key, value] : rc.asKeyValueRange()) {
                    auto k = fmt::format(FMT_STRING(R"([{}])"), key);
                    auto typeId = engine->GetTypeIdByDecl(value.type.toUtf8());
                    auto temp = value.buffer;
                    if (temp) {
                        auto child = var->CreateChildVariable(
                            k, asIDBVarAddr{typeId, false, temp},
                            var->typeName);
                        child->stackValue =
                            asIDBValue(engine, temp, typeId, asTM_NONE);
                        child->address.address =
                            child->stackValue.GetPointer<void>();
                        continue;
                    }

                    auto child = var->CreateChildVariable(
                        k, asIDBVarAddr{asTYPEID_VOID, false, nullptr},
                        var->typeName);
                    child->stackValue =
                        asIDBValue(engine, nullptr, asTYPEID_VOID, asTM_NONE);
                    child->address.address =
                        child->stackValue.GetPointer<void>();
                }
            }
        } else {
            asIDBObjectTypeEvaluator::Expand(var);
        }
    }

private:
    QHash<std::string_view, WingHex::IWingAngel::Evaluator> evals;
};

#endif // ASTYPE_EVALUATOR_H
