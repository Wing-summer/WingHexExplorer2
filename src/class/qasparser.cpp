#include "qasparser.h"

#include "AngelScript/sdk/angelscript/source/as_builder.h"
#include "AngelScript/sdk/angelscript/source/as_parser.h"
#include "codemodel/qcodemodel.h"
#include "codemodel/qcodenode.h"

#include <QDebug>

QAsParser::QAsParser(asIScriptEngine *engine) : asBuilder(), _engine(engine) {
    constexpr const char *_HEADER_ =
        "namespace reader { bool copy(bool hex = false); }";
    constexpr size_t _HEADER_LEN_ = std::char_traits<char>::length(_HEADER_);

    // module->AddScriptSection("header", _HEADER_, _HEADER_LEN_, 0);

    ClearAll();

    if (StartNewModule(_engine, "_HEADER_PARSER_") != 0) {
        Q_ASSERT(false);
    }

    auto mod = dynamic_cast<asCModule *>(GetModule());
    auto en = dynamic_cast<asCScriptEngine *>(engine);
    Q_ASSERT(en && mod);
    asCParser parser(new asCBuilder(en, mod));

    m_code.reset(new asCScriptCode);
    m_code->SetCode("_HEADER_", _HEADER_, true);

    parser.ParseScript(m_code.get());

    auto pnodes = parser.GetScriptNode();

    QList<QCodeNode *> qnodes;

    auto node = new QCodeNode;
    processNode(nullptr, pnodes, node);

    do {
        auto p = qnodes.first();
    } while (false);
}

QAsParser::~QAsParser() {}

void QAsParser::processNode(asCScriptCode *code, asCScriptNode *raw,
                            QCodeNode *node) {
    Q_ASSERT(raw && node);

    node->model = nullptr;
    int row = -1;
    if (code) {
        code->ConvertPosToRowCol(raw->tokenPos, &row, nullptr);
    }
    node->line = row;

    QByteArray ns; // namespace

    switch (raw->nodeType) {
    case snScript:
        break;
    case snNamespace:
        break;
    case snIdentifier: {
        auto name = QByteArray(m_code->code + raw->tokenPos, raw->tokenLength);
        node->setRole(QCodeNode::Name, name);

#ifdef QT_DEBUG
        qDebug() << name;
#endif
    } break;
    case snDeclaration:
        qDebug() << QByteArray(m_code->code + raw->tokenPos, raw->tokenLength);
        break;
    case snImport:
    case snEnum:
    case snTypedef:
    case snClass:
    case snMixin:
    case snInterface:
    case snFuncDef:
    case snVirtualProperty:
    case snVariableAccess:
    case snFunction:
    default:
        break;
    }

    auto p = raw->firstChild;
    while (p) {
        auto cnode = new QCodeNode;
        // cnode->parent = node;
        processNode(code, p, cnode);
        node->children.append(cnode);
        p = p->next;
    }
    node->children.append(node);
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

    // do {
    //     auto node = asNode2CodeNode(pnodes);

    //     auto p = pnodes->firstChild;
    //     while (p) {
    //         auto cnode = asNode2CodeNode(pnodes);
    //         cnode->parent = node;
    //         node->children.append(cnode);

    //         p = pnodes->next;
    //     }

    //     qnodes.append(node);
    //     pnodes = pnodes->next;
    // } while (pnodes != nullptr);

    return true;
}

bool QAsParser::parse(const QString &code, const QString &section) {
    return ProcessScriptSection(code.toUtf8(), code.length(), section, 0);
}
