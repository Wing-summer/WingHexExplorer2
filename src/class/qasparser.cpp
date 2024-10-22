#include "qasparser.h"

#include "AngelScript/sdk/angelscript/source/as_builder.h"
#include "AngelScript/sdk/angelscript/source/as_parser.h"
#include "codemodel/qcodemodel.h"
#include "codemodel/qcodenode.h"

QAsParser::QAsParser(asIScriptEngine *engine) : asBuilder(), _engine(engine) {}

QAsParser::~QAsParser() {}

QCodeNode *QAsParser::asNode2CodeNode(asCScriptNode *node) {
    if (node == nullptr) {
        return nullptr;
    }

    auto n = new QCodeNode;
    n->model = nullptr;

    switch (node->nodeType) {
    case snUndefined:
    case snScript:
    case snFunction:
    case snConstant:
    case snDataType:
    case snIdentifier:
    case snParameterList:
    case snStatementBlock:
    case snDeclaration:
    case snExpressionStatement:
    case snIf:
    case snFor:
    case snWhile:
    case snReturn:
    case snExpression:
    case snExprTerm:
    case snFunctionCall:
    case snConstructCall:
    case snArgList:
    case snExprPreOp:
    case snExprPostOp:
    case snExprOperator:
    case snExprValue:
    case snBreak:
    case snContinue:
    case snDoWhile:
    case snAssignment:
    case snCondition:
    case snSwitch:
    case snCase:
    case snImport:
    case snClass:
    case snInitList:
    case snInterface:
    case snEnum:
    case snTypedef:
    case snCast:
    case snVariableAccess:
    case snFuncDef:
    case snVirtualProperty:
    case snNamespace:
    case snMixin:
    case snListPattern:
    case snNamedArgument:
    case snScope:
    case snTryCatch:
        break;
    }

    int row = -1, col = -1;
    m_code->ConvertPosToRowCol(node->tokenPos, &row, &col);
    n->line = row;

    return n;
}

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
        auto node = asNode2CodeNode(pnodes);

        auto p = pnodes->firstChild;
        while (p) {
            auto cnode = asNode2CodeNode(pnodes);
            cnode->parent = node;
            node->children.append(cnode);

            p = pnodes->next;
        }

        qnodes.append(node);
        pnodes = pnodes->next;
    } while (pnodes != nullptr);

    return true;
}
