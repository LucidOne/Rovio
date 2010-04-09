#ifndef __RTC_H__
#define __RTC_H__

//#include "wblib.h"
//#include "W99702_reg.h"
//#include "wberrcode.h"
#include "i2clib.h"

//extern unsigned char _i2c_s_daddr;
//extern int _i2c_bEnClockDivisor;
//extern int _i2c_FSB_clockDivisor;
#ifdef CYGFUN_KERNEL_API_C
#include "cyg/infra/cyg_type.h"

#endif
typedef void TimerHandler(void);

#ifdef __cplusplus
extern "C"{
#endif

typedef struct rtc_i2c_type_t
{
	bool bIsGPIO;
	cyg_int32 uSDIN;
	cyg_int32 uSCLK;
	cyg_int32 uINTPin;
	cyg_int32 ClearInterruptStatus;
}RTC_I2C_TYPE_T;

//void set_alarm(cyg_uint32 time_stamp, cyg_uint32 week_day);
void SetTimerHandler( TimerHandler *handler);
void set_alarm(cyg_uint32 day,cyg_uint32 hour,cyg_uint32 minute, cyg_uint32 week_day);
void get_alarmtime(cyg_uint32 * day,cyg_uint32 * hour,cyg_uint32 * minute,cyg_uint32 * weekday);
void set_alarm_daily(cyg_uint32 hour,cyg_uint32 minute);
void set_alarm_monthly(cyg_uint32 day,cyg_uint32 hour,cyg_uint32 minute);
void set_alarm_weekly(cyg_uint32 hour,cyg_uint32 minute,cyg_uint32 week_day);
void set_dayofweek(cyg_uint32 dayofweek);
cyg_uint32 get_control2();
cyg_uint32 Get_RTC_time();
cyg_int32 rtcSetI2CType(RTC_I2C_TYPE_T eI2CType);

#ifdef __cplusplus
}
#endif
#endif