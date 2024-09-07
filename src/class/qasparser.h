#ifndef _QAS_PARSER_H_
#define _QAS_PARSER_H_

#include "qcodelexer.h"
#include "qcodeparser.h"

#include <QString>

class QAsParser : public QCodeParser {
public:
    enum KeywordId {
        KEYWORD_NONE = -1,

        KEYWORD_ATTRIBUTE,
        KEYWORD_ASM,
        KEYWORD_AUTO,
        KEYWORD_BOOL,
        KEYWORD_BREAK,
        KEYWORD_CASE,
        KEYWORD_CATCH,
        KEYWORD_CHAR,
        KEYWORD_CLASS,
        KEYWORD_CONST,
        KEYWORD_CONTINUE,
        KEYWORD_DEFAULT,
        KEYWORD_DELETE,
        KEYWORD_DO,
        KEYWORD_DOUBLE,
        KEYWORD_ELSE,
        KEYWORD_ENUM,
        KEYWORD_EXPLICIT,
        KEYWORD_EXTERN,
        KEYWORD_FLOAT,
        KEYWORD_FOR,
        KEYWORD_FRIEND,
        KEYWORD_FUNCTION,
        KEYWORD_GOTO,
        KEYWORD_IF,
        KEYWORD_INLINE,
        KEYWORD_INT,
        KEYWORD_INTERNAL,
        KEYWORD_LONG,
        KEYWORD_MUTABLE,
        KEYWORD_NAMESPACE,
        KEYWORD_NEW,
        KEYWORD_OPERATOR,
        KEYWORD_PRIVATE,
        KEYWORD_PROTECTED,
        KEYWORD_PUBLIC,
        KEYWORD_REGISTER,
        KEYWORD_RETURN,
        KEYWORD_SHORT,
        KEYWORD_SIGNED,
        KEYWORD_STATIC,
        KEYWORD_STRUCT,
        KEYWORD_SWITCH,
        KEYWORD_SIZEOF,
        KEYWORD_TEMPLATE,
        KEYWORD_THIS,
        KEYWORD_THROW,
        KEYWORD_TRY,
        KEYWORD_TYPEDEF,
        KEYWORD_TYPENAME,
        KEYWORD_UNION,
        KEYWORD_UNSIGNED,
        KEYWORD_USING,
        KEYWORD_VIRTUAL,
        KEYWORD_VOID,
        KEYWORD_VOLATILE,
        KEYWORD_WCHAR_T,
        KEYWORD_WHILE,

        KEYWORD_COUNT
    };

    QAsParser();
    virtual ~QAsParser();

    virtual QCodeLexer *lexer(QCodeStream *s);

    virtual QString language() const;
    virtual bool canParse(const QString &fn) const;
    virtual void update(QCodeNode *n, QCodeLexer *src, bool check = true);

    inline QCodeNode *getNode() { return QCodeParser::getNode(); }

    void update(QCodeNode *n, QCodeLexer *l, QTokenList &tokens, int id,
                int end, bool bNeedCxt = true, QTokenList *ns = 0);

private:
    QString sContext;
};

#endif // !_QCPP_PARSER_H_
