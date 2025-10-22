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
**
** The original License is MIT from Dax89/QHexView. I have modified a lot so I
** decide to change the Open Source License. You can use the original library
** under MIT. Thanks for Dax89's efforts.
** =============================================================================
*/

#ifndef QHEXUNDOSTACK_H
#define QHEXUNDOSTACK_H

#include <QSet>
#include <QUndoStack>

class QHexUndoStack : public QUndoStack {
    Q_OBJECT
public:
    explicit QHexUndoStack(QObject *parent = nullptr);
    virtual ~QHexUndoStack();

    bool hasUnsavedId(int id) const;

private slots:
    void recomputeUnsaved();

private:
    void collectRecursive(const QUndoCommand *cmd, QSet<int> &out) const;

private:
    QSet<int> m_unsavedCounts;
};

#endif // QHEXUNDOSTACK_H
