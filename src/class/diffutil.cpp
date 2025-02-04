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

#include "diffutil.h"

QVector<DiffUtil::DiffEntry> DiffUtil::compareFiles(const QStringList &lines1,
                                                    const QStringList &lines2) {
    return generateOrderedDiff(lines1, lines2);
}

QVector<QVector<int>> DiffUtil::computeLCSMatrix(const QStringList &A,
                                                 const QStringList &B) {
    auto m = A.size();
    auto n = B.size();
    QVector<QVector<int>> lcs(m + 1, QVector<int>(n + 1, 0));

    for (qsizetype i = 1; i <= m; ++i) {
        for (qsizetype j = 1; j <= n; ++j) {
            if (A[i - 1] == B[j - 1]) {
                lcs[i][j] = lcs[i - 1][j - 1] + 1;
            } else {
                lcs[i][j] = qMax(lcs[i - 1][j], lcs[i][j - 1]);
            }
        }
    }
    return lcs;
}

QVector<DiffUtil::DiffEntry>
DiffUtil::generateOrderedDiff(const QStringList &A, const QStringList &B) {
    auto lcs = computeLCSMatrix(A, B);
    auto i = A.size();
    auto j = B.size();
    QVector<DiffEntry> diffEntries;

    while (i > 0 && j > 0) {
        if (A[i - 1] == B[j - 1]) {
            diffEntries.prepend({B[j - 1], DiffEntry::UNCHANGED, i, j});
            --i;
            --j;
        } else if (lcs[i - 1][j] >= lcs[i][j - 1]) {
            diffEntries.prepend({A[i - 1], DiffEntry::REMOVED, i, -1});
            --i;
        } else {
            diffEntries.prepend({B[j - 1], DiffEntry::ADDED, -1, j});
            --j;
        }
    }

    // Remaining lines in A (deleted lines)
    while (i > 0) {
        diffEntries.prepend({A[i - 1], DiffEntry::REMOVED, i, -1});
        --i;
    }

    // Remaining lines in B (added lines)
    while (j > 0) {
        diffEntries.prepend({B[j - 1], DiffEntry::ADDED, -1, j});
        --j;
    }

    return diffEntries;
}
