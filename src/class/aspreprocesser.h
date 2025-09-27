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

#ifndef ASPREPROCESSER_H
#define ASPREPROCESSER_H

#include "WingPlugin/wingplugin_global.h"
#include "angelscript.h"

#include <QApplication>
#include <QEventLoop>
#include <QMap>
#include <QSet>
#include <QVector>

class AsPreprocesser;
class asCScriptCode;

/** for macros, we support:
 *      * #if <conditions>
 *      * #else
 *      * #elif
 *      * #endif
 *      * #ifdef <word>
 *      * #ifndef <word>
 */
class AsPreprocesser {
public:
    enum class Severity { Info = 0, Warning = 1, Error = 2 };

    enum class PreprocErrorCode {
        ERR_SUCCESS,
        ERR_ERROR,
        ERR_SOURCE_DEFINE_FORBIDDEN,
        ERR_ELIF_ELSE_WITHOUT_IF,
        ERR_ENDIF_MISSING,
        ERR_IF_PARSE,
        ERR_MACRO_EXPR_RECURSION,
        ERR_INCLUDE_NOT_FOUND,
        ERR_MARCO_ERROR
    };

    struct PreprocError {
        PreprocErrorCode code;
        Severity severity;
        QString file;
        qint64 line;
        qint64 column;
        QString message;
    };

    using CErrorCallback = std::function<void(
        const QString &file, qint64 line, qint64 column, PreprocErrorCode code,
        Severity severity, const QString &message)>;

    using CPragamaCallback = std::function<std::optional<WingHex::PragmaResult>(
        const QString &pragmaText, AsPreprocesser *builder,
        const QString &sectionname)>;

    struct SourcePos {
        QString file;
        qint64 line = 1;
        qint64 column = 1;
    };

    struct LineSegmentMap {
        qint64 outColStart = 1;
        qint64 length = 1;
        QString file;
        qint64 originalLine = 1;
        qint64 origColStart = 1;
    };

    struct OutputLineMap {
        qint64 outLineNumber = 1;
        QVector<LineSegmentMap> segments;
    };

    struct Mapping {
        QVector<OutputLineMap> lines;

        inline std::optional<SourcePos>
        mapOutputPositionToSource(qint64 outLine, qint64 outCol) const {
            if (outLine < 1)
                return std::nullopt;
            for (const auto &olm : lines) {
                if (olm.outLineNumber == outLine) {
                    for (const auto &seg : olm.segments) {
                        if (outCol >= seg.outColStart &&
                            outCol < seg.outColStart + seg.length) {
                            auto delta = outCol - seg.outColStart;
                            return SourcePos{seg.file, seg.originalLine,
                                             seg.origColStart + delta};
                        }
                    }
                    if (!olm.segments.isEmpty()) {
                        const auto &seg = olm.segments.last();
                        return SourcePos{seg.file, seg.originalLine,
                                         seg.origColStart + seg.length - 1};
                    }
                    return std::nullopt;
                }
            }
            return std::nullopt;
        }

        inline void addOutputLineMap(const OutputLineMap &olm) {
            lines.append(olm);
        }
    };

    struct Result {
        QString source;
        QByteArray checksum;
        QString script;
        Mapping mapping;
    };

public:
    explicit AsPreprocesser(asIScriptEngine *engine);
    virtual ~AsPreprocesser();

public:
    void registerBuiltin(const QString &name,
                         std::function<QString(const SourcePos &)> cb);

private:
    void processBuffer(const QByteArray &buf, const QString &sourceName,
                       const QString &currentDir, QString &outText,
                       Mapping &outMap);

    static bool parseIncludePathToken(const QString &args, QString &outPath,
                                      bool &isAngled);

    QString resolveIncludeFile(const QString &filename,
                               const QString &currentDir);

private:
    QString expandExpressionForIf(const QString &expr, const QString &file,
                                  qint64 line, qint64 col);

    std::optional<bool> evalExpression(const QString &expr, const QString &file,
                                       qint64 line, qint64 col);

private:
    QString m_currentSource;
    QHash<QString, QString> m_runtimeMacros;
    QHash<QString, std::function<QString(const SourcePos &)>> m_builtinMacros;

public:
    // Load a script section from a file on disk
    // Returns  1 if the file was included
    //          0 if the file had already been included before
    //         <0 on error
    int loadSectionFromFile(const QString &filename);

    void addScriptSection(const QString &section, const Result &result);

    QHash<QString, Result> scriptData() const;

    std::optional<SourcePos> mapErrPos2Src(const QString &section, qint64 line,
                                           qint64 col);

    // Returns the engine
    asIScriptEngine *getEngine() const;

    // Register the callback for resolving pragma directive
    void setPragmaCallback(const CPragamaCallback &callback);

    // Add a pre-processor define for conditional compilation
    void defineMacroWord(const QString &word, const QString &value = {});

    std::function<void(const PreprocError &)> getErrorHandler() const;
    void setErrorHandler(
        const std::function<void(const PreprocError &)> &newErrorHandler);

protected:
    void clearAll();

    int loadScriptSection(const QString &filename);
    bool includeIfNotAlreadyIncluded(const QString &filename);

    Result preprocess(const QByteArray &source, const QString &sourceName);

    void errorReport(const PreprocError &error);

private:
    asIScriptEngine *engine;
    QHash<QString, Result> modifiedScripts;

    CPragamaCallback pragmaCallback;
    std::function<void(const PreprocError &)> errorHandler;
    bool errOccurred = false;

    QStringList includedScripts;
};

#endif // ASPREPROCESSER_H
