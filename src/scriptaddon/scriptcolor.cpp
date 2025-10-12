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
static void Color_RGBACtor(void *memory, int r, int g, int b, int a) {
    new (memory) QColor(r, g, b, a);
}
static void Color_Dtor(void *memory) {
    reinterpret_cast<QColor *>(memory)->~QColor();
}

// wrappers for setRgb(QRgb) and setRgba(QRgb) and setRgba64(QRgba64)
// map script-side uint/uint64 -> C++ QRgb/QRgba64
static void Color_setRgbFromUint(void *obj, unsigned int rgb) {
    reinterpret_cast<QColor *>(obj)->setRgb(static_cast<QRgb>(rgb));
}
static void Color_setRgbaFromUint(void *obj, unsigned int rgba) {
    reinterpret_cast<QColor *>(obj)->setRgba(static_cast<QRgb>(rgba));
}
static void Color_setRgba64FromU64(void *obj, quint64 rgba64) {
    reinterpret_cast<QColor *>(obj)->setRgba64(QRgba64::fromRgba64(rgba64));
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
            auto s = names.at(i);
            arr->SetValue(i, &s);
        }
        return arr;
    }
    return nullptr;
}

static QColor Color_fromCmyk_wrap(int c, int m, int y, int k, int a) {
    return QColor::fromCmyk(c, m, y, k, a);
}
static QColor Color_fromCmykF_wrap(float c, float m, float y, float k,
                                   float a) {
    return QColor::fromCmykF(c, m, y, k, a);
}
static QColor Color_fromHsl_wrap(int h, int s, int l, int a) {
    return QColor::fromHsl(h, s, l, a);
}
static QColor Color_fromHslF_wrap(float h, float s, float l, float a) {
    return QColor::fromHslF(h, s, l, a);
}
static QColor Color_fromHsv_wrap(int h, int s, int v, int a) {
    return QColor::fromHsv(h, s, v, a);
}
static QColor Color_fromHsvF_wrap(float h, float s, float v, float a) {
    return QColor::fromHsvF(h, s, v, a);
}
static QColor Color_fromRgb_uint_wrap(unsigned int rgb) {
    return QColor::fromRgb(static_cast<QRgb>(rgb));
}
static QColor Color_fromRgb_int_wrap(int r, int g, int b, int a) {
    return QColor::fromRgb(r, g, b, a);
}
static QColor Color_fromRgba64_u64_wrap(quint64 rgba64) {
    return QColor::fromRgba64(QRgba64::fromRgba64(rgba64));
}
static QColor Color_fromRgba64_ush_wrap(unsigned short r, unsigned short g,
                                        unsigned short b, unsigned short a) {
    return QColor::fromRgba64(r, g, b, a);
}
static QColor Color_fromRgbF_wrap(float r, float g, float b, float a) {
    return QColor::fromRgbF(r, g, b, a);
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

    r = engine->RegisterEnum("format");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("format", "HexArgb",
                                  QColor::NameFormat::HexArgb);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterEnumValue("format", "HexRgb",
                                  QColor::NameFormat::HexRgb);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

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
    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f()",
        asFUNCTIONPR(Color_DefaultCtor, (void *), void), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(const color &in)",
        asFUNCTIONPR(Color_CopyCtor, (void *, const QColor &), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTIONPR(Color_StringCtor, (void *, const QString &), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(color::colors)",
        asFUNCTIONPR(Color_ColorsCtor, (void *, AngelColor::Colors), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "color", asBEHAVE_CONSTRUCT, "void f(int r, int g, int b, int a = 255)",
        asFUNCTIONPR(Color_RGBACtor, (void *, int, int, int, int), void),
        asCALL_CDECL_OBJFIRST);
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

    r = engine->RegisterObjectMethod(
        "color", "color &opAssign(color::colors)",
        asFUNCTIONPR(Color_ColorsCtor, (void *, AngelColor::Colors), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // Methods
    // simple getters (ints / floats)
    r = engine->RegisterObjectMethod("color", "int alpha() const",
                                     asMETHODPR(QColor, alpha, () const, int),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color", "float alphaF() const",
        asMETHODPR(QColor, alphaF, () const, float), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

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
    r = engine->RegisterObjectMethod(
        "color",
        "string name(color::format format = color::format::HexRgb) const",
        asMETHODPR(QColor, name, (QColor::NameFormat) const, QString),
        asCALL_THISCALL);
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
        "void getRgb(int &out r, int &out g, int &out b, int &out a = void) "
        "const",
        asMETHOD(QColor, getRgb), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color",
                                     "void getRgbF(float &out r, float &out g, "
                                     "float &out b, float &out a = void) "
                                     "const",
                                     asMETHOD(QColor, getRgbF),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getCmyk(int &out c, int &out m, int &out y, int &out k, int &out "
        "a = void) const",
        asMETHOD(QColor, getCmyk), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getCmykF(float &out c, float &out m, float &out y, float &out k, "
        "float &out a = void) const",
        asMETHOD(QColor, getCmykF), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getHsl(int &out h, int &out s, int &out l, int &out a) const",
        asMETHOD(QColor, getHsl), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("color",
                                     "void getHslF(float &out h, float &out s, "
                                     "float &out l, float &out a = void) "
                                     "const",
                                     asMETHOD(QColor, getHslF),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "color",
        "void getHsv(int &out h, int &out s, int &out v, int &out a) const",
        asMETHOD(QColor, getHsv), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color",
        "void getHsvF(float &out h, float &out s, float &out v, float &out a) "
        "const",
        asMETHOD(QColor, getHslF), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // RGB setters and other setters
    r = engine->RegisterObjectMethod("color", "void setAlpha(int alpha)",
                                     asMETHODPR(QColor, setAlpha, (int), void),
                                     asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setAlphaF(float alpha)",
        asMETHODPR(QColor, setAlphaF, (float), void), asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

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
        "color", "void setRgb(int r, int g, int b, int a = 255)",
        asMETHODPR(QColor, setRgb, (int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // setRgb(QRgb)
    r = engine->RegisterObjectMethod(
        "color", "void setRgb(uint rgb)",
        asFUNCTIONPR(Color_setRgbFromUint, (void *, unsigned int), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // setRgba(QRgb)
    r = engine->RegisterObjectMethod(
        "color", "void setRgba(uint rgba)",
        asFUNCTIONPR(Color_setRgbaFromUint, (void *, unsigned int), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    // setRgba64(QRgba64)
    r = engine->RegisterObjectMethod(
        "color", "void setRgba64(uint64 rgba64)",
        asFUNCTIONPR(Color_setRgba64FromU64, (void *, quint64), void),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setRgbF
    r = engine->RegisterObjectMethod(
        "color", "void setRgbF(float r, float g, float b, float a = 1.0)",
        asMETHODPR(QColor, setRgbF, (float, float, float, float), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setCmyk / setCmykF
    r = engine->RegisterObjectMethod(
        "color", "void setCmyk(int c, int m, int y, int k, int a = 255)",
        asMETHODPR(QColor, setCmyk, (int, int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color",
        "void setCmykF(float c, float m, float y, float k, float a = 1.0)",
        asMETHODPR(QColor, setCmykF, (float, float, float, float, float), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setHsl / setHslF
    r = engine->RegisterObjectMethod(
        "color", "void setHsl(int h, int s, int l, int a = 255)",
        asMETHODPR(QColor, setHsl, (int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setHslF(float h, float s, float l, float a = 1.0)",
        asMETHODPR(QColor, setHslF, (float, float, float, float), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // setHsv / setHsvF
    r = engine->RegisterObjectMethod(
        "color", "void setHsv(int h, int s, int v, int a = 255)",
        asMETHODPR(QColor, setHsv, (int, int, int, int), void),
        asCALL_THISCALL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "color", "void setHsvF(float h, float s, float v, float a = 1.0)",
        asMETHODPR(QColor, setHsvF, (float, float, float, float), void),
        asCALL_THISCALL);
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
        "color fromCmyk(int c, int m, int y, int k, int a = 255)",
        asFUNCTION(Color_fromCmyk_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromCmykF(float c, float m, float y, float k, float a = 1.0)",
        asFUNCTION(Color_fromCmykF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromHsl / fromHslF
    r = engine->RegisterGlobalFunction(
        "color fromHsl(int h, int s, int l, int a = 255)",
        asFUNCTION(Color_fromHsl_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromHslF(float h, float s, float l, float a = 1.0)",
        asFUNCTION(Color_fromHslF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromHsv / fromHsvF
    r = engine->RegisterGlobalFunction(
        "color fromHsv(int h, int s, int v, int a = 255)",
        asFUNCTION(Color_fromHsv_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromHsvF(float h, float s, float v, float a = 1.0)",
        asFUNCTION(Color_fromHsvF_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromRgb variants
    r = engine->RegisterGlobalFunction("color fromRgb(uint rgb)",
                                       asFUNCTION(Color_fromRgb_uint_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromRgb(int r, int g, int b, int a = 255)",
        asFUNCTION(Color_fromRgb_int_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction("color fromRgba(uint rgba)",
                                       asFUNCTION(Color_fromRgba_uint_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromRgba64 variants
    r = engine->RegisterGlobalFunction("color fromRgba64(uint64 rgba64)",
                                       asFUNCTION(Color_fromRgba64_u64_wrap),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "color fromRgba64(uint16 r, uint16 g, uint16 b, uint16 a = 65535)",
        asFUNCTION(Color_fromRgba64_ush_wrap), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // fromRgbF
    r = engine->RegisterGlobalFunction(
        "color fromRgbF(float r, float g, float b, float a = 1.0)",
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
