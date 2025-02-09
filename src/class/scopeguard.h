#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H

#include <QtGlobal>

template <typename CtorFn, typename DctorFn>
class ScopeGuard {
    const DctorFn &dctorfn;

    Q_DISABLE_COPY_MOVE(ScopeGuard)

public:
    ScopeGuard(CtorFn &&ctorfn, DctorFn &&dctorfn) : dctorfn(dctorfn) {
        ctorfn();
    }
    ~ScopeGuard() { dctorfn(); }
};

#endif // SCOPEGUARD_H
