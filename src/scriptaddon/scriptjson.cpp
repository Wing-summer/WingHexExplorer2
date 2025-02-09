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

#include "scriptjson.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

// ------------------------------
// QJsonDocument Wrappers
// ------------------------------

static void QJsonDocument_DefaultConstruct(void *memory) {
    new (memory) QJsonDocument();
}

static void QJsonDocument_CopyConstruct(void *memory,
                                        const QJsonDocument &other) {
    new (memory) QJsonDocument(other);
}

static void QJsonDocument_Destruct(QJsonDocument *doc) {
    doc->~QJsonDocument();
}

// Global function
static QJsonDocument QJsonDocument_FromJson(const QString &jsonStr) {
    return QJsonDocument::fromJson(jsonStr.toUtf8());
}

static QString QJsonDocument_ToJson(const QJsonDocument &doc,
                                    QJsonDocument::JsonFormat format) {
    return QString::fromUtf8(doc.toJson(format));
}

static QJsonObject QJsonDocument_Object(const QJsonDocument &doc) {
    return doc.object();
}

static QJsonArray QJsonDocument_Array(const QJsonDocument &doc) {
    return doc.array();
}

static bool QJsonDocument_IsArray(const QJsonDocument &doc) {
    return doc.isArray();
}

static bool QJsonDocument_IsNull(const QJsonDocument &doc) {
    return doc.isNull();
}

static bool QJsonDocument_IsObject(const QJsonDocument &doc) {
    return doc.isObject();
}

static bool QJsonDocument_IsEmpty(const QJsonDocument &doc) {
    return doc.isEmpty();
}

static void QJsonDocument_SetArray(QJsonDocument &doc,
                                   const QJsonArray &array) {
    doc.setArray(array);
}

static void QJsonDocument_SetObject(QJsonDocument &doc,
                                    const QJsonObject &obj) {
    doc.setObject(obj);
}

static QJsonValue QJsonDocument_opIndex(const QJsonDocument &doc,
                                        const QString &key) {
    return doc[key];
}

static QJsonValue QJsonDocument_opIndexInt(const QJsonDocument &doc, int i) {
    return doc[i];
}

// ------------------------------
// QJsonValue Wrappers
// ------------------------------

static void QJsonValue_DefaultConstruct(void *memory) {
    new (memory) QJsonValue();
}

static void QJsonValue_CopyConstruct(void *memory, const QJsonValue &other) {
    new (memory) QJsonValue(other);
}

static void QJsonValue_Destruct(QJsonValue *val) { val->~QJsonValue(); }

static void QJsonValue_ConstructBool(QJsonValue *self, bool b) {
    new (self) QJsonValue(b);
}

static void QJsonValue_ConstructString(QJsonValue *self, const QString &str) {
    new (self) QJsonValue(str);
}

static void QJsonValue_ConstructQJsonArray(QJsonValue *self,
                                           const QJsonArray &array) {
    new (self) QJsonValue(array);
}

static void QJsonValue_ConstructQJsonObject(QJsonValue *self,
                                            const QJsonObject &obj) {
    new (self) QJsonValue(obj);
}

static void QJsonValue_ConstructDouble(QJsonValue *self, double v) {
    new (self) QJsonValue(v);
}

static void QJsonValue_ConstructInt(QJsonValue *self, int v) {
    new (self) QJsonValue(v);
}

static bool QJsonValue_IsNull(const QJsonValue &val) { return val.isNull(); }

static bool QJsonValue_IsBool(const QJsonValue &val) { return val.isBool(); }

static bool QJsonValue_IsDouble(const QJsonValue &val) {
    return val.isDouble();
}

static bool QJsonValue_IsArray(const QJsonValue &val) { return val.isArray(); }

static bool QJsonValue_IsObject(const QJsonValue &val) {
    return val.isObject();
}

static bool QJsonValue_IsString(const QJsonValue &val) {
    return val.isString();
}

static bool QJsonValue_IsUndefined(const QJsonValue &val) {
    return val.isUndefined();
}

static QJsonArray QJsonValue_ToArray(const QJsonValue &val) {
    return val.toArray();
}

static bool QJsonValue_ToBool(const QJsonValue &val) { return val.toBool(); }

static double QJsonValue_ToDouble(const QJsonValue &val) {
    return val.toDouble();
}

static int QJsonValue_ToInt(const QJsonValue &val) { return val.toInt(); }

static QJsonObject QJsonValue_ToQJsonObject(const QJsonValue &val) {
    return val.toObject();
}

static QString QJsonValue_ToString(const QJsonValue &val) {
    return val.toString();
}

static QJsonValue QJsonValue_opIndex(const QJsonValue &obj,
                                     const QString &key) {
    return obj[key];
}

static QJsonValue QJsonValue_opIndexInt(const QJsonValue &obj, int i) {
    return obj[i];
}

// ------------------------------
// QJsonArray Wrappers
// ------------------------------

static void QJsonArray_DefaultConstruct(void *memory) {
    new (memory) QJsonArray();
}

static void QJsonArray_CopyConstruct(void *memory, const QJsonArray &other) {
    new (memory) QJsonArray(other);
}

static void QJsonArray_Destruct(QJsonArray *arr) { arr->~QJsonArray(); }

static int QJsonArray_Size(const QJsonArray &arr) { return arr.size(); }

static QJsonValue QJsonArray_At(const QJsonArray &arr, int index) {
    return arr.at(index);
}

static bool QJsonArray_IsEmpty(const QJsonArray &val) { return val.isEmpty(); }

static void QJsonArray_Append(QJsonArray &arr, const QJsonValue &val) {
    arr.append(val);
}

static void QJsonArray_Prepend(QJsonArray &arr, const QJsonValue &val) {
    arr.prepend(val);
}

static void QJsonArray_Insert(QJsonArray &arr, int i, const QJsonValue &val) {
    arr.insert(i, val);
}

static void QJsonArray_Replace(QJsonArray &arr, int i, const QJsonValue &val) {
    arr.replace(i, val);
}

static void QJsonArray_RemoveAt(QJsonArray &arr, int i) { arr.removeAt(i); }

static void QJsonArray_RemoveFirst(QJsonArray &arr) { arr.removeFirst(); }

static void QJsonArray_RemoveLast(QJsonArray &arr) { arr.removeLast(); }

static QJsonArray QJsonArray_opAdd(const QJsonArray &arr,
                                   const QJsonValue &value) {
    return arr + value;
}

static QJsonArray &QJsonArray_opAddAssign(QJsonArray &arr,
                                          const QJsonValue &value) {
    arr += value;
    return arr;
}

// ------------------------------
// QJsonObject Wrappers
// ------------------------------

static void QJsonObject_Construct(QJsonObject *self) {
    new (self) QJsonObject();
}

static void QJsonObject_CopyConstruct(QJsonObject *self,
                                      const QJsonObject &other) {
    new (self) QJsonObject(other);
}

static void QJsonObject_Destruct(QJsonObject *self) { self->~QJsonObject(); }

static QJsonValue QJsonObject_opIndex(const QJsonObject &obj,
                                      const QString &key) {
    return obj[key];
}

static bool QJsonObject_HasKey(const QJsonObject &obj, const QString &key) {
    return obj.contains(key);
}

static void QJsonObject_SetValue(QJsonObject &obj, const QString &key,
                                 const QJsonValue &val) {
    obj.insert(key, val);
}

static void QJsonObject_Remove(QJsonObject &obj, const QString &key) {
    obj.remove(key);
}

static bool QJsonObject_IsEmpty(const QJsonObject &obj) {
    return obj.isEmpty();
}

static int QJsonObject_Size(const QJsonObject &obj) { return obj.size(); }

// ------------------------------
// Register functions
// ------------------------------

void RegisterQJsonDocument(asIScriptEngine *engine) {
    int r;

    // Register QJsonDocument as a value type.
    // The flag asOBJ_APP_CLASS_CDAK is used for types with a Constructor,
    // Destructor, Assignment operator, and Copy constructor.
    r = engine->RegisterObjectType("JsonDocument", sizeof(QJsonDocument),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonDocument>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonDocument", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonDocument_DefaultConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonDocument", asBEHAVE_CONSTRUCT, "void f(const JsonDocument &in)",
        asFUNCTION(QJsonDocument_CopyConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonDocument", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(QJsonDocument_Destruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterEnum("JsonFormat");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("JsonFormat", "Indented",
                                  QJsonDocument::Indented);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("JsonFormat", "Compact",
                                  QJsonDocument::Compact);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonDocument",
        "string toJson(JsonFormat format = Json::JsonFormat::Indented) const",
        asFUNCTION(QJsonDocument_ToJson), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonDocument", "bool isArray() const",
                                     asFUNCTION(QJsonDocument_IsArray),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonDocument", "bool isEmpty() const",
                                     asFUNCTION(QJsonDocument_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonDocument", "bool isNull() const",
                                     asFUNCTION(QJsonDocument_IsNull),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonDocument", "bool isObject() const",
                                     asFUNCTION(QJsonDocument_IsObject),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonDocument", "JsonObject object() const",
        asFUNCTION(QJsonDocument_Object), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonDocument", "void setArray(const JsonArray &in array)",
        asFUNCTION(QJsonDocument_SetArray), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonDocument", "void setObject(const JsonObject &in obj)",
        asFUNCTION(QJsonDocument_SetObject), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "JsonDocument", "JsonValue opIndex(const string &in key) const",
        asFUNCTION(QJsonDocument_opIndex), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonDocument", "JsonValue opIndex(int index) const",
        asFUNCTION(QJsonDocument_opIndexInt), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // global function
    r = engine->RegisterGlobalFunction(
        "JsonDocument fromJson(const string &in)",
        asFUNCTION(QJsonDocument_FromJson), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

// Register QJsonValue with AngelScript
void RegisterQJsonValue(asIScriptEngine *engine) {
    int r;
    r = engine->RegisterObjectType("JsonValue", sizeof(QJsonValue),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonValue>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Constructors / Destructor
    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonValue_DefaultConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(const JsonValue &in)",
        asFUNCTION(QJsonValue_CopyConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(bool)",
        asFUNCTION(QJsonValue_ConstructBool), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(int)",
        asFUNCTION(QJsonValue_ConstructInt), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(double)",
        asFUNCTION(QJsonValue_ConstructDouble), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(QJsonValue_ConstructString), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(QJsonValue_Destruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Methods
    r = engine->RegisterObjectMethod("JsonValue", "bool isNull() const",
                                     asFUNCTION(QJsonValue_IsNull),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isBool() const",
                                     asFUNCTION(QJsonValue_IsBool),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isDouble() const",
                                     asFUNCTION(QJsonValue_IsDouble),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isString() const",
                                     asFUNCTION(QJsonValue_IsString),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isUndefined() const",
                                     asFUNCTION(QJsonValue_IsUndefined),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "string toString() const",
                                     asFUNCTION(QJsonValue_ToString),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool toBool() const",
                                     asFUNCTION(QJsonValue_ToBool),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "double toDouble() const",
                                     asFUNCTION(QJsonValue_ToDouble),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "int toInt() const",
                                     asFUNCTION(QJsonValue_ToInt),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "JsonValue", "JsonValue opIndex(const string &in key) const",
        asFUNCTION(QJsonValue_opIndex), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonValue", "JsonValue opIndex(int index) const",
        asFUNCTION(QJsonValue_opIndexInt), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void RegisterQJsonValue_End(asIScriptEngine *engine) {
    auto r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(const JsonArray &in)",
        asFUNCTION(QJsonValue_ConstructQJsonArray), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonValue", asBEHAVE_CONSTRUCT, "void f(const JsonObject &in)",
        asFUNCTION(QJsonValue_ConstructQJsonObject), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isArray() const",
                                     asFUNCTION(QJsonValue_IsArray),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "bool isObject() const",
                                     asFUNCTION(QJsonValue_IsObject),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "JsonArray toArray() const",
                                     asFUNCTION(QJsonValue_ToArray),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonValue", "JsonObject toObject() const",
                                     asFUNCTION(QJsonValue_ToQJsonObject),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

// Register QJsonArray with AngelScript
void RegisterQJsonArray(asIScriptEngine *engine) {
    int r;
    r = engine->RegisterObjectType("JsonArray", sizeof(QJsonArray),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonArray>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Constructors / Destructor
    r = engine->RegisterObjectBehaviour(
        "JsonArray", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonArray_DefaultConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonArray", asBEHAVE_CONSTRUCT, "void f(const JsonArray &in)",
        asFUNCTION(QJsonArray_CopyConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonArray", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(QJsonArray_Destruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Methods
    r = engine->RegisterObjectMethod("JsonArray", "int size() const",
                                     asFUNCTION(QJsonArray_Size),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonArray", "bool isEmpty() const",
                                     asFUNCTION(QJsonArray_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonArray", "JsonValue at(int) const",
                                     asFUNCTION(QJsonArray_At),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "void append(const JsonValue &in val)",
        asFUNCTION(QJsonArray_Append), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "void prepend(const JsonValue &in val)",
        asFUNCTION(QJsonArray_Prepend), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "void insert(int index, const JsonValue &in val)",
        asFUNCTION(QJsonArray_Insert), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "void replace(int index, const JsonValue &in val)",
        asFUNCTION(QJsonArray_Replace), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonArray", "void removeAt(int index)",
                                     asFUNCTION(QJsonArray_RemoveAt),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonArray", "void removeFirst()",
                                     asFUNCTION(QJsonArray_RemoveFirst),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonArray", "void removeLast()",
                                     asFUNCTION(QJsonArray_RemoveLast),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "JsonArray opAdd(const JsonValue &in) const",
        asFUNCTION(QJsonArray_opAdd), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonArray", "JsonArray &opAddAssign(const JsonValue &in)",
        asFUNCTION(QJsonArray_opAddAssign), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void RegisterQJsonObject(asIScriptEngine *engine) {
    int r;

    r = engine->RegisterObjectType("JsonObject", sizeof(QJsonObject),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonObject>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonObject", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonObject_Construct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonObject", asBEHAVE_CONSTRUCT, "void f(const JsonObject &in)",
        asFUNCTION(QJsonObject_CopyConstruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "JsonObject", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(QJsonObject_Destruct), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonObject", "bool hasKey(const string &in key) const",
        asFUNCTION(QJsonObject_HasKey), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonObject",
        "void set(const string &in key, const JsonValue &in value)",
        asFUNCTION(QJsonObject_SetValue), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "JsonObject", "void remove(const string &in key)",
        asFUNCTION(QJsonObject_Remove), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonObject", "bool isEmpty() const",
                                     asFUNCTION(QJsonObject_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("JsonObject", "int size() const",
                                     asFUNCTION(QJsonObject_Size),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "JsonObject", "JsonValue opIndex(const string &in key) const",
        asFUNCTION(QJsonObject_opIndex), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
}

void RegisterQJson(asIScriptEngine *engine) {
    auto r = engine->SetDefaultNamespace("Json");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    RegisterQJsonValue(engine);
    RegisterQJsonArray(engine);
    RegisterQJsonObject(engine);
    RegisterQJsonValue_End(engine);
    RegisterQJsonDocument(engine);
    engine->SetDefaultNamespace("");
}
