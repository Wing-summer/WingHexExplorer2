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

#include "scriptcolor.h"

#include "AngelScript/sdk/add_on/scriptarray/scriptarray.h"
#include "class/angelscripthelper.h"

#include <QColor>

// helper functions with correct signatures for asCALL_CDECL_OBJFIRST
static void Color_DefaultCtor(void *memory) { new (memory) QColor(); }
static void Color_CopyCtor(void *memory, const QColor &other) {
    new (memory) QColor(other);
}
static void Color_StringCtor(void *memory, const QString &name) {
    new (memory) QColor(name);
}
static void Color_RGBCtor(void *memory, int r, int g, int b) {
    new (memory) QColor(r, g, b);
}
static void Color_Dtor(void *memory) {
    reinterpret_cast<QColor *>(memory)->~QColor();
}

static void Color_setRgbFromUint(void *obj, unsigned int rgb) {
    reinterpret_cast<QColor *>(obj)->setRgb(static_cast<QRgb>(rgb));
}
static void Color_setRgb(void *obj, int r, int g, int b) {
    reinterpret_cast<QColor *>(obj)->setRgb(r, g, b);
}
static void Color_getRgb(void *obj, int *r, int *g, int *b) {
    reinterpret_cast<QColor *>(obj)->getRgb(r, g, b);
}
static void Color_setRgbF(void *obj, float r, float g, float b) {
    reinterpret_cast<QColor *>(obj)->setRgbF(r, g, b);
}
static void Color_getRgbF(void *obj, float *r, float *g, float *b) {
    reinterpret_cast<QColor *>(obj)->getRgbF(r, g, b);
}
static void Color_getCmyk(void *obj, int *c, int *m, int *y, int *k) {
    reinterpret_cast<QColor *>(obj)->getCmyk(c, m, y, k);
}
static void Color_getCmykF(void *obj, float *c, float *m, float *y, float *k) {
    reinterpret_cast<QColor *>(obj)->getCmykF(c, m, y, k);
}
static void Color_setCmyk(void *obj, int c, int m, int y, int k) {
    reinterpret_cast<QColor *>(obj)->setCmyk(c, m, y, k);
}
static void Color_setCmykF(void *obj, float c, float m, float y, float k) {
    reinterpret_cast<QColor *>(obj)->setCmykF(c, m, y, k);
}
static void Color_getHsl(void *obj, int *h, int *s, int *l) {
    reinterpret_cast<QColor *>(obj)->getHsl(h, s, l);
}
static void Color_getHslF(void *obj, float *h, float *s, float *l) {
    reinterpret_cast<QColor *>(obj)->getHslF(h, s, l);
}
static void Color_setHsl(void *obj, int h, int s, int l) {
    reinterpret_cast<QColor *>(obj)->setHsl(h, s, l);
}
static void Color_setHslF(void *obj, float h, float s, float l) {
    reinterpret_cast<QColor *>(obj)->setHslF(h, s, l);
}
static void Color_getHsv(void *obj, int *h, int *s, int *v) {
    reinterpret_cast<QColor *>(obj)->getHsv(h, s, v);
}
static void Color_getHsvF(void *obj, float *h, float *s, float *v) {
    reinterpret_cast<QColor *>(obj)->getHsvF(h, s, v);
}
static void Color_setHsv(void *obj, int h, int s, int v) {
    reinterpret_cast<QColor *>(obj)->setHsv(h, s, v);
}
static void Color_setHsvF(void *obj, float h, float s, float v) {
    reinterpret_cast<QColor *>(obj)->setHsvF(h, s, v);
}
static QString Color_name(void *obj) {
    return reinterpret_cast<QColor *>(obj)->name();
}

static void Color_ColorsCtor(void *memory, AngelColor::Colors color) {
    static const QColor table[] = {QColorConstants::Svg::aliceblue,
                                   QColorConstants::Svg::antiquewhite,
                                   QColorConstants::Svg::aqua,
                                   QColorConstants::Svg::aquamarine,
                                   QColorConstants::Svg::azure,
                                   QColorConstants::Svg::beige,
                                   QColorConstants::Svg::bisque,
                                   QColorConstants::Svg::black,
                                   QColorConstants::Svg::blanchedalmond,
                                   QColorConstants::Svg::blue,
                                   QColorConstants::Svg::blueviolet,
                                   QColorConstants::Svg::brown,
                                   QColorConstants::Svg::burlywood,
                                   QColorConstants::Svg::cadetblue,
                                   QColorConstants::Svg::chartreuse,
                                   QColorConstants::Svg::chocolate,
                                   QColorConstants::Svg::coral,
                                   QColorConstants::Svg::cornflowerblue,
                                   QColorConstants::Svg::cornsilk,
                                   QColorConstants::Svg::crimson,
                                   QColorConstants::Svg::cyan,
                                   QColorConstants::Svg::darkblue,
                                   QColorConstants::Svg::darkcyan,
                                   QColorConstants::Svg::darkgoldenrod,
                                   QColorConstants::Svg::darkgray,
                                   QColorConstants::Svg::darkgreen,
                                   QColorConstants::Svg::darkgrey,
                                   QColorConstants::Svg::darkkhaki,
                                   QColorConstants::Svg::darkmagenta,
                                   QColorConstants::Svg::darkolivegreen,
                                   QColorConstants::Svg::darkorange,
                                   QColorConstants::Svg::darkorchid,
                                   QColorConstants::Svg::darkred,
                                   QColorConstants::Svg::darksalmon,
                                   QColorConstants::Svg::darkseagreen,
                                   QColorConstants::Svg::darkslateblue,
                                   QColorConstants::Svg::darkslategray,
                                   QColorConstants::Svg::darkslategrey,
                                   QColorConstants::Svg::darkturquoise,
                                   QColorConstants::Svg::darkviolet,
                                   QColorConstants::Svg::deeppink,
                                   QColorConstants::Svg::deepskyblue,
                                   QColorConstants::Svg::dimgray,
                                   QColorConstants::Svg::dimgrey,
                                   QColorConstants::Svg::dodgerblue,
                                   QColorConstants::Svg::firebrick,
                                   QColorConstants::Svg::floralwhite,
                                   QColorConstants::Svg::forestgreen,
                                   QColorConstants::Svg::fuchsia,
                                   QColorConstants::Svg::gainsboro,
                                   QColorConstants::Svg::ghostwhite,
                                   QColorConstants::Svg::gold,
                                   QColorConstants::Svg::goldenrod,
                                   QColorConstants::Svg::gray,
                                   QColorConstants::Svg::green,
                                   QColorConstants::Svg::greenyellow,
                                   QColorConstants::Svg::grey,
                                   QColorConstants::Svg::honeydew,
                                   QColorConstants::Svg::hotpink,
                                   QColorConstants::Svg::indianred,
                                   QColorConstants::Svg::indigo,
                                   QColorConstants::Svg::ivory,
                                   QColorConstants::Svg::khaki,
                                   QColorConstants::Svg::lavender,
                                   QColorConstants::Svg::lavenderblush,
                                   QColorConstants::Svg::lawngreen,
                                   QColorConstants::Svg::lemonchiffon,
                                   QColorConstants::Svg::lightblue,
                                   QColorConstants::Svg::lightcoral,
                                   QColorConstants::Svg::lightcyan,
                                   QColorConstants::Svg::lightgoldenrodyellow,
                                   QColorConstants::Svg::lightgray,
                                   QColorConstants::Svg::lightgreen,
                                   QColorConstants::Svg::lightgrey,
                                   QColorConstants::Svg::lightpink,
                                   QColorConstants::Svg::lightsalmon,
                                   QColorConstants::Svg::lightseagreen,
                                   QColorConstants::Svg::lightskyblue,
                                   QColorConstants::Svg::lightslategray,
                                   QColorConstants::Svg::lightslategrey,
                                   QColorConstants::Svg::lightsteelblue,
                                   QColorConstants::Svg::lightyellow,
                                   QColorConstants::Svg::lime,
                                   QColorConstants::Svg::limegreen,
                                   QColorConstants::Svg::linen,
                                   QColorConstants::Svg::magenta,
                                   QColorConstants::Svg::maroon,
                                   QColorConstants::Svg::mediumaquamarine,
                                   QColorConstants::Svg::mediumblue,
                                   QColorConstants::Svg::mediumorchid,
                                   QColorConstants::Svg::mediumpurple,
                                   QColorConstants::Svg::mediumseagreen,
                                   QColorConstants::Svg::mediumslateblue,
                                   QColorConstants::Svg::mediumspringgreen,
                                   QColorConstants::Svg::mediumturquoise,
                                   QColorConstants::Svg::mediumvioletred,
                                   QColorConstants::Svg::midnightblue,
                                   QColorConstants::Svg::mintcream,
                                   QColorConstants::Svg::mistyrose,
                                   QColorConstants::Svg::moccasin,
                                   QColorConstants::Svg::navajowhite,
                                   QColorConstants::Svg::navy,
                                   QColorConstants::Svg::oldlace,
                                   QColorConstants::Svg::olive,
                                   QColorConstants::Svg::olivedrab,
                                   QColorConstants::Svg::orange,
                                   QColorConstants::Svg::orangered,
                                   QColorConstants::Svg::orchid,
                                   QColorConstants::Svg::palegoldenrod,
                                   QColorConstants::Svg::palegreen,
                                   QColorConstants::Svg::paleturquoise,
                                   QColorConstants::Svg::palevioletred,
                                   QColorConstants::Svg::papayawhip,
                                   QColorConstants::Svg::peachpuff,
                                   QColorConstants::Svg::peru,
                                   QColorConstants::Svg::pink,
                                   QColorConstants::Svg::plum,
                                   QColorConstants::Svg::powderblue,
                                   QColorConstants::Svg::purple,
                                   QColorConstants::Svg::red,
                                   QColorConstants::Svg::rosybrown,
                                   QColorConstants::Svg::royalblue,
                                   QColorConstants::Svg::saddlebrown,
                                   QColorConstants::Svg::salmon,
                                   QColorConstants::Svg::sandybrown,
                                   QColorConstants::Svg::seagreen,
                                   QColorConstants::Svg::seashell,
                                   QColorConstants::Svg::sienna,
                                   QColorConstants::Svg::silver,
                                   QColorConstants::Svg::skyblue,
                                   QColorConstants::Svg::slateblue,
                                   QColorConstants::Svg::slategray,
                                   QColorConstants::Svg::slategrey,
                                   QColorConstants::Svg::snow,
                                   QColorConstants::Svg::springgreen,
                                   QColorConstants::Svg::steelblue,
                                   QColorConstants::Svg::tan,
                                   QColorConstants::Svg::teal,
                                   QColorConstants::Svg::thistle,
                                   QColorConstants::Svg::tomato,
                                   QColorConstants::Svg::turquoise,
                                   QColorConstants::Svg::violet,
                                   QColorConstants::Svg::wheat,
                                   QColorConstants::Svg::white,
                                   QColorConstants::Svg::whitesmoke,
                                   QColorConstants::Svg::yellow,
                                   QColorConstants::Svg::yellowgreen};

    int idx = static_cast<int>(color);
    const int count = static_cast<int>(sizeof(table) / sizeof(table[0]));
    if (idx >= 0 && idx < count) {
        new (memory) QColor(table[idx]);
    } else {
        new (memory) QColor();
    }
}

static CScriptArray *Color_colorNames_wrap() {
    auto ctx = asGetActiveContext();
    if (ctx) {
        QStringList names = QColor::colorNames();
        auto engine = ctx->GetEngine();
        auto arrType = engine->GetTypeInfoByDecl("array<string>");
        auto *arr = CScriptArray::Create(arrType, asUINT(names.size()));

        for (asUINT i = 0; i < names.size(); ++i) {
            reinterpret_cast<QString *>(arr->At(i))->assign(names.at(i));
        }
        return arr;
    }
    return nullptr;
}

static QColor Color_fromCmyk_wrap(int c, int m, int y, int k) {
    return QColor::fromCmyk(c, m, y, k);
}
static QColor Color_fromCmykF_wrap(float c, float m, float y, float k) {
    return QColor::fromCmykF(c, m, y, k);
}
static QColor Color_fromHsl_wrap(int h, int s, int l) {
    return QColor::fromHsl(h, s, l);
}
static QColor Color_fromHslF_wrap(float h, float s, float l) {
    return QColor::fromHslF(h, s, l);
}
static QColor Color_fromHsv_wrap(int h, int s, int v) {
    return QColor::fromHsv(h, s, v);
}
static QColor Color_fromHsvF_wrap(float h, float s, float v) {
    return QColor::fromHsvF(h, s, v);
}
static QColor Color_fromRgb_uint_wrap(unsigned int rgb) {
    return QColor::fromRgb(static_cast<QRgb>(rgb));
}
static QColor Color_fromRgb_int_wrap(int r, int g, int b) {
    return QColor::fromRgb(r, g, b);
}
static QColor Color_fromRgbF_wrap(float r, float g, float b) {
    return QColor::fromRgbF(r, g, b);
}
static QColor Color_fromRgba_uint_wrap(unsigned int rgba) {
    return QColor::fromRgba(static_cast<QRgb>(rgba));
}
static QColor Color_fromString_wrap(const QString &name) {
    return QColor::fromString(name);
}
static bool Color_isValidColorName_wrap(const QString &name) {
    return QColor::isValidColorName(name);
}

void RegisterColor(asIScriptEngine *engine) {
    int r;
    // register global color
    r = engine->SetDefaultNamespace("color");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    registerAngelType<AngelColor::Colors>(engine, "colors");

    r = engine->RegisterEnum("spec");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "Invalid", QColor::Spec::Invalid);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "Rgb", QColor::Spec::Rgb);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "Hsv", QColor::Spec::Hsv);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "Cmyk", QColor::Spec::Cmyk);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "Hsl", QColor::Spec::Hsl);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("spec", "ExtendedRgb",
                                  QColor::Spec::ExtendedRgb);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    engine->SetDefaultNamespace("");

    // color class
    r = engine->RegisterObjectType("color", sizeof(QColor),
                                   asOBJ_VALUE | asOBJ_APP_CLASS_ALLINTS |
                                       asOBJ_APP_CLASS_UNION |
                                       asGetTypeTraits<QColor>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Constructors
    r = engine->RegisterObjectBehaviour("color", asBEHAVE_CONSTRUCT, "void f()",
                                        asFUNCTION(Color_DefaultCtor),
                                        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(const color &in)",
        asFUNCTION(Color_CopyCtor), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(Color_StringCtor), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(color::colors)",
        asFUNCTION(Color_ColorsCtor), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(int r, int g, int b)",
        asFUNCTION(Color_RGBCtor), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Destructor — MUST call placement destructor, not delete
    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_DESTRUCT, "void f()",
        asFUNCTIONPR(Color_Dtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Assignment: bind operator= which returns QColor&
    r = engine->RegisterObjectMethod(
        "color", "color &opAssign(const color &in)",
        asMETHODPR(QColor, operator=, (const QColor &), QColor &),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "color &opAssign(color::colors)",
                                     asFUNCTION(Color_ColorsCtor),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Methods
    r = engine->RegisterObjectMethod("color", "int black() const",
                                     asMETHODPR(QColor, black, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float blackF() const",
        asMETHODPR(QColor, blackF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int blue() const",
                                     asMETHODPR(QColor, blue, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("color", "float blueF() const",
                                     asMETHODPR(QColor, blueF, () const, float),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int cyan() const",
                                     asMETHODPR(QColor, cyan, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("color", "float cyanF() const",
                                     asMETHODPR(QColor, cyanF, () const, float),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int green() const",
                                     asMETHODPR(QColor, green, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float greenF() const",
        asMETHODPR(QColor, greenF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int magenta() const",
                                     asMETHODPR(QColor, magenta, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float magentaF() const",
        asMETHODPR(QColor, magentaF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int red() const",
                                     asMETHODPR(QColor, red, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("color", "float redF() const",
                                     asMETHODPR(QColor, redF, () const, float),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int yellow() const",
                                     asMETHODPR(QColor, yellow, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float yellowF() const",
        asMETHODPR(QColor, yellowF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int value() const",
                                     asMETHODPR(QColor, value, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float valueF() const",
        asMETHODPR(QColor, valueF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "int saturation() const",
        asMETHODPR(QColor, saturation, () const, int), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float saturationF() const",
        asMETHODPR(QColor, saturationF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "int lightness() const",
        asMETHODPR(QColor, lightness, () const, int), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float lightnessF() const",
        asMETHODPR(QColor, lightnessF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // hue/hsv/hsl variants
    r = engine->RegisterObjectMethod("color", "int hslHue() const",
                                     asMETHODPR(QColor, hslHue, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float hslHueF() const",
        asMETHODPR(QColor, hslHueF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "int hslSaturation() const",
        asMETHODPR(QColor, hslSaturation, () const, int), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float hslSaturationF() const",
        asMETHODPR(QColor, hslSaturationF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int hsvHue() const",
                                     asMETHODPR(QColor, hsvHue, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float hsvHueF() const",
        asMETHODPR(QColor, hsvHueF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "int hsvSaturation() const",
        asMETHODPR(QColor, hsvSaturation, () const, int), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "float hsvSaturationF() const",
        asMETHODPR(QColor, hsvSaturationF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "int hue() const",
                                     asMETHODPR(QColor, hue, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("color", "float hueF() const",
                                     asMETHODPR(QColor, hueF, () const, float),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // validity, spec, conversions and name()
    r = engine->RegisterObjectMethod(
        "color", "bool isValid() const",
        asMETHODPR(QColor, isValid, () const, bool), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "color::spec getSpec() const",
        asMETHODPR(QColor, spec, () const, QColor::Spec), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "color convertTo(color::spec colorSpec) const",
        asMETHODPR(QColor, convertTo, (QColor::Spec) const, QColor),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // to* conversions
    r = engine->RegisterObjectMethod(
        "color", "color toCmyk() const",
        asMETHODPR(QColor, toCmyk, () const, QColor), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "color toExtendedRgb() const",
        asMETHODPR(QColor, toExtendedRgb, () const, QColor), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "color toHsl() const",
        asMETHODPR(QColor, toHsl, () const, QColor), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "color toHsv() const",
        asMETHODPR(QColor, toHsv, () const, QColor), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "color toRgb() const",
        asMETHODPR(QColor, toRgb, () const, QColor), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // name(format)
    r = engine->RegisterObjectMethod("color", "string name() const",
                                     asFunctionPtr(Color_name),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // darker / lighter (with default factors)
    r = engine->RegisterObjectMethod("color",
                                     "color darker(int factor = 200) const",
                                     asMETHOD(QColor, darker), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "color lighter(int factor = 150) const",
        asMETHOD(QColor, lighter), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getRgb(int &out r, int &out g, int &out b) "
        "const",
        asFUNCTION(Color_getRgb), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color",
                                     "void getRgbF(float &out r, float &out g, "
                                     "float &out b) const",
                                     asFUNCTION(Color_getRgbF),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getCmyk(int &out c, int &out m, int &out y, int &out k) const",
        asFUNCTION(Color_getCmyk), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color",
                                     "void getCmykF(float &out c, float &out "
                                     "m, float &out y, float &out k) const",
                                     asFUNCTION(Color_getCmykF),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "void getHsl(int &out h, int &out s, int &out l) const",
        asFUNCTION(Color_getHsl), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color",
                                     "void getHslF(float &out h, float &out s, "
                                     "float &out l) const",
                                     asFUNCTION(Color_getHslF),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "void getHsv(int &out h, int &out s, int &out v) const",
        asFUNCTION(Color_getHsv), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void getHsvF(float &out h, float &out s, float &out v) const",
        asFUNCTION(Color_getHslF), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // RGB setters and other setters
    r = engine->RegisterObjectMethod("color", "void setBlue(int blue)",
                                     asMETHODPR(QColor, setBlue, (int), void),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setBlueF(float blue)",
        asMETHODPR(QColor, setBlueF, (float), void), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "void setGreen(int green)",
                                     asMETHODPR(QColor, setGreen, (int), void),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setGreenF(float green)",
        asMETHODPR(QColor, setGreenF, (float), void), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color", "void setRed(int red)",
                                     asMETHODPR(QColor, setRed, (int), void),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("color", "void setRedF(float red)",
                                     asMETHODPR(QColor, setRedF, (float), void),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setRgb int version
    r = engine->RegisterObjectMethod(
        "color", "void setRgb(int r, int g, int b)", asFUNCTION(Color_setRgb),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // setRgb(QRgb)
    r = engine->RegisterObjectMethod(
        "color", "void setRgb(uint rgb)",
        asFUNCTIONPR(Color_setRgbFromUint, (void *, unsigned int), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setRgbF
    r = engine->RegisterObjectMethod(
        "color", "void setRgbF(float r, float g, float b)",
        asFUNCTION(Color_setRgbF), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setCmyk / setCmykF
    r = engine->RegisterObjectMethod(
        "color", "void setCmyk(int c, int m, int y, int k)",
        asFUNCTION(Color_setCmyk), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setCmykF(float c, float m, float y, float k)",
        asFUNCTION(Color_setCmykF), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setHsl / setHslF
    r = engine->RegisterObjectMethod(
        "color", "void setHsl(int h, int s, int l)", asFUNCTION(Color_setHsl),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setHslF(float h, float s, float l)",
        asFUNCTION(Color_setHslF), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setHsv / setHsvF
    r = engine->RegisterObjectMethod(
        "color", "void setHsv(int h, int s, int v)", asFUNCTION(Color_setHsv),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setHsvF(float h, float s, float v)",
        asFUNCTION(Color_setHsvF), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "bool opEquals(const color &in) const",
        asMETHODPR(QColor, operator==, (const QColor &c) const, bool),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->SetDefaultNamespace("color");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // global functions
    Q_ASSERT(engine->GetTypeInfoByDecl("array<string>"));
    r = engine->RegisterGlobalFunction("array<string>@ colorNames()",
                                       asFUNCTION(Color_colorNames_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromCmyk / fromCmykF
    r = engine->RegisterGlobalFunction(
        "color fromCmyk(int c, int m, int y, int k)",
        asFUNCTION(Color_fromCmyk_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromCmykF(float c, float m, float y, float k)",
        asFUNCTION(Color_fromCmykF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromHsl / fromHslF
    r = engine->RegisterGlobalFunction("color fromHsl(int h, int s, int l)",
                                       asFUNCTION(Color_fromHsl_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromHslF(float h, float s, float l)",
        asFUNCTION(Color_fromHslF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromHsv / fromHsvF
    r = engine->RegisterGlobalFunction("color fromHsv(int h, int s, int v)",
                                       asFUNCTION(Color_fromHsv_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromHsvF(float h, float s, float v)",
        asFUNCTION(Color_fromHsvF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromRgb variants
    r = engine->RegisterGlobalFunction("color fromRgb(uint rgb)",
                                       asFUNCTION(Color_fromRgb_uint_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("color fromRgb(int r, int g, int b)",
                                       asFUNCTION(Color_fromRgb_int_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("color fromRgba(uint rgba)",
                                       asFUNCTION(Color_fromRgba_uint_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromRgbF
    r = engine->RegisterGlobalFunction(
        "color fromRgbF(float r, float g, float b)",
        asFUNCTION(Color_fromRgbF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromString
    r = engine->RegisterGlobalFunction(
        "color fromString(const string &in name)",
        asFUNCTION(Color_fromString_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // isValidColorName
    r = engine->RegisterGlobalFunction(
        "bool isValidColorName(const string &in name)",
        asFUNCTION(Color_isValidColorName_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    engine->SetDefaultNamespace("");
}
