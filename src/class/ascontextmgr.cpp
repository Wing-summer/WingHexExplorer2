#include "ascontextmgr.h"

asContextMgr::asContextMgr() : CContextMgr() {}

bool asContextMgr::isRunning() const { return !m_threads.empty(); }
