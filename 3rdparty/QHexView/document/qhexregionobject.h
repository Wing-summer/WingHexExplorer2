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
#ifndef QHEXREGIONOBJECT_H
#define QHEXREGIONOBJECT_H

#include <type_traits>
#include <utility>

namespace REQUIRE_CHECK {

template <typename, typename = void>
struct has_greater_equal : std::false_type {};

template <typename T>
struct has_greater_equal<
    T, std::void_t<decltype(std::declval<T>() >= std::declval<T>())>>
    : std::true_type {};

// Repeat for other operators
template <typename, typename = void>
struct has_greater : std::false_type {};

template <typename T>
struct has_greater<T,
                   std::void_t<decltype(std::declval<T>() > std::declval<T>())>>
    : std::true_type {};

template <typename, typename = void>
struct has_less_equal : std::false_type {};

template <typename T>
struct has_less_equal<
    T, std::void_t<decltype(std::declval<T>() <= std::declval<T>())>>
    : std::true_type {};

template <typename, typename = void>
struct has_less : std::false_type {};

template <typename T>
struct has_less<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>>
    : std::true_type {};

template <typename T, typename = void>
struct has_equal : std::false_type {};

template <typename T>
struct has_equal<T,
                 std::void_t<decltype(std::declval<T>() == std::declval<T>())>>
    : std::true_type {};

template <typename, typename = void>
struct has_not_equal : std::false_type {};

template <typename T>
struct has_not_equal<
    T, std::void_t<decltype(std::declval<T>() != std::declval<T>())>>
    : std::true_type {};

} // namespace REQUIRE_CHECK

#include <variant>

#include <QMutex>

template <typename T, typename P>
struct QHexRegionObject {
    static_assert(REQUIRE_CHECK::has_greater<T>::value,
                  "Operator > is required");
    static_assert(REQUIRE_CHECK::has_less<T>::value, "Operator < is required");
    static_assert(REQUIRE_CHECK::has_greater_equal<T>::value,
                  "Operator >= is required");
    static_assert(REQUIRE_CHECK::has_less_equal<T>::value,
                  "Operator <= is required");
    static_assert(REQUIRE_CHECK::has_equal<T>::value,
                  "Operator == is required");
    static_assert(REQUIRE_CHECK::has_not_equal<T>::value,
                  "Operator != is required");

    using Super = QHexRegionObject<T, P>;

private:
    bool _valid = true;

protected:
    T next(const T &obj) const {
        T ret(obj);
        ++ret;
        return ret;
    }

public:
    T begin;
    T end;

    void normalize(QMutex *locker = nullptr) {
        Q_ASSERT(isValid());
        if (locker) {
            locker->lock();
        }
        if (end < begin) {
            std::swap(begin, end);
        }
        if (locker) {
            locker->unlock();
        }
    }

    qsizetype length() const {
        Q_ASSERT(isValid());
        return qAbs(end - begin) + 1;
    }

    bool contains(const P &sel) const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return this->begin <= sel.begin && this->end >= sel.end;
    }

    bool contains(const T &offset) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return this->begin <= offset && this->end >= offset;
    }

    bool isNormalized() const {
        Q_ASSERT(isValid());
        return end >= begin;
    }

    P normalized() const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        P sel = *reinterpret_cast<const P *>(this);
        if (end < begin) {
            std::swap(sel.begin, sel.end);
        }
        return sel;
    }

    bool isIntersected(const P &sel) const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return !(sel.end < begin || sel.begin > end);
    }

    bool canMerge(const P &sel) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return !(next(sel.end) < this->begin || sel.begin > next(this->end));
    }

    bool intersect(const P &sel, QMutex *locker = nullptr) {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        if (!isIntersected(sel)) {
            return false;
        }
        auto s = sel.normalized();
        if (locker) {
            locker->lock();
        }

        this->begin = qMax(this->begin, s.begin);
        this->end = qMin(this->end, s.end);

        if (locker) {
            locker->unlock();
        }
        return true;
    }

    Q_REQUIRED_RESULT virtual std::variant<bool, P>
    removeRegion(const P &sel, QMutex *locker = nullptr) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        Q_ASSERT(sel.isNormalized());

        std::variant<bool, P> result = false;

        if (locker) {
            locker->lock();
        }

        if (sel.begin <= this->begin) {
            if (sel.end >= this->begin) {
                if (sel.end < this->end) {
                    this->begin = sel.end;
                    ++this->begin;
                } else {
                    // makes it invalid, delete later
                    _valid = false;
                }
                result = true;
            }
        } else if (sel.begin > this->begin && sel.begin < this->end) {
            auto oldend = this->end;

            this->end = sel.begin;
            --this->end;
            result = true;

            if (sel.end < oldend) {
                // break into two ranges
                P s = *static_cast<const P *>(this);
                s.begin = next(sel.end);
                s.end = oldend;

                if (locker) {
                    locker->unlock();
                }

                return s.normalized();
            }
        }

        if (locker) {
            locker->unlock();
        }
        return result;
    }

    virtual std::variant<bool, P> mergeRegion(const P &sel,
                                              QMutex *locker = nullptr) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        if (!canMerge(sel)) {
            return false;
        }

        if (locker) {
            locker->lock();
        }

        this->begin = qMin(this->begin, sel.begin);
        this->end = qMax(this->end, sel.end);

        if (locker) {
            locker->unlock();
        }
        return true;
    };

    bool isValid() const { return _valid; }

    bool operator<(const Super &item) const { return begin < item.begin; }
};

#include <QtConcurrent/QtConcurrentMap>

template <typename T, typename P>
class QHexRegionObjectList : public QVector<P> {
    static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);

public:
    struct MergeAddItem {
        // original indices (in the list BEFORE this call) that were removed.
        QVector<qsizetype> removed;
        // indices in the final list (AFTER this call) of newly inserted.
        QVector<qsizetype> inserted;
    };

public:
    QHexRegionObjectList() = default;

    void mergeRemove(const P &sel) {
        Q_ASSERT(sel.isNormalized());

        QList<P> buffer;
        QMutex locker;
        QtConcurrent::blockingMap(*this, [&buffer, &locker, &sel](P &s) {
            auto r = s.removeRegion(sel);
            if (std::holds_alternative<P>(r)) {
                auto region = std::get<P>(r);
                QMutexLocker l(&locker);
                buffer.append(region);
            }
        });

        // clean up invalid selections
        auto cleanup = [](const P &s) { return !s.isValid(); };
        this->removeIf(cleanup);
        QtConcurrent::blockingMap(
            buffer, [&locker, this](P &s) { mergeAdd(s, &locker); });
    }

    MergeAddItem mergeAdd(const P &sel, QMutex *locker = nullptr) {
        Q_ASSERT(sel.isNormalized());
        MergeAddItem ret;

        P sel0 = sel;
        qsizetype removedSoFar = 0;
        QList<P> regionSlices;
        QList<P> insertsFromRemoved;
        qsizetype i = 0;
        while (i < this->size()) {
            auto &p = this->operator[](i);

            if (sel0.contains(p)) {
                qsizetype origIndex = static_cast<qsizetype>(i) +
                                      static_cast<qsizetype>(removedSoFar);
                ret.removed.append(origIndex);

                this->takeAt(i);
                ++removedSoFar;
                continue;
            }

            auto r = p.mergeRegion(sel0, locker);
            if (std::holds_alternative<bool>(r)) {
                bool flag = std::get<bool>(r);
                qsizetype origIndex = static_cast<qsizetype>(i) +
                                      static_cast<qsizetype>(removedSoFar);

                ret.removed.append(origIndex);

                P taken = this->takeAt(i);
                ++removedSoFar;

                if (flag) {
                    sel0 = taken;
                    continue;
                } else {
                    insertsFromRemoved.append(taken);
                    continue;
                }
            } else {
                auto v = std::get<P>(r);
                regionSlices.append(v);
                break;
            }
        }

        QVector<P> toInsert;
        for (auto &v : regionSlices)
            toInsert.append(v);
        for (auto &v : insertsFromRemoved)
            toInsert.append(v);

        auto equal_by_less = [](const P &a, const P &b) -> bool {
            return !(a < b) && !(b < a);
        };
        bool sel_present = false;
        for (const auto &e : toInsert) {
            if (equal_by_less(e, sel0)) {
                sel_present = true;
                break;
            }
        }
        if (!sel_present)
            toInsert.append(sel0);

        QVector<P> finalInsert;
        for (const auto &v : toInsert) {
            bool found = false;
            for (const auto &u : finalInsert) {
                if (equal_by_less(u, v)) {
                    found = true;
                    break;
                }
            }
            if (!found)
                finalInsert.append(v);
        }

        for (const auto &v : finalInsert) {
            auto it = std::upper_bound(this->constBegin(), this->constEnd(), v);
            qsizetype idx =
                static_cast<qsizetype>(std::distance(this->constBegin(), it));
            this->insert(idx, v);
            ret.inserted.append(idx);
        }

        return ret;
    }
};

#endif // QHEXREGIONOBJECT_H
