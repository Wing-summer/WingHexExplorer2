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

#include <optional>
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
                P s;
                s.begin = sel.end;
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

    virtual std::variant<bool, std::optional<P>>
    mergeRegion(const P &sel, QMutex *locker = nullptr) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        if (canMerge(sel)) {
            if (locker) {
                locker->lock();
            }

            if (this->begin < sel.end) {
                this->begin = qMin(this->begin, next(sel.begin));
                this->end = qMax(next(this->end), sel.end);
            } else {
                this->begin = qMin(next(this->begin), sel.begin);
                this->end = qMax(this->end, next(sel.end));
            }

            if (locker) {
                locker->unlock();
            }
            return true;
        }
        return false;
    };

    bool isValid() const { return _valid; }

    bool operator<(const Super &item) const { return begin < item.begin; }
};

#include <QtConcurrent/QtConcurrentMap>

template <typename T, typename P>
class QHexRegionObjectList : public QVector<P> {
    static_assert(std::is_base_of_v<QHexRegionObject<T, P>, P>);

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        this->removeIf(cleanup);
#else
        this->erase(std::remove_if(this->begin(), this->end(), cleanup));
#endif

        QtConcurrent::blockingMap(
            buffer, [&locker, this](P &s) { mergeAdd(s, &locker); });
    }

    qsizetype mergeAdd(const P &sel, QMutex *locker = nullptr) {
        std::variant<bool, std::optional<P>> res = false;
        Q_ASSERT(sel.isNormalized());

        qsizetype idx = -1;

        auto p = this->begin();
        for (; p != this->end(); ++p) {
            res = p->mergeRegion(sel, locker);
            if (std::holds_alternative<bool>(res)) {
                auto merged = std::get<bool>(res);
                if (merged) {
                    break;
                }
            } else {
                auto region = std::get<std::optional<P>>(res);
                idx = std::distance(this->begin(), p);
                if (region.has_value()) {
                    auto v = region.value();
                    this->insert(
                        std::distance(this->constBegin(),
                                      std::upper_bound(this->constBegin(),
                                                       this->constEnd(), v)),
                        v);
                }
                break;
            }
        }

        if (std::holds_alternative<bool>(res)) {
            auto merged = std::get<bool>(res);
            if (merged) {
                auto idx = std::distance(this->begin(), p);
                auto m = this->takeAt(idx);
                mergeAdd(m, locker);
            } else {
                this->insert(
                    std::distance(this->constBegin(),
                                  std::upper_bound(this->constBegin(),
                                                   this->constEnd(), sel)),
                    sel);
            }
        }

        return idx;
    }
};

#endif // QHEXREGIONOBJECT_H
