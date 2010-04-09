#ifndef __TT_THREAD_H__
#define __TT_THREAD_H__

#include "../../../../SysLib/Inc/wb_syslib_addon.h"

#include "./InInc/tt_sys.h"
#include "../../libList/Inc/list.h"
#include "../../libMemPool/Inc/memory_pool.h"




#include "./InInc/tt_msg.h"
#include "../InInc/tt_semaphore.h"
UINT64 tt_get_time (void);
UINT32 tt_get_ticks (void);
UINT32 tt_ticks_to_msec (UINT32 ticks);
UINT32 tt_msec_to_ticks (UINT32 msec);
void tt_msleep (UINT32 msec);
void tt_usleep (UINT32 usec);


#endif

