#include "scriptoptbox.h"

#include "define.h"
#include "dialog/framelessdialogbase.h"
#include "scriptdatetime.h"
#include "scriptqdictionary.h"

#include <QHash>
#include <QString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QTimeEdit>
#include <QVBoxLayout>

class OptBox {
private:
    enum class ControlType {
        Unknown,      // ???
        Bool,         // checkbox
        BoolItems,    // multi-checkbox
        BoolOptItems, // multi-optionbox
        Int,          // spinbox
        Double,       // doublespinbox
        String,       // lineedit
        StaticString, // label
        StringItems,  // combobox
        Time,         // timeedit
        Date,         // dateedit
        DateTime,     // datatimeedit
    };

    enum class EntryType { Form, Group, Space };

    static constexpr auto PROPERTY_DEFAULT = "__DEFAULT__";
    static constexpr auto PROPERTY_VALUE = "__VALUE__";

private:
    struct WidgetInfo {
        QWidget *widget = nullptr;
        QString title;
        int stretch = 0;
        Qt::Alignment align = {};
    };

public:
    OptBox() : m_ref(1) {}

    OptBox(const QString &json)
        : OptBox(QJsonDocument::fromJson(json.toUtf8())) {}

    OptBox(const QJsonDocument &doc) : OptBox() { buildUpForm(doc); }

    ~OptBox() {
        if (_dialog) {
            delete _dialog;
        }
    }

    void AddRef() { ++m_ref; }
    void Release() {
        if (--m_ref == 0) {
            this->~OptBox();
            asFreeMem(const_cast<OptBox *>(this));
        }
    }

    CScriptDictionary *exec() const {
        if (_dialog == nullptr) {
            return nullptr;
        }

        auto ctx = asGetActiveContext();
        if (ctx == nullptr) {
            return nullptr;
        }

        for (auto &&[w, t] : _data.asKeyValueRange()) {
            auto p = w->property(PROPERTY_DEFAULT);
            if (p.isNull()) {
                continue;
            }
            switch (t) {
            case ControlType::Unknown:
            case ControlType::BoolItems:
                // should not go there
                break;
            case ControlType::Bool: {
                w->setProperty("checked", p);
            } break;
            case ControlType::BoolOptItems: {
                auto rb = w->findChild<QRadioButton *>(p.toString());
                if (rb) {
                    rb->setChecked(true);
                }
            } break;
            case ControlType::Int: {
                w->setProperty("value", p);
            } break;
            case ControlType::Double: {
                w->setProperty("value", p);
            } break;
            case ControlType::String: {
                w->setProperty("text", p);
            } break;
            case ControlType::StaticString: {
                w->setProperty("text", p);
            } break;
            case ControlType::StringItems: {
                auto cb = qobject_cast<QComboBox *>(w);
                if (cb) {
                    auto total = cb->count();
                    for (int i = 0; i < total; ++i) {
                        auto item = cb->itemData(i);
                        if (item == p) {
                            cb->setCurrentIndex(i);
                            break;
                        }
                    }
                }
            } break;
            case ControlType::Time: {
                w->setProperty("time", p);
            } break;
            case ControlType::Date: {
                w->setProperty("date", p);
            } break;
            case ControlType::DateTime: {
                w->setProperty("dateTime", p);
            } break;
            }
        }

        auto dic = CScriptDictionary::Create(ctx->GetEngine());

        if (_dialog->exec()) {
            for (auto &&[w, t] : _data.asKeyValueRange()) {
                switch (t) {
                case ControlType::Unknown:
                case ControlType::BoolItems:
                    // should not go there
                    break;
                case ControlType::Bool: {
                    bool b = w->property("checked").toBool();
                    dic->Set(w->objectName(), &b, asTYPEID_BOOL);
                } break;
                case ControlType::BoolOptItems: {
                    bool b = w->property(PROPERTY_VALUE).toBool();
                    dic->Set(w->objectName(), &b, asTYPEID_BOOL);
                } break;
                case ControlType::Int: {
                    int v = w->property("value").toInt();
                    dic->Set(w->objectName(), &v, asTYPEID_INT32);
                } break;
                case ControlType::Double: {
                    double v = w->property("value").toDouble();
                    dic->Set(w->objectName(), v);
                } break;
                case ControlType::String: {
                    QString v = w->property("text").toString();
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_StringTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                case ControlType::StaticString: {
                    QString v = w->property("text").toString();
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_StringTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                case ControlType::StringItems: {
                    QString v = w->property("currentData").toString();
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_StringTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                case ControlType::Time: {
                    QTime v = w->property("time").toTime();
                    auto d = QDate::currentDate();
                    CDateTime time(d.year(), d.month(), d.day(), v.hour(),
                                   v.minute(), v.second());
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_DateTimeTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                case ControlType::Date: {
                    QDate v = w->property("date").toDate();
                    auto t = QTime::currentTime();
                    CDateTime time(v.year(), v.month(), v.day(), t.hour(),
                                   t.minute(), t.second());
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_DateTimeTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                case ControlType::DateTime: {
                    QDateTime v = w->property("dateTime").toDateTime();
                    auto d = v.date();
                    auto t = v.time();
                    CDateTime time(d.year(), d.month(), d.day(), t.hour(),
                                   t.minute(), t.second());
                    auto engine = ctx->GetEngine();
                    auto type = static_cast<asITypeInfo *>(engine->GetUserData(
                        AsUserDataType::UserData_DateTimeTypeInfo));
                    dic->Set(w->objectName(), &v, type->GetTypeId());
                } break;
                }
            }
        }

        return dic;
    }

private:
    Qt::Alignment toAlignment(const QString &str) {
        static QHash<QString, Qt::Alignment> _maps{
            {"center", Qt::AlignCenter},   {"vcenter", Qt::AlignVCenter},
            {"hcenter", Qt::AlignHCenter}, {"left", Qt::AlignLeft},
            {"right", Qt::AlignRight},     {"bottom", Qt::AlignBottom},
            {"top", Qt::AlignTop},         {"bottom", Qt::AlignBottom}};
        return _maps.value(str);
    }

    ControlType toCtlType(const QString &str) {
        static QHash<QString, ControlType> _maps{
            {"bool", ControlType::Bool},
            {"bools", ControlType::BoolItems},
            {"boolopt", ControlType::BoolOptItems},
            {"int", ControlType::Int},
            {"double", ControlType::Double},
            {"string", ControlType::String},
            {"text", ControlType::StaticString},
            {"strings", ControlType::StringItems},
            {"time", ControlType::Time},
            {"date", ControlType::Date},
            {"datetime", ControlType::DateTime}};
        return _maps.value(str, ControlType::Unknown);
    }

private:
    void buildUpForm(const QJsonDocument &doc) {
        _dialog = new FramelessDialogBase;
        auto w = new QWidget;
        auto layout = new QVBoxLayout(w);
        layout->setParent(w);
        auto jobj = doc.object();
        _usedObjNames.clear();

        for (auto &&[key, value] : jobj.asKeyValueRange()) {
            if (key == QLatin1String("title")) {
                w->setWindowTitle(value.toString());
            } else if (key == QLatin1String("fixedWidth")) {
                w->setFixedWidth(value.toInt());
            } else if (key == QLatin1String("fixedHeight")) {
                w->setFixedHeight(value.toInt());
            } else if (key == QLatin1String("minWidth")) {
                w->setMinimumWidth(value.toInt());
            } else if (key == QLatin1String("minHeight")) {
                w->setMinimumHeight(value.toInt());
            } else if (key == QLatin1String("maxWidth")) {
                w->setMaximumWidth(value.toInt());
            } else if (key == QLatin1String("maxHeight")) {
                w->setMaximumHeight(value.toInt());
            } else if (key == QLatin1String("icon")) {
                w->setWindowIcon(QIcon(value.toString()));
            } else if (key == QLatin1String("margins")) {
                auto mstr = value.toString();
                applyLayoutMargins(layout, mstr);
            } else if (key == QLatin1String("spacing")) {
                auto spacing = value.toInt();
                layout->setSpacing(spacing);
            } else if (key == QLatin1String("data")) {
                // ok, all the entries
                auto entries = value.toArray();
                for (auto p = entries.begin(); p != entries.end(); p++) {
                    auto obj = p->toObject();
                    auto type = obj.value("type").toString();

                    if (type == QLatin1String("form")) {
                        auto r = createFormLayout(obj);
                        layout->addLayout(r.first, r.second);
                    } else if (type == QLatin1String("group")) {
                        auto g = new QGroupBox;
                        g->setTitle(obj.value("title").toString());
                        auto r = createFormLayout(obj);
                        g->setLayout(r.first);
                        r.first->setParent(g);
                        layout->addWidget(g, r.second);
                    } else if (type == QLatin1String("space")) {
                        auto size = obj.value("size").toInt();
                        layout->addSpacing(size);
                    } else {
                        qWarning("[optbox::buildUpForm] Invalid entry '%s' for "
                                 "'data'",
                                 qUtf8Printable(type));
                    }
                }
            } else {
                qWarning("[optbox::buildUpForm] Unsupported property '%s'",
                         qUtf8Printable(key.toString()));
            }
        }

        auto btnbox = new QDialogButtonBox(w);
        btnbox->setStandardButtons(QDialogButtonBox::Ok |
                                   QDialogButtonBox::Cancel);
        QObject::connect(btnbox, &QDialogButtonBox::rejected, _dialog,
                         &FramelessDialogBase::reject);
        QObject::connect(btnbox, &QDialogButtonBox::accepted, _dialog,
                         &FramelessDialogBase::accept);
        layout->addWidget(btnbox);

        _dialog->buildUpContent(w);
    }

    QPair<QFormLayout *, int> createFormLayout(const QJsonObject &obj) {
        auto f = new QFormLayout;
        auto data = obj.value("data").toArray();
        auto stretch = obj.value("stretch").toInt();
        auto margins = obj.value("margins").toString();
        if (!margins.isNull()) {
            applyLayoutMargins(f, margins);
        }
        auto hspacing = obj.value("hspacing").toInt(6);
        auto vspacing = obj.value("vspacing").toInt(6);
        f->setHorizontalSpacing(hspacing);
        f->setVerticalSpacing(vspacing);

        for (auto p = data.begin(); p != data.end(); ++p) {
            if (!p->isObject()) {
                qWarning("[optbox::createFormLayout] Invalid data");
                continue;
            }
            auto obj = p->toObject();
            auto type = toCtlType(obj.value("type").toString());
            if (type == ControlType::Unknown) {
                qWarning("[optbox::createFormLayout] Invalid item type");
                continue;
            }
            auto ptr = createWidget(obj, type);
            if (ptr.widget && type != ControlType::Unknown) {
                f->addRow(ptr.title, ptr.widget);
                _usedObjNames.append(ptr.widget->objectName());
            }
        }
        return qMakePair(f, stretch);
    }

    void applyLayoutMargins(QLayout *layout, const QString &data) {
        int left = -1, top = -1, right = -1, bottom = -1;
        QTextStream s(&const_cast<QString &>(data));
        s >> left >> top >> right >> bottom;
        if (left < 0 || top < 0 || right < 0 || bottom < 0) {
            qWarning("[optbox::applyLayoutMargins] Invalid margins '%s'",
                     qUtf8Printable(data));
            return;
        }
        layout->setContentsMargins(left, top, right, bottom);
    }

    WidgetInfo createWidget(const QJsonObject &obj, ControlType type) {
        if (obj.isEmpty()) {
            return {};
        }

        auto objname = obj.value("name").toString();
        if (objname.isEmpty()) {
            qWarning(
                "[optbox::createWidget] Cannot create widget without a name.");
            return {};
        }

        if (_usedObjNames.contains(objname)) {
            qWarning("[optbox::createWidget] Cannot create widget with a used "
                     "name.");
            return {};
        }

        WidgetInfo ret;
        ret.title = obj.value("title").toString(objname);
        ret.stretch = obj.value("stretch").toInt();
        ret.align = toAlignment(obj.value("align").toString());

        switch (type) {
        case ControlType::Bool: {
            auto cb = new QCheckBox;
            cb->setText(obj.value("text").toString());
            auto dv = obj.value("value").toBool();
            cb->setProperty(PROPERTY_DEFAULT, dv);
            cb->setChecked(dv);
            ret.widget = cb;
            _data.insert(cb, type);
        } break;
        case ControlType::BoolItems: {
            auto w = new QWidget;
            auto layout = new QHBoxLayout(w);
            layout->setParent(w);
            auto data = obj.value("data").toArray();
            for (auto p = data.begin(); p != data.end(); p++) {
                if (!p->isObject()) {
                    qWarning("[optbox::createWidget] Invalid data for 'bools'");
                    continue;
                }
                auto obj = p->toObject();
                auto cb = new QCheckBox;
                cb->setText(obj.value("text").toString());
                auto dv = obj.value("value").toBool();
                cb->setProperty(PROPERTY_DEFAULT, dv);
                cb->setObjectName(obj.value("name").toString());
                cb->setToolTip(obj.value("tooltip").toString());
                cb->setChecked(dv);
                _data.insert(cb, ControlType::Bool);
                layout->addWidget(cb);
            }
            ret.widget = w;
        } break;
        case ControlType::BoolOptItems: {
            auto w = new QWidget;
            auto layout = QHBoxLayout(w);
            auto dv = obj.value("value").toString();
            w->setProperty(PROPERTY_DEFAULT, dv);
            auto group = QButtonGroup(w);
            group.setExclusive(true);
            auto data = obj.value("data").toArray();
            for (auto p = data.begin(); p != data.end(); p++) {
                if (!p->isObject()) {
                    qWarning(
                        "[optbox::createWidget] Invalid data for 'boolopt'");
                    continue;
                }
                auto obj = p->toObject();
                auto rb = new QRadioButton;
                rb->setText(obj.value("text").toString());
                auto name = obj.value("name").toString();
                rb->setObjectName(name);
                rb->setToolTip(obj.value("tooltip").toString());
                if (name == dv) {
                    rb->setChecked(true);
                }
                QObject::connect(
                    rb, &QRadioButton::toggled, w, [rb, w](bool b) {
                        if (b) {
                            w->setProperty(PROPERTY_VALUE, rb->objectName());
                        }
                    });
                group.addButton(rb);
            }
            ret.widget = w;
            _data.insert(w, type);
        } break;
        case ControlType::Int: {
            auto sb = new QSpinBox;
            auto dv = obj.value("value").toInt(0);
            sb->setProperty(PROPERTY_DEFAULT, dv);
            sb->setValue(dv);
            sb->setRange(obj.value("min").toInt(0),
                         obj.value("max").toInt(100));
            sb->setSingleStep(obj.value("step").toInt(1));
            sb->setDisplayIntegerBase(obj.value("base").toInt(10));
            sb->setPrefix(obj.value("prefix").toString());
            sb->setSuffix(obj.value("suffix").toString());
            ret.widget = sb;
            _data.insert(sb, type);
        } break;
        case ControlType::Double: {
            auto sb = new QDoubleSpinBox;
            auto dv = obj.value("value").toDouble(0);
            sb->setProperty(PROPERTY_DEFAULT, dv);
            sb->setValue(dv);
            sb->setRange(obj.value("min").toDouble(0),
                         obj.value("max").toDouble(100));
            sb->setSingleStep(obj.value("step").toDouble(1));
            sb->setDecimals(obj.value("decimals").toInt(2));
            sb->setPrefix(obj.value("prefix").toString());
            sb->setSuffix(obj.value("suffix").toString());
            ret.widget = sb;
            _data.insert(sb, type);
        } break;
        case ControlType::String: {
            auto le = new QLineEdit;
            auto dv = obj.value("value").toString();
            le->setProperty(PROPERTY_DEFAULT, dv);
            le->setText(dv);
            le->setAlignment(toAlignment(obj.value("align").toString()));
            le->setPlaceholderText(obj.value("placeholder").toString());
            le->setMaxLength(
                obj.value("maxLength").toInt(std::numeric_limits<int>::max()));
            ret.widget = le;
            _data.insert(le, type);
        } break;
        case ControlType::StaticString: {
            auto lbl = new QLabel;
            lbl->setText(obj.value("value").toString());
            lbl->setAlignment(toAlignment(obj.value("align").toString()));
            lbl->setWordWrap(obj.value("wordWrap").toBool());
            lbl->setOpenExternalLinks(obj.value("isLink").toBool());
            ret.widget = lbl;
        } break;
        case ControlType::StringItems: {
            auto cb = new QComboBox;
            auto dv = obj.value("value").toString();
            auto data = obj.value("data").toArray();

            cb->setProperty(PROPERTY_DEFAULT, dv);
            for (auto p = data.begin(); p != data.end(); p++) {
                if (p->isObject()) {
                    auto obj = p->toObject();
                    auto value = obj.value("value").toString();
                    auto tag = obj.value("tag").toString();
                    cb->addItem(value, tag);
                } else if (p->isString()) {
                    cb->addItem(p->toString());
                }
            }
            cb->setCurrentText(dv);

            ret.widget = cb;
            _data.insert(cb, type);
        } break;
        case ControlType::Time: {
            auto te = new QTimeEdit;
            auto dv = QTime::fromString(obj.value("value").toString());
            te->setProperty(PROPERTY_DEFAULT, dv);
            te->setTime(dv);
            ret.widget = te;
            _data.insert(te, type);
        } break;
        case ControlType::Date: {
            auto de = new QDateEdit;
            auto dv = QDate::fromString(obj.value("value").toString());
            de->setProperty(PROPERTY_DEFAULT, dv);
            de->setDate(dv);
            ret.widget = de;
            _data.insert(de, type);
        } break;
        case ControlType::DateTime: {
            auto dte = new QDateTimeEdit;
            auto dv = QDateTime::fromString(obj.value("value").toString());
            dte->setProperty(PROPERTY_DEFAULT, dv);
            dte->setDateTime(dv);
            ret.widget = dte;
            _data.insert(dte, type);
        } break;
        case ControlType::Unknown:
            return {};
        }

        auto w = ret.widget;
        w->setToolTip(obj.value("tooltip").toString());
        w->setObjectName(objname);

        return ret;
    }

private:
    int m_ref;
    FramelessDialogBase *_dialog = nullptr;
    QHash<QWidget *, ControlType> _data;
    QStringList _usedObjNames;
};

void ScriptOptBoxFactory_Generic(asIScriptGeneric *gen) {
    auto param = static_cast<QString *>(gen->GetArgObject(0));
    auto mem = static_cast<OptBox *>(asAllocMem(sizeof(OptBox)));
    *(OptBox **)gen->GetAddressOfReturnLocation() = new (mem) OptBox(*param);
}

void ScriptOptBoxFactory_Json_Generic(asIScriptGeneric *gen) {
    auto param = static_cast<QJsonDocument *>(gen->GetArgObject(0));
    auto mem = static_cast<OptBox *>(asAllocMem(sizeof(OptBox)));
    *(OptBox **)gen->GetAddressOfReturnLocation() = new (mem) OptBox(*param);
}

void RegisterOptBox(asIScriptEngine *engine) {
    auto r = engine->RegisterObjectType("optbox", sizeof(OptBox), asOBJ_REF);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("optbox", asBEHAVE_ADDREF, "void f()",
                                        asMETHOD(OptBox, AddRef),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour("optbox", asBEHAVE_RELEASE, "void f()",
                                        asMETHOD(OptBox, Release),
                                        asCALL_THISCALL);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "optbox", asBEHAVE_FACTORY, "optbox@ f(const string&in)",
        asFUNCTION(ScriptOptBoxFactory_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterObjectBehaviour(
        "optbox", asBEHAVE_FACTORY, "optbox@ f(const json::document&in)",
        asFUNCTION(ScriptOptBoxFactory_Json_Generic), asCALL_GENERIC);
    ASSERT(r >= 0);
    r = engine->RegisterObjectMethod("optbox", "dictionary@ exec() const",
                                     asMETHOD(OptBox, exec), asCALL_THISCALL);
    ASSERT(r >= 0);
}
