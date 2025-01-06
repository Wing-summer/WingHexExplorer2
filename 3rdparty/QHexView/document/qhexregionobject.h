#ifndef QHEXREGIONOBJECT_H
#define QHEXREGIONOBJECT_H

namespace REQUIRE_CHECK {

struct NO_OP {};

template <typename T, typename Arg>
NO_OP &operator>(const T &, const Arg &);
template <typename T, typename Arg>
NO_OP &operator>=(const T &, const Arg &);
template <typename T, typename Arg>
NO_OP &operator<(const T &, const Arg &);
template <typename T, typename Arg>
NO_OP &operator<=(const T &, const Arg &);
template <typename T, typename Arg>
NO_OP &operator==(const T &, const Arg &);
template <typename T, typename Arg>
NO_OP &operator!=(const T &, const Arg &);
int optest(NO_OP const &);

template <typename T>
char optest(T const &);

template <typename T, typename Arg = T>
struct GreatThanExists {
    enum { value = sizeof(optest(*(T *)(0) > *(Arg *)(0))) == sizeof(char) };
};
template <typename T, typename Arg = T>
struct LessThanExists {
    enum { value = sizeof(optest(*(T *)(0) < *(Arg *)(0))) == sizeof(char) };
};
template <typename T, typename Arg = T>
struct GreatEqualThanExists {
    enum { value = sizeof(optest(*(T *)(0) >= *(Arg *)(0))) == sizeof(char) };
};
template <typename T, typename Arg = T>
struct LessEqualThanExists {
    enum { value = sizeof(optest(*(T *)(0) <= *(Arg *)(0))) == sizeof(char) };
};
template <typename T, typename Arg = T>
struct EqualExists {
    enum { value = sizeof(optest(*(T *)(0) == *(Arg *)(0))) == sizeof(char) };
};
template <typename T, typename Arg = T>
struct NotEqualExists {
    enum { value = sizeof(optest(*(T *)(0) != *(Arg *)(0))) == sizeof(char) };
};

} // namespace REQUIRE_CHECK

#include <optional>
#include <variant>

#include <QMutex>

template <typename T>
struct QHexRegionGadget final {
    static_assert(REQUIRE_CHECK::GreatThanExists<T>::value,
                  "Operator > is required");
    static_assert(REQUIRE_CHECK::LessThanExists<T>::value,
                  "Operator < is required");
    static_assert(REQUIRE_CHECK::GreatEqualThanExists<T>::value,
                  "Operator >= is required");
    static_assert(REQUIRE_CHECK::LessEqualThanExists<T>::value,
                  "Operator <= is required");
    static_assert(REQUIRE_CHECK::EqualExists<T>::value,
                  "Operator == is required");
    static_assert(REQUIRE_CHECK::NotEqualExists<T>::value,
                  "Operator != is required");

    bool _valid = true;

public:
    T begin;
    T end;

    QHexRegionGadget() : begin(T()), end(T()) {}

    explicit QHexRegionGadget(const T &begin, const T &end)
        : begin(begin), end(end) {}

    void normalize() {
        Q_ASSERT(isValid());
        if (end < begin) {
            std::swap(begin, end);
        }
    }

    qsizetype length() const {
        Q_ASSERT(isValid());
        return qAbs(end - begin) + 1;
    }

    bool contains(const QHexRegionGadget &sel) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return this->begin <= sel.begin && this->end >= sel.end;
    }

    bool isNormalized() const {
        Q_ASSERT(isValid());
        return end >= begin;
    }

    QHexRegionGadget normalized() const {
        Q_ASSERT(isValid());
        QHexRegionGadget sel = *this;
        if (end < begin) {
            std::swap(sel.begin, sel.end);
        }
        return sel;
    }

    bool isIntersected(const QHexRegionGadget &sel) const {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        return !(sel.end < begin || sel.begin > end);
    }

    bool intersect(const QHexRegionGadget &sel) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());

        if (!isIntersected(sel)) {
            _valid = false;
            return false;
        }

        auto s = sel.normalized();

        this->begin = qMax(this->begin, s.begin);
        this->end = qMin(this->end, s.end);

        return true;
    }

    Q_REQUIRED_RESULT std::variant<bool, QHexRegionGadget>
    removeRegion(const QHexRegionGadget &sel) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        Q_ASSERT(sel.isNormalized());

        std::variant<bool, QHexRegionGadget> result = false;
        if (sel.begin <= this->begin) {
            if (sel.end >= this->begin) {
                if (sel.end < this->end) {
                    this->begin = sel.end;
                } else {
                    // makes it invalid, delete later
                    _valid = false;
                }
                result = true;
            }
        } else if (sel.begin > this->begin && sel.begin < this->end) {
            this->end = sel.begin;
            result = true;

            if (sel.end < this->end) {
                // break into two ranges
                QHexRegionGadget sel;
                sel.begin = sel.end;
                sel.end = this->end;

                return sel;
            }
        }

        return result;
    }

    std::variant<bool, std::optional<QHexRegionGadget>>
    mergeRegion(const QHexRegionGadget &sel) {
        Q_ASSERT(isValid());
        Q_ASSERT(isNormalized());
        if (isIntersected(sel)) {
            this->begin = qMin(this->begin, sel.begin);
            this->end = qMax(this->end, sel.end);
            return true;
        }
        return false;
    };

    bool isValid() const { return _valid; }
};

template <typename T, typename P>
struct QHexRegionObject {
    static_assert(REQUIRE_CHECK::GreatThanExists<T>::value,
                  "Operator > is required");
    static_assert(REQUIRE_CHECK::LessThanExists<T>::value,
                  "Operator < is required");
    static_assert(REQUIRE_CHECK::GreatEqualThanExists<T>::value,
                  "Operator >= is required");
    static_assert(REQUIRE_CHECK::LessEqualThanExists<T>::value,
                  "Operator <= is required");
    static_assert(REQUIRE_CHECK::EqualExists<T>::value,
                  "Operator == is required");
    static_assert(REQUIRE_CHECK::NotEqualExists<T>::value,
                  "Operator != is required");

    using Super = QHexRegionObject<T, P>;

private:
    bool _adjusted = false;

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
        if (_adjusted) {
            return !(next(sel.end) < this->begin ||
                     sel.begin > next(this->end));
        }
        return !(sel.end < begin || sel.begin > end);
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
        if (_adjusted) {
            if (this->begin < s.end) {
                this->begin = qMax(this->begin, next(s.begin));
                this->end = qMin(next(this->end), s.end);
            } else {
                this->begin = qMax(next(this->begin), s.begin);
                this->end = qMin(this->end, next(s.end));
            }
        } else {
            this->begin = qMax(this->begin, s.begin);
            this->end = qMin(this->end, s.end);
        }
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
                    if (_adjusted) {
                        ++this->begin;
                    }
                } else {
                    // makes it invalid, delete later
                    _valid = false;
                }
                result = true;
            }
        } else if (sel.begin > this->begin && sel.begin < this->end) {
            this->end = sel.begin;
            if (_adjusted) {
                --this->end;
            }
            result = true;

            if (sel.end < this->end) {
                // break into two ranges
                P sel;
                sel.begin = sel.end;
                sel.end = this->end;

                if (locker) {
                    locker->unlock();
                }

                return sel;
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
        if (isIntersected(sel)) {
            if (locker) {
                locker->lock();
            }
            if (_adjusted) {
                if (this->begin < sel.end) {
                    this->begin = qMin(this->begin, next(sel.begin));
                    this->end = qMax(next(this->end), sel.end);
                } else {
                    this->begin = qMin(next(this->begin), sel.begin);
                    this->end = qMax(this->end, next(sel.end));
                }
            } else {
                this->begin = qMin(this->begin, sel.begin);
                this->end = qMax(this->end, sel.end);
            }
            if (locker) {
                locker->unlock();
            }
            return true;
        }
        return false;
    };

    bool adjusted() const {
        Q_ASSERT(isValid());
        return _adjusted;
    };

    void setAdjusted(bool newAdjusted) {
        Q_ASSERT(isValid());
        _adjusted = newAdjusted;
    };

    bool isValid() const { return _valid; }
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
                    this->append(region.value());
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
                this->append(sel);
            }
        } else {
            this->append(sel);
        }

        return idx;
    }
};

#endif // QHEXREGIONOBJECT_H
