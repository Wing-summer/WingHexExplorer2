#ifndef CLICKCALLBACK_H
#define CLICKCALLBACK_H

#include "angelscript.h"
#include "plugin/iwingplugin.h"

class ClickCallBack {
public:
    ClickCallBack() {}

    ClickCallBack(const WingHex::WingPlugin::DataVisual::ClickedCallBack &b)
        : _call(b) {}

    explicit ClickCallBack(asIScriptEngine *engine, asIScriptFunction *func)
        : _engine(engine), _func(func) {
        if (_engine && _func) {
            _func->AddRef();
        }
    }

    ~ClickCallBack() {
        if (_func) {
            _func->Release();
        }
    }

public:
    ClickCallBack &
    operator=(const WingHex::WingPlugin::DataVisual::ClickedCallBack &_Right) {
        *this = ClickCallBack(_Right);
        return *this;
    }

    ClickCallBack &
    operator=(WingHex::WingPlugin::DataVisual::ClickedCallBack &&_Right) {
        *this = ClickCallBack(_Right);
        return *this;
    }

public:
    explicit operator bool() const noexcept {
        return _call || (_engine && _func);
    }

    void operator()(const QModelIndex &index) {
        if (_call) {
            _call(index);
        } else {
            if (_engine && _func) {
                auto ctx = _engine->CreateContext();
                if (ctx) {
                    auto r = ctx->Prepare(_func);
                    if (r >= 0) {
                        auto idx = index;
                        ctx->SetArgObject(0, &idx);
                        ctx->Execute();
                    }
                    ctx->Release();
                }
            }
        }
    }

private:
    WingHex::WingPlugin::DataVisual::ClickedCallBack _call;
    asIScriptEngine *_engine = nullptr;
    asIScriptFunction *_func = nullptr;
};

#endif // CLICKCALLBACK_H
