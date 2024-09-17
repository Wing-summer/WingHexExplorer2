#ifndef ASCONTEXTMGR_H
#define ASCONTEXTMGR_H

#include "AngelScript/sdk/add_on/contextmgr/contextmgr.h"

class asContextMgr : public CContextMgr {
public:
    asContextMgr();

    bool isRunning() const;
};

#endif // ASCONTEXTMGR_H
