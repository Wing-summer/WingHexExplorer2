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

#ifndef DIFFUTIL_H
#define DIFFUTIL_H

#include <QString>
#include <QStringList>
#include <QVector>

class DiffUtil {
public:
    struct DiffEntry {
        QString lineContent;
        enum ChangeType { UNCHANGED, ADDED, REMOVED } changeType;
        qsizetype lineNumberA; // Line number in file1 (original)
        qsizetype lineNumberB; // Line number in file2 (new)
    };

public:
    static QVector<DiffEntry> compareFiles(const QStringList &lines1,
                                           const QStringList &lines2,
                                           bool keepUnchanged);

private:
    // LCS algorithm to find longest common subsequence of lines
    static QVector<QVector<int>> computeLCSMatrix(const QStringList &A,
                                                  const QStringList &B);

    static QVector<DiffEntry> generateOrderedDiff(const QStringList &A,
                                                  const QStringList &B,
                                                  bool keepUnchanged);
};

#endif // DIFFUTIL_H
