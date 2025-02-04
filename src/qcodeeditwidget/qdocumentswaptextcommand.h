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

class QDocumentSwapTextCommand : public QDocumentCommand {
public:
    explicit QDocumentSwapTextCommand(const QString &text, QDocument *doc,
                                      QDocumentCommand *p = nullptr);

    // QUndoCommand interface
public:
    void undo();
    void redo();

private:
    QString header, oldheader;
    QList<QDocumentLineHandle *> _handles;
    QList<QDocumentLineHandle *> _oldhandles;
    QVector<DiffUtil::DiffEntry> diffs;

    int _line, _column;
};

#endif // QDOCUMENTSWAPTEXTCOMMAND_H
