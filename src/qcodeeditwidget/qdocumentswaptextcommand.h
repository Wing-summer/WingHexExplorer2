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

#ifndef QDOCUMENTSWAPTEXTCOMMAND_H
#define QDOCUMENTSWAPTEXTCOMMAND_H

#include "class/diffutil.h"
#include "qdocumentcommand.h"
#include "qlinemarksinfocenter.h"

#include <QMap>

class QEditor;

class QDocumentSwapTextCommand : public QDocumentCommand {
public:
    explicit QDocumentSwapTextCommand(const QString &text, QDocument *doc,
                                      QEditor *e,
                                      QDocumentCommand *p = nullptr);

    // QUndoCommand interface
public:
    void undo();
    void redo();

private:
    using Ranges = QMap<int, int>;

private:
    QLineMarkList backupMarks();
    void applyLineMarks(const QLineMarkList &marks);
    void undoApplyLineMarks(const QLineMarkList &marks);

    void removeCommonElements(QVector<int> &vec1, QVector<int> &vec2);

    Ranges getRange(const QVector<int> &vec);

private:
    QString header, oldheader;

    QList<QDocumentLineHandle *> _handles;
    QList<QDocumentLineHandle *> _oldhandles;
    QVector<DiffUtil::DiffEntry> diffs;

    QEditor *_e;
    int _vBarValue;
    int _hBarValue;

    Ranges _rmRanges;
    Ranges _addRanges;
};

#endif // QDOCUMENTSWAPTEXTCOMMAND_H
