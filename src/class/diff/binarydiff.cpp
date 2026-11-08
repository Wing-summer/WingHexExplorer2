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

#include "binarydiff.h"

#include <QElapsedTimer>
#include <QMutexLocker>
#include <algorithm>
#include <cstring>
#include <limits>

namespace WingDiff {

Q_LOGGING_CATEGORY(lcBinaryDiff, "wing.diff")

void Options::normalize() {
    fileLimit = std::clamp(fileLimit, Limits::MinimumFileSize, Limits::MaximumFileSize);
    if (!Limits::isMemoryBudget(memoryLimit)) {
        memoryLimit = Limits::DefaultMemoryBudget;
    }
    detailLimit = std::clamp(detailLimit, Limits::MinimumDetailedRanges,
                             Limits::MaximumDetailedRanges);
}

ResultStore::ResultStore(quint64 budget) : m_budget(budget) {}

quint64 ResultStore::count() const {
    QMutexLocker lock(&m_mutex);
    return m_count;
}

quint64 ResultStore::differences() const {
    QMutexLocker lock(&m_mutex);
    return m_differences;
}

quint64 ResultStore::publishedDifferences() const {
    QMutexLocker lock(&m_mutex);
    return m_publishedDifferences;
}

quint64 ResultStore::coarseCount() const {
    QMutexLocker lock(&m_mutex);
    return m_coarse;
}

QPair<quint64, quint64> ResultStore::coveredOffsets() const {
    QMutexLocker lock(&m_mutex);
    if (!m_count) {
        return {0, 0};
    }
    const auto last = atUnlocked(m_count - 1);
    return {last.leftOffset + last.leftLength,
            last.rightOffset + last.rightLength};
}

quint64 ResultStore::allocatedBytes() const {
    QMutexLocker lock(&m_mutex);
    return m_blocks.size() * sizeof(Block) +
           m_blocks.capacity() * sizeof(void *);
}

quint64 ResultStore::remainingBytes() const {
    return m_budget - std::min(m_budget, allocatedBytes());
}

bool ResultStore::append(const Range &range) {
    if (!range.leftLength && !range.rightLength) {
        return true;
    }
    QMutexLocker lock(&m_mutex);
    if (!m_blocks.empty() && m_count > m_publishedCount) {
        auto &previous = m_blocks.back()->ranges[m_blocks.back()->used - 1];
        if (previous.type == range.type && previous.coarse == range.coarse &&
            previous.leftOffset + previous.leftLength == range.leftOffset &&
            previous.rightOffset + previous.rightLength == range.rightOffset) {
            previous.leftLength += range.leftLength;
            previous.rightLength += range.rightLength;
            return true;
        }
    }
    if (m_blocks.empty() || m_blocks.back()->used == BlockSize) {
        if ((m_blocks.size() + 1) * (sizeof(Block) + 2 * sizeof(void *)) >
            m_budget) {
            return false;
        }
        auto block = std::make_unique<Block>();
        block->first = m_count;
        block->firstDifference = m_differences;
        m_blocks.push_back(std::move(block));
    }
    auto &block = *m_blocks.back();
    block.ranges[block.used] = range;
    if (range.type != Type::Equal) {
        block.changed[block.differences++] = quint16(block.used);
        ++m_differences;
        if (range.coarse) {
            ++m_coarse;
        }
    }
    ++block.used;
    ++m_count;
    return true;
}

void ResultStore::publish() {
    QMutexLocker lock(&m_mutex);
    m_publishedCount = m_count;
    m_publishedDifferences = m_differences;
}

Range ResultStore::atUnlocked(quint64 index) const {
    return m_blocks[size_t(index / BlockSize)]
        ->ranges[size_t(index % BlockSize)];
}

bool ResultStore::difference(quint64 row, Range *range) const {
    QMutexLocker lock(&m_mutex);
    if (row >= m_differences) {
        return false;
    }
    auto it = std::upper_bound(m_blocks.begin(), m_blocks.end(), row,
                               [](quint64 n, const auto &block) {
                                   return n < block->firstDifference;
                               });
    // Blocks containing only Equal records may share the same prefix count.
    while (it != m_blocks.begin()) {
        --it;
        const auto &block = **it;
        if (row >= block.firstDifference &&
            row < block.firstDifference + block.differences) {
            *range =
                block
                    .ranges[block.changed[size_t(row - block.firstDifference)]];
            return true;
        }
    }
    return false;
}

QVector<Range> ResultStore::page(quint64 first, int count) const {
    QVector<Range> result;
    result.reserve(count);
    // Keep each critical section short even for background full-result
    // searches.
    for (int i = 0; i < count; ++i) {
        Range range;
        if (!difference(first + quint64(i), &range)) {
            break;
        }
        result.append(range);
    }
    return result;
}

quint64 ResultStore::lowerBoundUnlocked(quint64 offset, bool right) const {
    quint64 first = 0, last = m_count;
    while (first < last) {
        const auto mid = first + (last - first) / 2;
        const auto r = atUnlocked(mid);
        const auto start = right ? r.rightOffset : r.leftOffset;
        const auto length = right ? r.rightLength : r.leftLength;
        if (start + length < offset || (length && start + length == offset)) {
            first = mid + 1;
        } else {
            last = mid;
        }
    }
    return first;
}

QVector<Range> ResultStore::intersect(quint64 offset, quint64 length,
                                      bool right) const {
    QMutexLocker lock(&m_mutex);
    QVector<Range> result;
    for (auto i = lowerBoundUnlocked(offset, right); i < m_count; ++i) {
        const auto r = atUnlocked(i);
        const auto begin = right ? r.rightOffset : r.leftOffset;
        if (begin >= offset + length) {
            break;
        }
        if (r.type != Type::Equal) {
            result.append(r);
        }
    }
    return result;
}

quint64 ResultStore::mapOffset(quint64 offset, bool fromRight) const {
    QMutexLocker lock(&m_mutex);
    const auto index = lowerBoundUnlocked(offset, fromRight);
    if (index >= m_count) {
        return std::numeric_limits<quint64>::max();
    }
    const auto r = atUnlocked(index);
    const auto from = fromRight ? r.rightOffset : r.leftOffset;
    const auto to = fromRight ? r.leftOffset : r.rightOffset;
    const auto fromLength = fromRight ? r.rightLength : r.leftLength;
    const auto toLength = fromRight ? r.leftLength : r.rightLength;
    const auto relative =
        offset > from ? std::min(offset - from, fromLength) : 0;
    if (!fromLength) {
        return to;
    }
    if (fromLength == toLength) {
        return to + relative;
    }
    const auto mapped = quint64(qreal(relative) * qreal(toLength) / fromLength);
    return to + std::min(mapped, toLength);
}

namespace {
struct ReadSpan {
    QByteArray storage;
    qsizetype start = 0;
    qsizetype count = 0;
    qsizetype size() const { return count; }
    const char *constData() const { return storage.constData() + start; }
    char operator[](qsizetype index) const { return constData()[index]; }
    operator QByteArrayView() const {
        return QByteArrayView(constData(), count);
    }
};

struct Reader {
    QIODevice *device;
    Outcome *outcome;
    bool failed = false;
    Error error = Error::None;
    quint64 failedOffset = 0;
    QByteArray cache;
    quint64 cacheOffset = 0;
    quint64 cacheSize = 2 * MiB;
    ReadSpan read(quint64 offset, quint64 length, bool window = false) {
        if (!length) {
            return {};
        }
        if (offset >= cacheOffset &&
            offset < cacheOffset + quint64(cache.size())) {
            const auto available = cacheOffset + quint64(cache.size()) - offset;
            if (window || available >= length) {
                return {cache, qsizetype(offset - cacheOffset),
                        qsizetype(std::min(available, length))};
            }
        }
        const auto count = std::min(std::max(length, cacheSize),
                                    quint64(device->size()) - offset);
        ++outcome->seeks;
        if (!device->seek(qint64(offset))) {
            failed = true;
            error = Error::SeekFailed;
            failedOffset = offset;
            return {};
        }
        // Do not detach an old buffer while readers still hold a span into it.
        QByteArray bytes(qsizetype(count), Qt::Uninitialized);
        qint64 done = 0;
        while (done < qint64(count)) {
            const auto n =
                device->read(bytes.data() + done, qint64(count) - done);
            if (n <= 0) {
                failed = true;
                error = Error::ReadFailed;
                failedOffset = offset + quint64(done);
                return {};
            }
            done += n;
            outcome->bytesRead += quint64(n);
        }
        cache = std::move(bytes);
        cacheOffset = offset;
        return {cache, 0, qsizetype(length)};
    }
};

constexpr int MatchSize = 64;

quint64 fingerprint(const char *data) {
    quint64 value = 0;
    for (int i = 0; i < MatchSize; ++i) {
        value = value * 257 + quint8(data[i]);
    }
    return value;
}

struct Anchor {
    quint64 hash;
    quint64 offset;
};

class Matcher {
public:
    std::vector<Anchor> index;
    std::vector<quint64> filter;
    quint64 step = 4096;
    quint64 power = 1;

    explicit Matcher(quint64 budget) {
        for (int i = 1; i < MatchSize; ++i) {
            power *= 257;
        }
        filter.resize(size_t(std::max(quint64(1024), budget / 16 / 8)), 0);
    }

    void add(quint64 hash, quint64 offset) {
        index.push_back({hash, offset});
        const auto bit = hash % (filter.size() * 64);
        filter[size_t(bit / 64)] |= quint64(1) << (bit % 64);
    }

    void sort() {
        std::sort(index.begin(), index.end(),
                  [](const Anchor &a, const Anchor &b) {
                      return a.hash < b.hash ||
                             (a.hash == b.hash && a.offset < b.offset);
                  });
    }
    template <typename Verify>
    bool find(QByteArrayView bytes, quint64 minimum, quint64 *old,
              quint64 *position, const Verify &verify,
              const BinaryDiff::Cancel &cancel) const {
        if (bytes.size() < MatchSize) {
            return false;
        }
        auto hash = fingerprint(bytes.constData());
        for (qsizetype i = 0; i <= bytes.size() - MatchSize; ++i) {
            if ((i & 16383) == 0 && cancel && cancel()) {
                return false;
            }
            const auto bit = hash % (filter.size() * 64);
            if (filter[size_t(bit / 64)] & (quint64(1) << (bit % 64))) {
                auto it = std::lower_bound(
                    index.begin(), index.end(), Anchor{hash, minimum},
                    [](const Anchor &a, const Anchor &b) {
                        return a.hash < b.hash ||
                               (a.hash == b.hash && a.offset < b.offset);
                    });
                for (int candidate = 0;
                     candidate < 4 && it != index.end() && it->hash == hash;
                     ++candidate, ++it) {
                    if (verify(it->offset, bytes.constData() + i)) {
                        *old = it->offset;
                        *position = quint64(i);
                        return true;
                    }
                }
            }
            if (i < bytes.size() - MatchSize) {
                hash = (hash - quint8(bytes[i]) * power) * 257 +
                       quint8(bytes[i + MatchSize]);
            }
        }
        return false;
    }
};

// Bounded Myers trace. The trace itself and output are charged to the local
// budget.
bool localDiff(QByteArrayView a, QByteArrayView b, quint64 x, quint64 y,
               quint64 budget, quint64 workLimit,
               const BinaryDiff::Cancel &cancel, std::vector<Range> &output) {
    const int n = int(a.size()), m = int(b.size()), maximum = n + m;
    if (maximum > 131072) {
        return false;
    }
    const auto width = size_t(2 * maximum + 3);
    if (width * sizeof(int) > budget / 2) {
        return false;
    }
    std::vector<int> v(width, 0);
    std::vector<std::vector<int>> trace;
    const int zero = maximum + 1;
    quint64 work = 0;
    int distance = -1;
    for (int d = 0; d <= maximum; ++d) {
        if ((cancel && cancel()) ||
            (trace.size() + 2) * width * sizeof(int) > budget / 2) {
            return false;
        }
        trace.push_back(v);
        for (int k = -d; k <= d; k += 2) {
            if (++work > workLimit) {
                return false;
            }
            int ax;
            if (k == -d ||
                (k != d && v[size_t(zero + k - 1)] < v[size_t(zero + k + 1)])) {
                ax = v[size_t(zero + k + 1)];
            } else {
                ax = v[size_t(zero + k - 1)] + 1;
            }
            int by = ax - k;
            while (ax < n && by < m && a[ax] == b[by]) {
                ++ax;
                ++by;
            }
            v[size_t(zero + k)] = ax;
            if (ax >= n && by >= m) {
                distance = d;
                break;
            }
        }
        if (distance >= 0) {
            break;
        }
    }
    if (distance < 0) {
        return false;
    }
    std::vector<Type> reversed;
    reversed.reserve(size_t(maximum));
    int ax = n, by = m;
    for (int d = distance; d >= 0; --d) {
        const auto &previous = trace[size_t(d)];
        const int k = ax - by;
        const int previousK =
            (k == -d || (k != d && previous[size_t(zero + k - 1)] <
                                       previous[size_t(zero + k + 1)]))
                ? k + 1
                : k - 1;
        const int previousX = previous[size_t(zero + previousK)];
        const int previousY = previousX - previousK;
        while (ax > previousX && by > previousY) {
            reversed.push_back(Type::Equal);
            --ax;
            --by;
        }
        if (!d) {
            break;
        }
        if (ax == previousX) {
            reversed.push_back(Type::Insert);
            --by;
        } else {
            reversed.push_back(Type::Delete);
            --ax;
        }
    }
    const auto maximumRanges =
        std::min(quint64(maximum), budget / 4 / sizeof(Range));
    output.reserve(size_t(maximumRanges));
    for (auto it = reversed.rbegin(); it != reversed.rend(); ++it) {
        const auto type = *it;
        const quint64 left = type == Type::Insert ? 0 : 1;
        const quint64 right = type == Type::Delete ? 0 : 1;
        if (!output.empty() && output.back().type == type) {
            output.back().leftLength += left;
            output.back().rightLength += right;
        } else if (!output.empty() && type != Type::Equal &&
                   output.back().type != Type::Equal) {
            auto &last = output.back();
            last.type = Type::Replace;
            last.leftLength += left;
            last.rightLength += right;
        } else {
            if (output.size() >= maximumRanges) {
                output.clear();
                return false;
            }
            output.push_back({x, left, y, right, type, false});
        }
        x += left;
        y += right;
    }
    return true;
}
constexpr quint64 FastFrontierUpdates = 2'000'000;
constexpr quint64 PreciseFrontierUpdates = 8'000'000;

struct ResultWriter {
    ResultStore &results;
    const Options &options;
    Outcome &outcome;
    QElapsedTimer &elapsed;
    Reader &a;
    Reader &b;
    const BinaryDiff::Cancel &cancel;
    quint64 &x;
    quint64 &y;
    quint64 &summaryRecords;

    bool emitRange(Range r) {
        if (outcome.firstResultMs < 0) {
            outcome.firstResultMs = elapsed.elapsed();
        }
        if (!results.append(r)) {
            outcome.error = Error::ResultBudgetExceeded;
            qCWarning(lcBinaryDiff)
                << "The result memory budget was exhausted.";
            return false;
        }
        if (results.summarizing) {
            ++summaryRecords;
        }
        x = r.leftOffset + r.leftLength;
        y = r.rightOffset + r.rightLength;
        results.processed = x + y;
        return true;
    }
    bool emitGap(quint64 endX, quint64 endY, bool coarse) {
        if (x == endX && y == endY) {
            return true;
        }
        const auto lx = endX - x, ly = endY - y;
        if (!coarse && lx && ly && lx <= 64 * KiB && ly <= 64 * KiB) {
            const auto aa = a.read(x, lx), bb = b.read(y, ly);
            std::vector<Range> ranges;
            if (!a.failed && !b.failed &&
                localDiff(aa, bb, x, y, options.memoryLimit / 8,
                          options.precise ? PreciseFrontierUpdates
                                          : FastFrontierUpdates,
                          cancel, ranges)) {
                for (const auto &range : ranges) {
                    if (results.differences() >= options.detailLimit ||
                        results.remainingBytes() < 4 * MiB) {
                        results.summarizing = true;
                        break;
                    }
                    if (!emitRange(range)) {
                        return false;
                    }
                }
                if (x == endX && y == endY) {
                    return true;
                }
            }
        }
        return emitRange({x, endX - x, y, endY - y,
                          x == endX
                              ? Type::Insert
                              : (y == endY ? Type::Delete : Type::Replace),
                          (x != endX && y != endY)});
    }
};
} // namespace

Outcome BinaryDiff::compare(QIODevice *left, QIODevice *right,
                            const Options &requestedOptions,
                            ResultStore &results, const Cancel &cancel) {
    Options options = requestedOptions;
    options.normalize();
    Outcome outcome;
    QElapsedTimer elapsed;
    elapsed.start();
    if (!left || !right || !left->isReadable() || !right->isReadable() ||
        left->isSequential() || right->isSequential() || left->size() < 0 ||
        right->size() < 0) {
        outcome.error = Error::InvalidInput;
        qCWarning(lcBinaryDiff)
            << "Both inputs must be readable, seekable devices.";
        return outcome;
    }
    const auto leftSize = quint64(left->size()),
               rightSize = quint64(right->size());
    if (leftSize > options.fileLimit || rightSize > options.fileLimit) {
        outcome.error = Error::FileLimitExceeded;
        qCWarning(lcBinaryDiff)
            << "An input exceeds the configured file size limit.";
        return outcome;
    }
    Reader a{left, &outcome}, b{right, &outcome};
    quint64 x = 0, y = 0, summaryRecords = 0;
    quint64 summaryMisses = 0;
    const quint64 window = std::min(4 * MiB, options.memoryLimit / 32);
    ResultWriter writer{results, options, outcome, elapsed, a,
                        b,       cancel,  x,       y,       summaryRecords};
    std::unique_ptr<Matcher> global;
    while (x < leftSize || y < rightSize) {
        if (cancel && cancel()) {
            outcome.status = Status::Cancelled;
            return outcome;
        }
        if (a.failed || b.failed) {
            break;
        }
        if (results.differences() >= options.detailLimit ||
            results.remainingBytes() < 4 * MiB) {
            results.summarizing = true;
        }
        if (results.summarizing && (summaryRecords >= 65000 ||
                                    results.remainingBytes() < 256 * 1024)) {
            // Reserve a single coarse tail. Still verify all input is readable,
            // with cancellation.
            const auto startX = x, startY = y;
            while (x < leftSize || y < rightSize) {
                if (cancel && cancel()) {
                    x = startX;
                    y = startY;
                    outcome.status = Status::Cancelled;
                    return outcome;
                }
                const auto aa = a.read(x, std::min(MiB, leftSize - x));
                const auto bb = b.read(y, std::min(MiB, rightSize - y));
                if (a.failed || b.failed) {
                    break;
                }
                x += quint64(aa.size());
                y += quint64(bb.size());
                results.processed = x + y;
            }
            x = startX;
            y = startY;
            if (a.failed || b.failed ||
                !writer.emitGap(leftSize, rightSize, true)) {
                break;
            }
            continue;
        }
        if (x == leftSize || y == rightSize) {
            if (!writer.emitGap(leftSize, rightSize, true)) {
                break;
            }
            continue;
        }
        auto aa = a.read(x, std::min(window, leftSize - x), true);
        auto bb = b.read(y, std::min(window, rightSize - y), true);
        if (a.failed || b.failed) {
            break;
        }
        const auto common = std::min(aa.size(), bb.size());
        qsizetype prefix = 0;
        if (std::memcmp(aa.constData(), bb.constData(), size_t(common)) == 0) {
            prefix = common;
        } else {
            while (prefix < common && aa[prefix] == bb[prefix]) {
                ++prefix;
            }
        }
        if (prefix) {
            if (!writer.emitRange({x, quint64(prefix), y, quint64(prefix),
                                   Type::Equal, false})) {
                break;
            }
            continue;
        }
        // Small gaps are exact without paying for the large-file matcher.
        if (leftSize - x <= 64 * KiB && rightSize - y <= 64 * KiB) {
            if (!writer.emitGap(leftSize, rightSize, results.summarizing)) {
                break;
            }
            continue;
        }
        if (results.summarizing && leftSize - x <= MiB &&
            rightSize - y <= MiB) {
            if (!writer.emitGap(leftSize, rightSize, true)) {
                break;
            }
            continue;
        }
        // Long unrelated regions should spend their time on sequential reads,
        // rather than repeating a rolling search for every single window.
        if (results.summarizing && summaryMisses >= 8 && summaryMisses % 16) {
            ++summaryMisses;
            if (!writer.emitGap(std::min(x + MiB, leftSize),
                                std::min(y + MiB, rightSize), true)) {
                break;
            }
            continue;
        }
        const auto localLimit = results.summarizing
                                    ? qsizetype(window)
                                    : (options.precise ? qsizetype(256 * 1024)
                                                       : qsizetype(64 * 1024));
        const QByteArrayView localLeft(aa.constData(),
                                       std::min(aa.size(), localLimit));
        const QByteArrayView localRight(bb.constData(),
                                        std::min(bb.size(), localLimit));
        Matcher local(256 * 1024);
        local.step =
            results.summarizing ? 4096 : (options.precise ? 256 : 1024);
        local.index.reserve(size_t(quint64(localLeft.size()) / local.step + 1) *
                            2);
        const quint64 localStart = results.summarizing ? MiB : 0;
        for (quint64 p = localStart; p + MatchSize <= quint64(localLeft.size());
             p += local.step) {
            local.add(fingerprint(aa.constData() + p), x + p);
            if (p + 2 * MatchSize <= quint64(localLeft.size())) {
                local.add(fingerprint(aa.constData() + p + MatchSize),
                          x + p + MatchSize);
            }
        }
        local.sort();
        quint64 old = 0, position = 0;
        bool found = local.find(
            localRight, x, &old, &position,
            [&](quint64 offset, const char *bytes) {
                return std::memcmp(aa.constData() + (offset - x), bytes,
                                   MatchSize) == 0;
            },
            cancel);
        quint64 newPosition = y + position;
        // Only detailed mode builds a global index. Summary keeps sequential
        // bounded windows.
        if (!found && !results.summarizing) {
            if (!global) {
                global = std::make_unique<Matcher>(options.memoryLimit / 8);
                const quint64 maxEntries =
                    (options.memoryLimit / 8 - MiB) / sizeof(Anchor);
                global->step = options.precise ? 1024 : 4096;
                while ((leftSize - x) / global->step + 1 > maxEntries) {
                    global->step *= 2;
                }
                outcome.sampleStep = global->step;
                global->index.reserve(
                    size_t((leftSize - x) / global->step + 1));
                for (auto start = x; start < leftSize;) {
                    if (cancel && cancel()) {
                        outcome.status = Status::Cancelled;
                        return outcome;
                    }
                    auto chunk =
                        a.read(start, std::min(window, leftSize - start));
                    if (a.failed) {
                        break;
                    }
                    for (quint64 p = 0; p + MatchSize <= quint64(chunk.size());
                         p += global->step) {
                        global->add(fingerprint(chunk.constData() + p),
                                    start + p);
                    }
                    start += quint64(chunk.size());
                }
                global->sort();
            }
            const auto searchEnd = std::min(
                rightSize, y + (options.precise ? 256 * MiB : 64 * MiB));
            for (auto start = y; start < searchEnd && !a.failed && !b.failed;) {
                if (cancel && cancel()) {
                    outcome.status = Status::Cancelled;
                    return outcome;
                }
                auto chunk = b.read(start, std::min(window, searchEnd - start));
                if (b.failed) {
                    break;
                }
                found = global->find(
                    chunk, x, &old, &position,
                    [&](quint64 offset, const char *bytes) {
                        const auto candidate = a.read(offset, MatchSize);
                        return !a.failed && candidate.size() == MatchSize &&
                               std::memcmp(candidate.constData(), bytes,
                                           MatchSize) == 0;
                    },
                    cancel);
                if (found) {
                    newPosition = start + position;
                    break;
                }
                if (chunk.size() <= MatchSize) {
                    break;
                }
                start += quint64(chunk.size() - MatchSize + 1);
            }
            if (!found && !a.failed && !b.failed) {
                if (searchEnd < rightSize) {
                    results.searchLimited = true;
                    results.summarizing = true;
                    global.reset();
                    continue;
                }
                if (!writer.emitGap(leftSize, rightSize, true)) {
                    break;
                }
                continue;
            }
        }
        if (a.failed || b.failed) {
            break;
        }
        if (found) {
            summaryMisses = 0;
            // Extend backwards in cached windows to avoid misclassifying a
            // small insertion.
            while (old > x && newPosition > y &&
                   old - x <= quint64(aa.size()) &&
                   newPosition - y <= quint64(bb.size()) &&
                   aa[qsizetype(old - x - 1)] ==
                       bb[qsizetype(newPosition - y - 1)]) {
                --old;
                --newPosition;
            }
            if (old == x && newPosition == y) {
                if (!writer.emitRange(
                        {x, MatchSize, y, MatchSize, Type::Equal, false})) {
                    break;
                }
            } else if (!writer.emitGap(old, newPosition, results.summarizing)) {
                break;
            }
        } else {
            ++summaryMisses;
            if (!writer.emitGap(std::min(x + MiB, leftSize),
                                std::min(y + MiB, rightSize), true)) {
                break;
            }
        }
    }
    if (cancel && cancel()) {
        outcome.status = Status::Cancelled;
    } else if (a.failed || b.failed) {
        const auto &reader = a.failed ? a : b;
        outcome.error = reader.error;
        qCWarning(lcBinaryDiff)
            << "Input I/O failed" << reader.device->objectName()
            << reader.failedOffset << reader.device->errorString();

    } else if (outcome.error == Error::None) {
        outcome.status = Status::Complete;
    }
    return outcome;
}
} // namespace WingDiff
