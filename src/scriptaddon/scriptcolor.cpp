#include "scriptcolor.h"
#include <QColor>

void RegisterColor(asIScriptEngine *engine) {
    // Register the QColor type
    int r = engine->RegisterObjectType("color", sizeof(QColor),
                                       asOBJ_VALUE | asOBJ_POD |
                                           asOBJ_APP_CLASS_ALLINTS |
                                           asGetTypeTraits<QColor>());
    Q_ASSERT(r >= 0);

    // Register the constructors
    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR([](void *memory) { new (memory) QColor(); }, (void *),
                     void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(int r, int g, int b, int a = 255)",
        asFUNCTIONPR([](void *memory, int r, int g, int b,
                        int a) { new (memory) QColor(r, g, b, a); },
                     (void *, int, int, int, int), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);

    // Register the methods
    r = engine->RegisterObjectMethod(
        "color", "void setRgb(int r, int g, int b, int a = 255)",
        asMETHODPR(QColor, setRgb, (int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int red() const",
                                     asMETHOD(QColor, red), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int green() const",
                                     asMETHOD(QColor, green), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int blue() const",
                                     asMETHOD(QColor, blue), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int alpha() const",
                                     asMETHOD(QColor, alpha), asCALL_THISCALL);
    Q_ASSERT(r >= 0);

    r = engine->RegisterObjectMethod(
        "color", "void setHsv(int r, int g, int b, int a = 255)",
        asMETHODPR(QColor, setHsv, (int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int hsvHue() const",
                                     asMETHOD(QColor, hsvHue), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int hsvSaturation() const",
                                     asMETHOD(QColor, hsvSaturation),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("color", "int value() const",
                                     asMETHOD(QColor, value), asCALL_THISCALL);
    Q_ASSERT(r >= 0);

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    r = engine->RegisterObjectMethod("color", "string name()",
                                     asMETHOD(QColor, name), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
#else
    r = engine->RegisterObjectMethod(
        "color", "string name()", asMETHODPR(QColor, name, () const, QString),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
#endif
}
