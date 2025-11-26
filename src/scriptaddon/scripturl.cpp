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

#include "scripturl.h"

#include <QUrl>

static void QUrl_default_ctor(void *memory) { new (memory) QUrl(); }
static void QUrl_default_dtor(QUrl *self) { self->~QUrl(); }
static void QUrl_copy_ctor(void *memory, const QUrl &o) {
    new (memory) QUrl(o);
}
static void QUrl_ctor_fromQString(void *memory, const QString &url) {
    new (memory) QUrl(url);
}

static QUrl *QUrl_assign(QUrl *self, const QUrl &other) {
    *self = other;
    return self;
}

static void QUrl_setUrl_noMode(QUrl *self, const QString &url) {
    self->setUrl(url);
}

static QString QUrl_url_noFormat(const QUrl *self) { return self->url(); }
static QString QUrl_toString_noFormat(const QUrl *self) {
    return self->toString();
}
static QString QUrl_toDisplayString_noFormat(const QUrl *self) {
    return self->toDisplayString();
}

static QUrl QUrl_fromUserInput(const QString &userInput,
                               const QString &workDirectory) {
    return QUrl::fromUserInput(userInput, workDirectory);
}

static bool QUrl_isValid(const QUrl *self) { return self->isValid(); }
static QString QUrl_errorString(const QUrl *self) {
    return self->errorString();
}
static bool QUrl_isEmpty(const QUrl *self) { return self->isEmpty(); }
static void QUrl_clear(QUrl *self) { self->clear(); }

static void QUrl_setScheme(QUrl *self, const QString &s) { self->setScheme(s); }
static QString QUrl_scheme(const QUrl *self) { return self->scheme(); }

static void QUrl_setAuthority_noMode(QUrl *self, const QString &authority) {
    self->setAuthority(authority);
}
static QString QUrl_authority_noFormat(const QUrl *self) {
    return self->authority();
}

static void QUrl_setUserInfo_noMode(QUrl *self, const QString &ui) {
    self->setUserInfo(ui);
}
static QString QUrl_userInfo_noFormat(const QUrl *self) {
    return self->userInfo();
}

static void QUrl_setUserName_noMode(QUrl *self, const QString &un) {
    self->setUserName(un);
}
static QString QUrl_userName_noFormat(const QUrl *self) {
    return self->userName();
}

static void QUrl_setPassword_noMode(QUrl *self, const QString &pw) {
    self->setPassword(pw);
}
static QString QUrl_password_noFormat(const QUrl *self) {
    return self->password();
}

static void QUrl_setHost_noMode(QUrl *self, const QString &host) {
    self->setHost(host);
}
static QString QUrl_host_noFormat(const QUrl *self) { return self->host(); }

static void QUrl_setPort(QUrl *self, int port) { self->setPort(port); }
static int QUrl_port_default(const QUrl *self) { return self->port(); }

static void QUrl_setPath_noMode(QUrl *self, const QString &p) {
    self->setPath(p);
}
static QString QUrl_path_noFormat(const QUrl *self) { return self->path(); }
static QString QUrl_fileName_noFormat(const QUrl *self) {
    return self->fileName();
}

static void QUrl_setQuery_str(QUrl *self, const QString &q) {
    self->setQuery(q);
}
static bool QUrl_hasQuery(const QUrl *self) { return self->hasQuery(); }
static QString QUrl_query_noFormat(const QUrl *self) { return self->query(); }

// fragment
static bool QUrl_hasFragment(const QUrl *self) { return self->hasFragment(); }
static QString QUrl_fragment_noFormat(const QUrl *self) {
    return self->fragment();
}
static void QUrl_setFragment_noMode(QUrl *self, const QString &f) {
    self->setFragment(f);
}

static QUrl QUrl_resolved(const QUrl *self, const QUrl &rel) {
    return self->resolved(rel);
}
static bool QUrl_isRelative(const QUrl *self) { return self->isRelative(); }
static bool QUrl_isParentOf(const QUrl *self, const QUrl &url) {
    return self->isParentOf(url);
}
static bool QUrl_isLocalFile(const QUrl *self) { return self->isLocalFile(); }
static QUrl QUrl_fromLocalFile_str(const QString &lf) {
    return QUrl::fromLocalFile(lf);
}
static QString QUrl_toLocalFile(const QUrl *self) {
    return self->toLocalFile();
}

// comparisons
static bool QUrl_opEquals(const QUrl *self, const QUrl &other) {
    return (*self) == other;
}

static bool QUrl_opLess(const QUrl *self, const QUrl &other) {
    return (*self) < other;
}

static bool QUrl_matches_default(const QUrl *self, const QUrl &url) {
    return self->matches(url, QUrl::FormattingOptions(QUrl::PrettyDecoded));
}

void RegisterScriptUrl(asIScriptEngine *engine) {
    auto r = engine->RegisterObjectType("url", sizeof(QUrl),
                                        asOBJ_VALUE | asGetTypeTraits<QUrl>());
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour("url", asBEHAVE_CONSTRUCT, "void f()",
                                        asFUNCTION(QUrl_default_ctor),
                                        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour("url", asBEHAVE_DESTRUCT, "void f()",
                                        asFUNCTION(QUrl_default_dtor),
                                        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectBehaviour(
        "url", asBEHAVE_CONSTRUCT, "void f(const string &in)",
        asFUNCTION(QUrl_ctor_fromQString), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectBehaviour(
        "url", asBEHAVE_CONSTRUCT, "void f(const url &in)",
        asFUNCTION(QUrl_copy_ctor), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "url &opAssign(const url &in)",
                                     asFUNCTION(QUrl_assign),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "void setUrl(const string &in)",
                                     asFUNCTION(QUrl_setUrl_noMode),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string url() const",
                                     asFUNCTION(QUrl_url_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "string toString() const",
                                     asFUNCTION(QUrl_toString_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string toDisplayString() const",
                                     asFUNCTION(QUrl_toDisplayString_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // validity / errors
    r = engine->RegisterObjectMethod("url", "bool isValid() const",
                                     asFUNCTION(QUrl_isValid),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string errorString() const",
                                     asFUNCTION(QUrl_errorString),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "bool isEmpty() const",
                                     asFUNCTION(QUrl_isEmpty),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "url", "void clear()", asFUNCTION(QUrl_clear), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // scheme / authority / userinfo / username / password / host / port / path
    // / filename
    r = engine->RegisterObjectMethod("url", "void setScheme(const string &in)",
                                     asFUNCTION(QUrl_setScheme),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string scheme() const",
                                     asFUNCTION(QUrl_scheme),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "url", "void setAuthority(const string &in)",
        asFUNCTION(QUrl_setAuthority_noMode), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string authority() const",
                                     asFUNCTION(QUrl_authority_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "url", "void setUserInfo(const string &in)",
        asFUNCTION(QUrl_setUserInfo_noMode), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string userInfo() const",
                                     asFUNCTION(QUrl_userInfo_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "url", "void setUserName(const string &in)",
        asFUNCTION(QUrl_setUserName_noMode), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string userName() const",
                                     asFUNCTION(QUrl_userName_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod(
        "url", "void setPassword(const string &in)",
        asFUNCTION(QUrl_setPassword_noMode), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string password() const",
                                     asFUNCTION(QUrl_password_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "void setHost(const string &in)",
                                     asFUNCTION(QUrl_setHost_noMode),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string host() const",
                                     asFUNCTION(QUrl_host_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "void setPort(int)",
                                     asFUNCTION(QUrl_setPort),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "int port() const",
                                     asFUNCTION(QUrl_port_default),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "void setPath(const string &in)",
                                     asFUNCTION(QUrl_setPath_noMode),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string path() const",
                                     asFUNCTION(QUrl_path_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string fileName() const",
                                     asFUNCTION(QUrl_fileName_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // query / fragment
    r = engine->RegisterObjectMethod("url", "bool hasQuery() const",
                                     asFUNCTION(QUrl_hasQuery),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "void setQuery(const string &in)",
                                     asFUNCTION(QUrl_setQuery_str),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string query() const",
                                     asFUNCTION(QUrl_query_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "bool hasFragment() const",
                                     asFUNCTION(QUrl_hasFragment),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "string fragment() const",
                                     asFUNCTION(QUrl_fragment_noFormat),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "url", "void setFragment(const string &in)",
        asFUNCTION(QUrl_setFragment_noMode), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // resolved / relative / parent / localfile
    r = engine->RegisterObjectMethod("url", "url resolved(const url &in) const",
                                     asFUNCTION(QUrl_resolved),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "bool isRelative() const",
                                     asFUNCTION(QUrl_isRelative),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod(
        "url", "bool isParentOf(const url &in) const",
        asFUNCTION(QUrl_isParentOf), asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "bool isLocalFile() const",
                                     asFUNCTION(QUrl_isLocalFile),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterObjectMethod("url", "string toLocalFile() const",
                                     asFUNCTION(QUrl_toLocalFile),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // comparisons
    r = engine->RegisterObjectMethod(
        "url", "bool opEquals(const url &in) const", asFUNCTION(QUrl_opEquals),
        asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterObjectMethod("url", "bool opCmp(const url &in) const",
                                     asFUNCTION(QUrl_opLess),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    // matches
    r = engine->RegisterObjectMethod("url", "bool matches(const url &in) const",
                                     asFUNCTION(QUrl_matches_default),
                                     asCALL_CDECL_OBJFIRST);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->SetDefaultNamespace("url");
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    r = engine->RegisterGlobalFunction("url fromLocalFile(const string &in)",
                                       asFUNCTION(QUrl_fromLocalFile_str),
                                       asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);

    r = engine->RegisterGlobalFunction(
        "url fromUserInput(const string &in userInput, "
        "const string &in workDirectory)",
        asFUNCTION(QUrl_fromUserInput), asCALL_CDECL);
    Q_ASSERT(r >= 0);
    Q_UNUSED(r);
    engine->SetDefaultNamespace("");
}
