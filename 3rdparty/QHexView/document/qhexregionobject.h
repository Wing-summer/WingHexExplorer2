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

public:
    inline static T next(const T &obj) {
        T ret(obj);
        ++ret;
        return ret;
    }

    inline static T prev(const T &obj) {
        T ret(obj);
        --ret;
        return ret;
    }

public:
    T begin{};
    T end{};

    inline void normalize(QMutex *locker = nullptr) {
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

    inline qsizetype length() const {
        Q_ASSERT(isValid());
        return qAbs(end - begin) + 1;
    }

    inline bool contains(const P &sel) const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return this->begin <= sel.begin && this->end >= sel.end;
    }

    inline bool contains(const T &offset) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return this->begin <= offset && this->end >= offset;
    }

    inline bool isNormalized() const {
        Q_ASSERT(isValid());
        return end >= begin;
    }

    inline P normalized() const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        P sel = *reinterpret_cast<const P *>(this);
        if (end < begin) {
            std::swap(sel.begin, sel.end);
        }
        return sel;
    }

    inline bool isIntersected(const P &sel) const {
        static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return !(sel.end < begin || sel.begin > end);
    }

    inline bool canMerge(const P &sel) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return !(next(sel.end) < this->begin || sel.begin > next(this->end));
    }

    inline bool intersect(const P &sel, QMutex *locker = nullptr) {
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

    inline virtual std::variant<bool, P>
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

    inline virtual std::variant<bool, P> mergeRegion(const P &sel,
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

    inline bool isValid() const { return _valid; }

    inline bool operator<(const Super &item) const {
        return begin < item.begin;
    }
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
    inline QHexRegionObjectList() = default;

    inline void mergeRemove(const P &sel) {
        Q_ASSERT(sel.isNormalized());

        QVector<P> buffer;
        QMutex mutex;
        QtConcurrent::blockingMap(*this, [&](P &s) {
            auto r = s.removeRegion(sel);
            if (std::holds_alternative<P>(r)) {
                QMutexLocker lock(&mutex);
                buffer.append(std::get<P>(r));
            }
        });

        this->removeIf([](const P &s) { return !s.isValid(); });
        for (const auto &fragment : std::as_const(buffer)) {
            mergeAdd(fragment);
        }
    }

    inline MergeAddItem mergeAdd(const P &sel, QMutex *locker = nullptr) {
        Q_ASSERT(sel.isNormalized());
        if (locker) {
            QMutexLocker lock(locker);
            return mergeAddImpl(sel);
        }
        return mergeAddImpl(sel);
    }

private:
    inline MergeAddItem mergeAddImpl(const P &sel) {
        MergeAddItem ret;
        const qsizetype n = this->size();
        if (n == 0) {
            this->append(sel);
            ret.inserted.append(0);
            return ret;
        }

        auto it = std::lower_bound(
            this->begin(), this->end(), sel,
            [](const P &a, const P &b) { return a.begin < b.begin; });
        qsizetype pos = std::distance(this->begin(), it);

        P merged = sel;
        qsizetype left = pos, right = pos;
        QVector<bool> touched(n, false);

        bool changed = true;
        while (changed) {
            changed = false;
            while (left > 0) {
                qsizetype i = left - 1;
                const P &candidate = this->at(i);
                if (candidate.end < P::prev(merged.begin))
                    break;
                P tmp = candidate;
                auto res = tmp.mergeRegion(merged, nullptr);
                if (std::holds_alternative<bool>(res) && std::get<bool>(res) &&
                    tmp.isValid()) {
                    merged = tmp.normalized();
                    touched[i] = true;
                    --left;
                    changed = true;
                } else {
                    break;
                }
            }
            while (right < n) {
                qsizetype i = right;
                const P &candidate = this->at(i);
                if (candidate.begin > P::next(merged.end))
                    break;
                P tmp = candidate;
                auto res = tmp.mergeRegion(merged, nullptr);
                if (std::holds_alternative<bool>(res) && std::get<bool>(res) &&
                    tmp.isValid()) {
                    merged = tmp.normalized();
                    touched[i] = true;
                    ++right;
                    changed = true;
                } else {
                    break;
                }
            }
        }

        qsizetype anchor = -1;
        for (qsizetype i = left; i < right; ++i) {
            if (this->at(i).begin == merged.begin &&
                this->at(i).end == merged.end) {
                anchor = i;
                break;
            }
        }

        qsizetype carveLeft = left;
        qsizetype carveRight = right;
        if (left > 0 && this->at(left - 1).isIntersected(merged))
            --carveLeft;
        if (right < n && this->at(right).isIntersected(merged))
            ++carveRight;

        struct Node {
            P item;
            bool inserted;
        };
        QVector<Node> leftFrag, rightFrag;

        for (qsizetype i = carveLeft; i < carveRight; ++i) {
            if (i >= left && i < right && touched[i]) {
                if (i != anchor)
                    ret.removed.append(i);
                continue;
            }

            const P old = this->at(i);
            if (old.end < merged.begin) {
                leftFrag.append({old, false});
            } else if (old.begin > merged.end) {
                rightFrag.append({old, false});
            } else {
                P cur = old;
                auto r = cur.removeRegion(merged, nullptr);
                ret.removed.append(i);
                if (cur.isValid())
                    leftFrag.append({cur, true});
                if (std::holds_alternative<P>(r))
                    rightFrag.append({std::get<P>(r), true});
            }
        }

        QVector<P> out;
        out.reserve(carveLeft + leftFrag.size() + 1 + rightFrag.size() +
                    (n - carveRight));

        for (qsizetype i = 0; i < carveLeft; ++i)
            out.append(this->at(i));

        for (const auto &node : leftFrag) {
            if (node.inserted)
                ret.inserted.append(out.size());
            out.append(node.item);
        }

        if (anchor >= 0) {
            out.append(this->at(anchor));
        } else {
            ret.inserted.append(out.size());
            out.append(merged);
        }

        for (const auto &node : rightFrag) {
            if (node.inserted)
                ret.inserted.append(out.size());
            out.append(node.item);
        }

        for (qsizetype i = carveRight; i < n; ++i)
            out.append(this->at(i));

        this->swap(out);
        return ret;
    }
};

#endif // QHEXREGIONOBJECT_H
