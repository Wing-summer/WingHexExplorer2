#include "qasparser.h"

#include "AngelScript/sdk/angelscript/source/as_builder.h"
#include "AngelScript/sdk/angelscript/source/as_parser.h"
#include "codemodel/qcodemodel.h"

QAsParser::QAsParser(asIScriptEngine *engine) : asBuilder(), _engine(engine) {}

QAsParser::~QAsParser() {}

bool QAsParser::parse(const QString &filename) {
    if (StartNewModule(_engine, "as_parser") != 0) {
        return false;
    }

    auto ret = LoadScriptSection(filename);
    if (ret != 0) {
        return false;
    }

    ClearAll();

    auto mod = dynamic_cast<asCModule *>(GetModule());
    asCParser parser(mod->m_builder);

    m_code.reset(new asCScriptCode);
    m_code->SetCode("as_parser", modifiedScript.data(), true);

    parser.ParseScript(m_code.get());

    auto nodes = parser.GetScriptNode();

    QCodeModel model;

    return true;
}
