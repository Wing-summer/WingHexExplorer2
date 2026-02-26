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
#include "define.h"

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

static void QJsonDocument_ObjConstruct(void *memory, const QJsonObject &other) {
    new (memory) QJsonDocument(other);
}

static void QJsonDocument_ArrayConstruct(void *memory,
                                         const QJsonArray &other) {
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

static QString QJsonDocument_ToString(const QJsonDocument &doc) {
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
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

static QString QJsonValue_ToStringValue(const QJsonValue &val) {
    return val.toString();
}

static QString QJsonObject_ToString(const QJsonObject &);
static QString QJsonArray_ToString(const QJsonArray &);

static QString QJsonValue_ToString(const QJsonValue &val) {
    switch (val.type()) {
    case QJsonValue::Null:
        return QStringLiteral("null");
    case QJsonValue::Bool:
        return val.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    case QJsonValue::Double:
        return QString::number(val.toDouble());
    case QJsonValue::String:
        return val.toString();
    case QJsonValue::Array:
        return QJsonArray_ToString(val.toArray());
    case QJsonValue::Object:
        return QJsonObject_ToString(val.toObject());
    case QJsonValue::Undefined:
        return QStringLiteral("undefined");
    }
    return {};
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

asUINT QJsonArray_opForBegin(const QJsonArray *) { return 0; }

bool QJsonArray_opForEnd(asUINT iter, const QJsonArray *arr) {
    if (arr == nullptr || arr->count() <= iter)
        return true;

    return false;
}

asUINT QJsonArray_opForNext(asUINT iter, const QJsonArray *) {
    return iter + 1;
}

asUINT QJsonArray_opForValue1(asUINT iter, const QJsonArray *) { return iter; }

static QString QJsonArray_ToString(const QJsonArray &arr) {
    auto doc = QJsonDocument(arr);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
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

static QString QJsonObject_ToString(const QJsonObject &obj) {
    QJsonDocument doc(obj);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

// ------------------------------
// Register functions
// ------------------------------

void RegisterQJsonDocument(asIScriptEngine *engine) {
    int r;

    // Register QJsonDocument as a value type.
    // The flag asOBJ_APP_CLASS_CDAK is used for types with a Constructor,
    // Destructor, Assignment operator, and Copy constructor.
    r = engine->RegisterObjectType("document", sizeof(QJsonDocument),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonDocument>());
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "document", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonDocument_DefaultConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "document", asBEHAVE_CONSTRUCT, "void f(const json::document &in)",
        asFUNCTION(QJsonDocument_CopyConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "document", asBEHAVE_CONSTRUCT, "void f(const json::object &in)",
        asFUNCTION(QJsonDocument_ObjConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "document", asBEHAVE_CONSTRUCT, "void f(const json::array &in)",
        asFUNCTION(QJsonDocument_ArrayConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "document", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTION(QJsonDocument_Destruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterEnum("Format");
    ASSERT(r >= 0);

    r = engine->RegisterEnumValue("Format", "Indented",
                                  QJsonDocument::Indented);
    ASSERT(r >= 0);

    r = engine->RegisterEnumValue("Format", "Compact", QJsonDocument::Compact);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "document",
        "string toJson(json::Format format = json::Format::Indented) const",
        asFUNCTION(QJsonDocument_ToJson), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "string toString() const",
                                     asFUNCTION(QJsonDocument_ToString),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "bool isArray() const",
                                     asFUNCTION(QJsonDocument_IsArray),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "bool isEmpty() const",
                                     asFUNCTION(QJsonDocument_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "bool isNull() const",
                                     asFUNCTION(QJsonDocument_IsNull),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "bool isObject() const",
                                     asFUNCTION(QJsonDocument_IsObject),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("document", "json::object object() const",
                                     asFUNCTION(QJsonDocument_Object),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "document", "void setArray(const json::array &in array)",
        asFUNCTION(QJsonDocument_SetArray), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "document", "void setObject(const json::object &in obj)",
        asFUNCTION(QJsonDocument_SetObject), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "document", "json::value &opIndex(const string &in key) const",
        asFUNCTION(QJsonDocument_opIndex), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "document", "json::value opIndex(int index) const",
        asFUNCTION(QJsonDocument_opIndexInt), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // global function
    r = engine->RegisterGlobalFunction(
        "json::document fromJson(const string &in)",
        asFUNCTION(QJsonDocument_FromJson), asCALL_CDECL);
    ASSERT(r >= 0);
}

// Register QJsonValue with AngelScript
void RegisterQJsonValue(asIScriptEngine *engine) {
    int r;
    r = engine->RegisterObjectType("value", sizeof(QJsonValue),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonValue>());
    ASSERT(r >= 0);

    // Constructors / Destructor
    r = engine->RegisterObjectBehaviour("value", asBEHAVE_CONSTRUCT, "void f()",
                                        asFUNCTION(QJsonValue_DefaultConstruct),
                                        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(const json::value &in)",
        asFUNCTION(QJsonValue_CopyConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(bool)",
        asFUNCTION(QJsonValue_ConstructBool), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(int)",
        asFUNCTION(QJsonValue_ConstructInt), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(double)",
        asFUNCTION(QJsonValue_ConstructDouble), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(QJsonValue_ConstructString), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour("value", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(QJsonValue_Destruct),
                                        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // Methods
    r = engine->RegisterObjectMethod("value", "bool isNull() const",
                                     asFUNCTION(QJsonValue_IsNull),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isBool() const",
                                     asFUNCTION(QJsonValue_IsBool),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isDouble() const",
                                     asFUNCTION(QJsonValue_IsDouble),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isString() const",
                                     asFUNCTION(QJsonValue_IsString),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isUndefined() const",
                                     asFUNCTION(QJsonValue_IsUndefined),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "string toStringValue() const",
                                     asFUNCTION(QJsonValue_ToStringValue),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "string toString() const",
                                     asFUNCTION(QJsonValue_ToString),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool toBool() const",
                                     asFUNCTION(QJsonValue_ToBool),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "double toDouble() const",
                                     asFUNCTION(QJsonValue_ToDouble),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "int toInt() const",
                                     asFUNCTION(QJsonValue_ToInt),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "value", "value opIndex(const string &in key) const",
        asFUNCTION(QJsonValue_opIndex), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "value opIndex(int index) const",
                                     asFUNCTION(QJsonValue_opIndexInt),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQJsonValue_End(asIScriptEngine *engine) {
    auto r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(const array &in)",
        asFUNCTION(QJsonValue_ConstructQJsonArray), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "value", asBEHAVE_CONSTRUCT, "void f(const object &in)",
        asFUNCTION(QJsonValue_ConstructQJsonObject), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isArray() const",
                                     asFUNCTION(QJsonValue_IsArray),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "bool isObject() const",
                                     asFUNCTION(QJsonValue_IsObject),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "array toArray() const",
                                     asFUNCTION(QJsonValue_ToArray),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("value", "object toObject() const",
                                     asFUNCTION(QJsonValue_ToQJsonObject),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

// Register QJsonArray with AngelScript
void RegisterQJsonArray(asIScriptEngine *engine) {
    int r;
    r = engine->RegisterObjectType("array", sizeof(QJsonArray),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonArray>());
    ASSERT(r >= 0);

    // Constructors / Destructor
    r = engine->RegisterObjectBehaviour("array", asBEHAVE_CONSTRUCT, "void f()",
                                        asFUNCTION(QJsonArray_DefaultConstruct),
                                        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "array", asBEHAVE_CONSTRUCT, "void f(const array &in)",
        asFUNCTION(QJsonArray_CopyConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour("array", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(QJsonArray_Destruct),
                                        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // Methods
    r = engine->RegisterObjectMethod("array", "int size() const",
                                     asFUNCTION(QJsonArray_Size),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "bool isEmpty() const",
                                     asFUNCTION(QJsonArray_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "value at(int) const",
                                     asFUNCTION(QJsonArray_At),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "void append(const value &in val)",
        asFUNCTION(QJsonArray_Append), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "void prepend(const value &in val)",
        asFUNCTION(QJsonArray_Prepend), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "void insert(int index, const value &in val)",
        asFUNCTION(QJsonArray_Insert), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "void replace(int index, const value &in val)",
        asFUNCTION(QJsonArray_Replace), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "void removeAt(int index)",
                                     asFUNCTION(QJsonArray_RemoveAt),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "void removeFirst()",
                                     asFUNCTION(QJsonArray_RemoveFirst),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "void removeLast()",
                                     asFUNCTION(QJsonArray_RemoveLast),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "array opAdd(const value &in) const",
        asFUNCTION(QJsonArray_opAdd), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "array &opAddAssign(const value &in)",
        asFUNCTION(QJsonArray_opAddAssign), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // Support for foreach
    r = engine->RegisterObjectMethod(
        "array", "uint opForBegin() const",
        asFUNCTIONPR(QJsonArray_opForBegin, (const QJsonArray *), asUINT),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "bool opForEnd(uint) const",
        asFUNCTIONPR(QJsonArray_opForEnd, (asUINT, const QJsonArray *), bool),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "uint opForNext(uint) const",
                                     asFUNCTIONPR(QJsonArray_opForNext,
                                                  (asUINT, const QJsonArray *),
                                                  asUINT),
                                     asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "value opForValue0(uint index) const",
        asMETHODPR(QJsonArray, at, (qsizetype) const, QJsonValue),
        asCALL_THISCALL);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "array", "uint opForValue1(uint index) const",
        asFUNCTIONPR(QJsonArray_opForValue1, (asUINT, const QJsonArray *),
                     asUINT),
        asCALL_CDECL_OBJLAST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("array", "string toString() const",
                                     asFUNCTION(QJsonArray_ToString),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQJsonObject(asIScriptEngine *engine) {
    int r;

    r = engine->RegisterObjectType("object", sizeof(QJsonObject),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_CDAK |
                                       asGetTypeTraits<QJsonObject>());
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "object", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTION(QJsonObject_Construct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour(
        "object", asBEHAVE_CONSTRUCT, "void f(const object &in)",
        asFUNCTION(QJsonObject_CopyConstruct), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectBehaviour("object", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(QJsonObject_Destruct),
                                        asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "object", "bool hasKey(const string &in key) const",
        asFUNCTION(QJsonObject_HasKey), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "object", "void set(const string &in key, const value &in value)",
        asFUNCTION(QJsonObject_SetValue), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "object", "void remove(const string &in key)",
        asFUNCTION(QJsonObject_Remove), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("object", "bool isEmpty() const",
                                     asFUNCTION(QJsonObject_IsEmpty),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("object", "int size() const",
                                     asFUNCTION(QJsonObject_Size),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    // opIndex for getting values
    r = engine->RegisterObjectMethod(
        "object", "value opIndex(const string &in key) const",
        asFUNCTION(QJsonObject_opIndex), asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);

    r = engine->RegisterObjectMethod("object", "string toString() const",
                                     asFUNCTION(QJsonObject_ToString),
                                     asCALL_CDECL_OBJFIRST);
    ASSERT(r >= 0);
}

void RegisterQJson(asIScriptEngine *engine) {
    auto r = engine->SetDefaultNamespace("json");
    ASSERT(r >= 0);

    RegisterQJsonValue(engine);
    RegisterQJsonArray(engine);
    RegisterQJsonObject(engine);
    RegisterQJsonValue_End(engine);
    RegisterQJsonDocument(engine);
    engine->SetDefaultNamespace("");
}
