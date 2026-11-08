/*==============================================================================
** Copyright (C) 2026-2029 WingSummer
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

#ifndef BINARYDIFF_H
#define BINARYDIFF_H

#include "diffconstants.h"

#include <QIODevice>
#include <QLoggingCategory>
#include <QMutex>
#include <QPair>
#include <QString>
#include <QVector>

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace WingDiff {

enum class Type : quint8 { Equal, Insert, Delete, Replace };

struct Range {
    quint64 leftOffset = 0;
    quint64 leftLength = 0;
    quint64 rightOffset = 0;
    quint64 rightLength = 0;
    Type type = Type::Equal;
    bool coarse = false;
};

struct Options {
    quint64 fileLimit = Limits::DefaultFileSize;
    quint64 memoryLimit = Limits::DefaultMemoryBudget;
    quint64 detailLimit = Limits::DefaultDetailedRanges;
    bool precise = false;
    void normalize();
};

enum class Status { Complete, Cancelled, Failed };

enum class Error {
    None,
    InvalidInput,
    FileLimitExceeded,
    OpenFailed,
    SourceChanged,
    ReadFailed,
    SeekFailed,
    ResultBudgetExceeded
};
Q_DECLARE_LOGGING_CATEGORY(lcBinaryDiff)

struct Outcome {
    Status status = Status::Failed;
    Error error = Error::None;
    quint64 bytesRead = 0;
    quint64 seeks = 0;
    quint64 sampleStep = 0;
    qint64 firstResultMs = -1;
};

// Fixed-size blocks avoid reallocating/copying the complete result on
// publication. Readers take short locks and copy only their requested page or
// visible range.
class ResultStore {
public:
    explicit ResultStore(quint64 budget);

public:
    bool append(const Range &range);
    void publish();
    quint64 count() const;
    quint64 differences() const;
    quint64 publishedDifferences() const;
    quint64 coarseCount() const;
    QPair<quint64, quint64> coveredOffsets() const;
    quint64 allocatedBytes() const;
    quint64 remainingBytes() const;
    QVector<Range> page(quint64 first, int count = 2048) const;
    QVector<Range> intersect(quint64 offset, quint64 length, bool right) const;
    quint64 mapOffset(quint64 offset, bool fromRight) const;
    bool difference(quint64 row, Range *range) const;

public:
    std::atomic<quint64> processed{0};
    std::atomic<bool> summarizing{false};
    std::atomic<bool> searchLimited{false};

private:
    static constexpr size_t BlockSize = 1024;
    struct Block {
        std::array<Range, BlockSize> ranges;
        std::array<quint16, BlockSize> changed;
        quint64 first = 0;
        quint64 firstDifference = 0;
        size_t used = 0;
        size_t differences = 0;
    };

private:
    Range atUnlocked(quint64 index) const;
    quint64 lowerBoundUnlocked(quint64 offset, bool right) const;

private:
    mutable QMutex m_mutex;
    std::vector<std::unique_ptr<Block>> m_blocks;
    quint64 m_budget;
    quint64 m_count = 0;
    quint64 m_publishedCount = 0;
    quint64 m_publishedDifferences = 0;
    quint64 m_differences = 0;
    quint64 m_coarse = 0;
};

class BinaryDiff {
public:
    using Cancel = std::function<bool()>;
    Outcome compare(QIODevice *left, QIODevice *right, const Options &options,
                    ResultStore &results, const Cancel &cancel = {});
};

} // namespace WingDiff
#endif
