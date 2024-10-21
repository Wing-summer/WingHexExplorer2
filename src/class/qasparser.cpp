#include "qasparser.h"

#include "AngelScript/sdk/angelscript/source/as_builder.h"
#include "AngelScript/sdk/angelscript/source/as_parser.h"
#include "codemodel/qcodenode.h"

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
    Q_ASSERT(mod);
    asCParser parser(mod->m_builder);

    m_code.reset(new asCScriptCode);
    m_code->SetCode("as_parser", modifiedScript.data(), true);

    parser.ParseScript(m_code.get());

    auto pnodes = parser.GetScriptNode();

    QList<QCodeNode *> qnodes;

    do {
        auto node = new QCodeNode;
        node->line = pnodes->tokenPos;

        auto &cs = node->children;
        for (auto p = pnodes->firstChild; p != pnodes->lastChild; p++) {
        }

    } while (pnodes->next != nullptr);

    return true;
}
