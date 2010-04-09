#include "wbtypes.h"
#include "time.h"

//cyg_uint32 Tick1ISR(cyg_vector_t vector, cyg_addrword_t data);
//INT32 StartTick1(UINT32 uTicksPerSecond, INT32 nOpMode);
//INT32 StopTick1(void);
void my_gettimeofday(struct timeval* tv);
UINT32 GetTick1CurCount(void);
//VOID UpdataTick1Count(UINT32 uTick);