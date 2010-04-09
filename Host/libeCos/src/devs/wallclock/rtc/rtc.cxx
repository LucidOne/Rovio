#include "pkgconf/system.h"
#include "pkgconf/wallclock.h"          // Wallclock device config
#include "cyg/infra/cyg_type.h"         // Common type definitions and support
#include "cyg/hal/drv_api.h"	        // Driver API
#include "cyg/io/wallclock.hxx"         // The WallClock API
#include "cyg/io/wallclock/wallclock.inl" // Helpers
#include "cyg/infra/diag.h"
#include "cyg/hal/hal_io.h"             // IO macros
#include "cyg/hal/hal_intr.h" 


//#include "wblib.h"
#include "rtcio.h"
#include "702reg.h"
//#include "wberrcode.h"
#include "rtc.h"
#include "stdio.h"
//#include "settime.h"


#define RTC_CONTROL1			0x00
#define RTC_CONTROL2			0x01
#define RTC_SECOND 	 			0x02
#define RTC_MINUTE 				0x03
#define RTC_HOUR				0x04
#define RTC_DAYOFWEEK 			0x06
#define RTC_DAY 				0x05
#define RTC_MONTH				0x07
#define RTC_YEAR				0x08
#define RTC_ALARM_MINUTE		0x09
#define RTC_ALARM_HOUR			0x0a
#define RTC_ALARM_DAY			0x0b
#define RTC_ALARM_WEEKDAY		0x0c
#define CLOCK_CONTROL			0x0d
#define TIMER_CONTROL			0x0e
#define TIMER					0x0f

#define RTC_SLAVE_ADDRESS		0xA2

#define RTC_READ	0x01
#define RTC_WRITE	0x00

static cyg_int32 wSCK_MASK = 0x00000001;		//0000 0001b
static cyg_int32 wSCK_UNMASK = 0xfffffffe; 
static cyg_int32 wSDA_MASK = 0x00000002;		//0000 1000b
static cyg_int32 wSDA_UNMASK = 0xfffffffd;		//0000 1000b

static cyg_int32 rSCK_MASK = 0x00000008;
static cyg_int32 rSCK_UNMASK = 0xfffffff7; 
static cyg_int32 rSDA_MASK = 0x00000010;
static cyg_int32 rSDA_UNMASK = 0xffffffef;

static cyg_uint32 RTC_ALARM_INT_NUM = 23;
cyg_handle_t handle;
cyg_interrupt intr;
int value;

static int _i2c_bEnClockDivisor;
static int _i2c_FSB_clockDivisor;
static unsigned char _i2c_s_daddr;

static unsigned int _i2c_timer;
static TimerHandler *timerhandler;
static int TimeCounter;
RTC_I2C_TYPE_T _eI2CType = {false,1,0,0,0};

//Cyg_WallClock *Cyg_WallClock::wallclock;
static Cyg_WallClock wallclock_instance CYGBLD_ATTRIB_INIT_AFTER( CYG_INIT_CLOCK );
extern "C" {
__weak cyg_uint32 GPIO_INTHandler(cyg_vector_t vector, cyg_addrword_t data);//__weak
}

static void i2c_delay (int cnt)
{	
	int volatile i;
	int delayCnt=cnt*(_i2c_FSB_clockDivisor+1);

	for (i=0;i<delayCnt;++i);

}

static void RtcSetGPIO5OutPutStatus(void)
{
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)& 0xFFFFFFDF);//output
}

static void RtcSetGPIO5InPutStatus(void)
{
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE)|(~0xFFFFFFDF));//input
}
//bIsSerialBusMode=0 ==> Software I2C
//bIsSerialBusMode=1 ==> Fast Serial Bus
static void initSerialBus (unsigned char bIsSerialBusMode)
{	
	cyg_uint32 FSBctrlReg, regSerialBusCtrl;

	//To use Software I2C to initialize Serial bus firstly
	FSBctrlReg=inpw (REG_FastSerialBusCR);
	FSBctrlReg=FSBctrlReg&0xFFFFFFF8;		//Fast serial bus disable, interrupt disable, clock divisor disable
	outpw (REG_FastSerialBusCR, FSBctrlReg);
	
	regSerialBusCtrl=inpw(REG_SerialBusCR);
	regSerialBusCtrl=regSerialBusCtrl|0x03;		//SDA, SCK = 1
	outpw (REG_SerialBusCR,regSerialBusCtrl);

	if (bIsSerialBusMode)	//Fast Serial Bus
	{	
		//To enable fast serial bus clock sources firstly
		FSBctrlReg=inpw (REG_CLKCON);
			FSBctrlReg=FSBctrlReg|0x40;			//CLK_BA+0x04[6] : Fast Serial Bus enable (from crystal input)
			outpw (REG_CLKCON,FSBctrlReg);

		i2c_delay (100);	//??? for capacitor to integrate voltage	//k03294-1

		//Clock Divisor -- FPGA emulation should not be 0 (Can be any value, but should > 0)	//k03294-1
/*		//k05144-1-b
		FSBctrlReg=inpw (REG_FastSerialBusCR);
		FSBctrlReg=FSBctrlReg|0x200;
		outpw (REG_FastSerialBusCR,FSBctrlReg);
*/		//k05144-1-a
		
		FSBctrlReg=inpw (REG_FastSerialBusCR);
		FSBctrlReg=FSBctrlReg&0xFFFFFFF8;		//keep clock divisor	//k05144-1-test
		if (_i2c_bEnClockDivisor)	FSBctrlReg=FSBctrlReg|0x05;			//enable Fast I2C and Fast I2C clock divider
		else	FSBctrlReg=FSBctrlReg|0x01;			//only enable Fast I2C
//Clock divisor
		/*	//k04075-1-b
		FSBctrlReg=FSBctrlReg&0xFFFF00FF;		//keep clock divisor	//k05144-1-test
		FSBctrlReg=FSBctrlReg|(_i2c_FSB_clockDivisor<<8);
		outpw (REG_FastSerialBusCR,FSBctrlReg);
		*/	//k04075-1-a
	}	//if (bIsSerialBusMode)	//Fast Serial Bus

//		/*	//k04075-1-b
	FSBctrlReg=FSBctrlReg&0xFFFF00FF;		//keep clock divisor	//k05144-1-test
	FSBctrlReg=FSBctrlReg|(_i2c_FSB_clockDivisor<<8);
	outpw (REG_FastSerialBusCR,FSBctrlReg);
//		*/	//k04075-1-a
}


//static 
static void OstartI2C ()
{
	cyg_uint32 regI2C;
	if(!_eI2CType.bIsGPIO)
	{
		//make sure -- low SCLK	//?
		regI2C=inpw (REG_SerialBusCR);
		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);	//k04214-1
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);	//k04214-1
		//low SDA
		regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		//i2c_delay loop
	//k04214-1	i2c_delay (2);
		i2c_delay (10);	//k04214-1
		//low SCL
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);	//k04214-1
	}
	else
	{
		RtcSetGPIO5OutPutStatus();
		//make sure -- low SCLK	//?
		regI2C=inpw (REG_GPIO_STS);
		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);	//k04214-1
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);	//k04214-1
		//low SDA
		regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_GPIO_DAT,regI2C);
		//i2c_delay loop
	//k04214-1	i2c_delay (2);
		i2c_delay (10);	//k04214-1
		//low SCL
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);	//k04214-1

	}
}

//static 
static void OstopI2C ()
{	
	cyg_uint32 regI2C;
	if(!_eI2CType.bIsGPIO)
	{
		regI2C=inpw (REG_SerialBusCR);

		//low SDA
		regI2C=regI2C&&wSDA_UNMASK;
		outpw (REG_SerialBusCR, regI2C);
		i2c_delay (2);
		i2c_delay (10);		//k03224-2

		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR, regI2C);
		i2c_delay (10);		//k03224-2

		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR, regI2C);
		i2c_delay (10);		//k03224-2
	}
	else
	{
		RtcSetGPIO5OutPutStatus();
		regI2C=inpw (REG_GPIO_STS);

		//low SDA
		regI2C=regI2C&wSDA_UNMASK;
		outpw (REG_GPIO_DAT, regI2C);
		i2c_delay (2);
		i2c_delay (10);		//k03224-2

		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_GPIO_DAT, regI2C);
		i2c_delay (10);		//k03224-2

		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_GPIO_DAT, regI2C);
		i2c_delay (10);		//k03224-2

	}
}

//static 
static bool O1B2I2C_wACK (unsigned char odata)
{	unsigned char i;
	bool i2c_bus_timeout;
	volatile cyg_uint32 regGPIOStatus;

	cyg_uint32 regI2C;
	 
	if(!_eI2CType.bIsGPIO)
	{
		regI2C=inpw (REG_SerialBusCR);

		i2c_bus_timeout=false;
		for (i=0;i<8;++i)
		{	//msb -> lsb
			if (odata&0x80)	regI2C=regI2C|wSDA_MASK;
			else	regI2C=regI2C&wSDA_UNMASK;
			outpw (REG_SerialBusCR,regI2C);

			i2c_delay (3);	//???
			//high SCLK
			regI2C=regI2C|wSCK_MASK;
			outpw (REG_SerialBusCR,regI2C);
			i2c_delay (10);		//k03224-2

			odata<<=1;
			//low SCLK
			regI2C=regI2C&wSCK_UNMASK;
			outpw (REG_SerialBusCR,regI2C);
			i2c_delay (10);		//k03224-2
		}

		//pull high SDA
		regI2C=inpw(REG_SerialBusCR);	//k03224-1
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		//pull SCLK high
		regI2C=inpw(REG_SerialBusCR);	//k03224-1
		regI2C=regI2C|wSCK_MASK;	//k03224-1
		outpw (REG_SerialBusCR,regI2C);	//k03224-1
		i2c_delay (10);		//k03224-2

		//wait ACK
		_i2c_timer=1000;
		while ((inpw(REG_SerialBusCR)&rSDA_MASK))
		{	--_i2c_timer;
			if (_i2c_timer==0)
			{	i2c_bus_timeout=true;
				break;
			}
		}

		i2c_delay (10);	//k04214-1
		regI2C=inpw(REG_SerialBusCR);
		//pull SCLK high
	//k03224-1	regI2C=regI2C|wSCK_MASK;
	//k03224-1	outpw (REG_SerialBusCR,regI2C);
		//pull SCLK low
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);	//k04214-1
	
		return i2c_bus_timeout;
	}
	else
	{
		regI2C=inpw (REG_GPIO_STS);

		i2c_bus_timeout=false;
		for (i=0;i<8;++i)
		{	//msb -> lsb
			if (odata&0x80)	regI2C=regI2C|wSDA_MASK;
			else	regI2C=regI2C&wSDA_UNMASK;
			outpw (REG_GPIO_DAT,regI2C);

			i2c_delay (3);	//???
			//high SCLK
			regI2C=regI2C|wSCK_MASK;
			outpw (REG_GPIO_DAT,regI2C);
			i2c_delay (10);		//k03224-2

			odata<<=1;
			//low SCLK
			regI2C=regI2C&wSCK_UNMASK;
			outpw (REG_GPIO_DAT,regI2C);
			i2c_delay (10);		//k03224-2
		}

		//pull high SDA
		regI2C=inpw(REG_GPIO_STS);	//k03224-1
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);		//k03224-2

		//JSTseng
		//pull SCLK high
		regI2C=inpw(REG_GPIO_STS);	//k03224-1
		regI2C=regI2C|wSCK_MASK;	//k03224-1
		outpw (REG_GPIO_DAT,regI2C);	//k03224-1
		i2c_delay (10);		//k03224-2
		RtcSetGPIO5InPutStatus();
		//wait ACK
		_i2c_timer=1000;
		while ((inpw(REG_GPIO_STS)&rSDA_MASK))
		{	--_i2c_timer;
			if (_i2c_timer==0)
			{	i2c_bus_timeout=true;
				break;
			}
			regGPIOStatus = inpw(REG_GPIO_STS);
		}
		RtcSetGPIO5OutPutStatus();
		i2c_delay (10);	//k04214-1
		regI2C=inpw(REG_GPIO_STS);
		//pull SCLK high
	//k03224-1	regI2C=regI2C|wSCK_MASK;
	//k03224-1	outpw (REG_SerialBusCR,regI2C);
		//pull SCLK low
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);	//k04214-1

		return i2c_bus_timeout;

	}
}

static unsigned char I1B_fromI2C ()
{	unsigned char i;
	unsigned char iData;

	cyg_uint32 regI2C;
	cyg_uint32 regAddr;

	i2c_delay (20);	//??
	if(!_eI2CType.bIsGPIO)
	{
		regI2C=inpw (REG_SerialBusCR);

		iData=0;
		for (i=0;i<8;++i)
		{	regI2C=regI2C|wSDA_MASK;
			outpw (REG_SerialBusCR,regI2C);
			i2c_delay (10);		//k03224-2
			//pull high SCLK
			regI2C=regI2C|wSCK_MASK;
			outpw (REG_SerialBusCR,regI2C);
			i2c_delay (10);		//k03224-2

			//in data
			iData=iData<<1;
			regI2C=inpw (REG_SerialBusCR);
			if (regI2C&rSDA_MASK)	iData=iData|0x01;

			//pull low SCLK
			regI2C=regI2C&wSCK_UNMASK;
			outpw (REG_SerialBusCR,regI2C);
			i2c_delay (10);		//k03224-2
		}

		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
		//assert ACK
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2

		i2c_delay (2);
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_SerialBusCR,regI2C);
		i2c_delay (10);		//k03224-2
	}
	else
	{
		regI2C=inpw (REG_GPIO_STS);

		iData=0;
		for (i=0;i<8;++i)
		{	regI2C=regI2C|wSDA_MASK;
			outpw (REG_GPIO_DAT,regI2C);
			i2c_delay (10);		//k03224-2
			//pull high SCLK
			regI2C=regI2C|wSCK_MASK;
			outpw (REG_GPIO_DAT,regI2C);
			i2c_delay (10);		//k03224-2

			RtcSetGPIO5InPutStatus();
			//in data
			iData=iData<<1;
			regI2C=inpw (REG_GPIO_STS);
			if (regI2C&rSDA_MASK)	iData=iData|0x01;

			RtcSetGPIO5OutPutStatus();
			//pull low SCLK
			regI2C=regI2C&wSCK_UNMASK;
			outpw (REG_GPIO_DAT,regI2C);
			i2c_delay (10);		//k03224-2
		}

		//high SDA
		regI2C=regI2C|wSDA_MASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);		//k03224-2
		//assert ACK
		//high SCLK
		regI2C=regI2C|wSCK_MASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);		//k03224-2

		i2c_delay (2);
		//low SCLK
		regI2C=regI2C&wSCK_UNMASK;
		outpw (REG_GPIO_DAT,regI2C);
		i2c_delay (10);		//k03224-2

	}
	//return iData;
	return iData;
}


static unsigned char rtc_DSP_ReadI2C (unsigned char addr)
{	unsigned char iData;

	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);			//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
	//OstopI2C ();
	
	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr|RTC_READ);		//need it every time and when multiple registers ?
	iData=I1B_fromI2C ();
	OstopI2C ();
	
	return iData;
}

static void rtc_DSP_WriteI2C (unsigned char addr,unsigned char odata)
{	
	OstartI2C ();
	O1B2I2C_wACK (_i2c_s_daddr);		//need it every time and when multiple registers ?
	O1B2I2C_wACK (addr);
	O1B2I2C_wACK (odata);
	OstopI2C ();
}

static void rtc_i2cInitSerialBus (bool bIsSIFmode, bool bIsEnableClkDivior, cyg_uint32 uFSB_Clk_Divisor)
{	
	cyg_uint32 regdata,GPIOctrlReg,clock_config;

	_i2c_bEnClockDivisor=(int)bIsEnableClkDivior;
	_i2c_FSB_clockDivisor=uFSB_Clk_Divisor;

	//k03255-1-GBD001-b
	regdata=inpw (REG_PADC0)|0x0e;	//GCR_BA+0x20 - turn on SDO, SDA, SCK
	outpw (REG_PADC0, regdata);
	//k03255-1-GBD001-a

	initSerialBus ((cyg_uint8)((int)bIsSIFmode&0x01));
	
}

static void rtc_i2cSetDeviceSlaveAddr (cyg_uint32 uSIF_Slave_Addr)
{	
	_i2c_s_daddr=uSIF_Slave_Addr;
}


static cyg_uint32 rtc_i2cReadI2C (cyg_uint32 uI2C_register_Addr)
{
	cyg_uint8 I2C_register_Data = 0;
	cyg_uint32 regvalue;

	if(!_eI2CType.bIsGPIO&&_eI2CType.uINTPin == 0)
	{
		outpw(0x7ff00020,inpw(0x7ff00020)|0xE);
		I2C_register_Data=rtc_DSP_ReadI2C (uI2C_register_Addr);
		outpw(0x7ff00020,inpw(0x7ff00020)&0xFFFFFFF1);
	}
	else if(!_eI2CType.bIsGPIO&&_eI2CType.uINTPin == 19)
	{
		I2C_register_Data=rtc_DSP_ReadI2C (uI2C_register_Addr);
	}
	else if(_eI2CType.bIsGPIO)
	{
		HAL_READ_UINT32(REG_GPIO_OE,regvalue);
		regvalue &= 0xFFFFFFDD;
		HAL_WRITE_UINT32(REG_GPIO_OE, regvalue);
		
		HAL_READ_UINT32(REG_GPIO_PE,regvalue);
		regvalue &= 0xFFFFFFDD;
		HAL_WRITE_UINT32(REG_GPIO_PE, regvalue);
		I2C_register_Data =rtc_DSP_ReadI2C (uI2C_register_Addr);
	}
    return ((cyg_uint32)I2C_register_Data);
}


static void rtc_i2cWriteI2C (cyg_uint32 uI2C_register_Addr, cyg_uint32 uI2C_register_Data)
{
	cyg_uint32 regvalue;
	if(!_eI2CType.bIsGPIO&&_eI2CType.uINTPin == 0)
	{
		outpw(0x7ff00020,inpw(0x7ff00020)|0xE);
		rtc_DSP_WriteI2C (uI2C_register_Addr,uI2C_register_Data);
		outpw(0x7ff00020,inpw(0x7ff00020)&0xFFFFFFF1);
		}
	else if(!_eI2CType.bIsGPIO&&_eI2CType.uINTPin == 19)
		rtc_DSP_WriteI2C (uI2C_register_Addr,uI2C_register_Data);
	else
	{
		HAL_READ_UINT32(REG_GPIO_OE,regvalue);
		regvalue &= 0xFFFFFFDD;
		HAL_WRITE_UINT32(REG_GPIO_OE, regvalue);
		
		HAL_READ_UINT32(REG_GPIO_PE,regvalue);
		regvalue &= 0xFFFFFFDD;
		HAL_WRITE_UINT32(REG_GPIO_PE, regvalue);
		rtc_DSP_WriteI2C (uI2C_register_Addr,uI2C_register_Data);
	}
}

static inline void set_rs_hwclock(cyg_uint32 year, cyg_uint32 month, cyg_uint32 mday,
               cyg_uint32 hour, cyg_uint32 minute, cyg_uint32 second)
{
	second &= 0x7f;
	minute &= 0x7f;
	hour &=0x3f;
	mday &= 0x3f;
	month &= 0x1f;
	month = TO_BCD(month);
	if( year > 99 )
	{
		//month &= 0x1f;
		year -=100;
	}
	else month |=0x80;
	
	rtc_i2cWriteI2C(RTC_SECOND, TO_BCD(second));//second
	rtc_i2cWriteI2C(RTC_MINUTE, TO_BCD(minute));//minutes
	rtc_i2cWriteI2C(RTC_HOUR, TO_BCD(hour));//hours
	rtc_i2cWriteI2C(RTC_DAY, TO_BCD(mday));//day
	rtc_i2cWriteI2C(RTC_MONTH,month);	//month
	rtc_i2cWriteI2C(RTC_YEAR, TO_BCD(year));//year
	if(_eI2CType.bIsGPIO)
		Get_RTC_time();
}

static void inline get_rs_hwclock(cyg_uint32 * year, cyg_uint32 * month,cyg_uint32 * mday,cyg_uint32 * hour, cyg_uint32 * minute, cyg_uint32 * second)
{
    cyg_uint32 i,j,k,l,m,y; 
    	
	i = rtc_i2cReadI2C(RTC_SECOND)&0x7f;
	j = rtc_i2cReadI2C(RTC_MINUTE)&0x7f;
	k = rtc_i2cReadI2C(RTC_HOUR)&0x3f;
	l = rtc_i2cReadI2C(RTC_DAY)&0x3f;
	m = rtc_i2cReadI2C(RTC_MONTH)&0x1f;
	y = rtc_i2cReadI2C(RTC_YEAR)&0xff;
	*mday = TO_DEC(l);
	*hour = TO_DEC(k); 
	*minute = TO_DEC(j); 
	*second = TO_DEC(i);
	*month = TO_DEC(m);
	
	if(rtc_i2cReadI2C(RTC_MONTH)&0x80)
	{
		*year = TO_DEC(y)+1900;
		}
	else 
	{
		*year = TO_DEC(y)+2000;
	}
}

cyg_uint32 Cyg_WallClock::get_hw_seconds(void)
{
    cyg_uint32 year, month, mday, hour, minute, second;
    cyg_uint32 now;

	if(!_eI2CType.bIsGPIO)
    {
    	get_rs_hwclock(&year, &month, &mday, &hour, &minute, &second);
    	if(month == 0)
    		month += 1;
    	if(mday == 0)
    		mday += 1; 
		now = _simple_mktime(year, month, mday, hour, minute, second);
    
    	return now;
    	}
    else
    	return TimeCounter +cyg_current_time()/100;
}

static cyg_uint32 alarm_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_uint32 volatile regvalue = 0;
	cyg_int32 mask = 0x0001;
	
	mask<<=_eI2CType.uINTPin;
	
	HAL_READ_UINT32(REG_GPIO_IS,regvalue);

	if(regvalue & mask)
	{
		regvalue = inpw(REG_GPIO_STS);
		
		if( (regvalue & mask) == 0)
		{
			timerhandler();
			if(!_eI2CType.bIsGPIO)
				rtc_i2cWriteI2C(RTC_CONTROL2,0x03);
		}
	
		HAL_WRITE_UINT32(REG_GPIO_IS,mask);
	}
	GPIO_INTHandler(vector,data);
	
	cyg_interrupt_acknowledge(RTC_ALARM_INT_NUM);
    return CYG_ISR_HANDLED;
	
}

void SetTimerHandler( TimerHandler *handler)
{
	timerhandler = handler;
}

static void interrupt_handler()
{
	
	//cyg_interrupt_configure(RTC_ALARM_INT_NUM,1,0);
	
	cyg_interrupt_create(RTC_ALARM_INT_NUM, 1, 0 , alarm_isr , NULL, &handle, &intr );
	
	cyg_interrupt_attach(handle);
	
	cyg_interrupt_unmask(RTC_ALARM_INT_NUM);
	
	cyg_interrupt_enable();

}

void Cyg_WallClock::init_hw_seconds()
{
	cyg_uint32 regvalue;
	
	
	HAL_READ_UINT32(REG_SYS_CFG,regvalue);
	regvalue &= 0xffffffef;
	HAL_WRITE_UINT32(REG_SYS_CFG, regvalue);
	
	HAL_READ_UINT32(REG_GPIO_PE,regvalue);
	regvalue |= 0x00000001;
	HAL_WRITE_UINT32(REG_GPIO_PE, regvalue);
	
	HAL_READ_UINT32(REG_CLKCON,regvalue);
	HAL_WRITE_UINT32(REG_CLKCON, regvalue | 0x40);//enable fsb clock
	
	rtc_i2cInitSerialBus(0,0,1);
	rtc_i2cSetDeviceSlaveAddr(RTC_SLAVE_ADDRESS);
	rtc_i2cWriteI2C(RTC_CONTROL1, 0x00);//ctrl_1
   	
   	rtc_i2cWriteI2C(RTC_CONTROL2,rtc_i2cReadI2C(RTC_CONTROL2)|0x03);
   	
   	interrupt_handler();
   	wallclock = &wallclock_instance;
   	
   	HAL_READ_UINT32(REG_GPIO_IS,regvalue);
	regvalue |= 0x00000001;
	HAL_WRITE_UINT32(REG_GPIO_IS,regvalue);
	
	regvalue = inpw(REG_GPIO_IE );
	regvalue |= 0x00000001;
	outpw(REG_GPIO_IE,regvalue);
}

void Cyg_WallClock::set_hw_seconds(cyg_uint32 time_stamp)
{
	cyg_uint32 year, month, mday, hour, minute, second;
	
	_simple_mkdate(time_stamp, &year, &month, &mday, &hour, &minute, &second);
	
	year -= 1900;
    set_rs_hwclock(year, month, mday, hour, minute, second);
}

static void inline set_alarm_dayofweek_reg(cyg_uint32 week_day)
{
	week_day &= 0x7;
	rtc_i2cWriteI2C(RTC_ALARM_WEEKDAY, week_day);//day of week
}

static void inline set_alarm_reg_day(cyg_uint32 mday)
{
	mday &= 0x3f;
	rtc_i2cWriteI2C(RTC_ALARM_DAY,TO_BCD(mday));//mday
}

static void inline set_alarm_reg_hour(cyg_uint32 hour)
{
	hour &= 0x3f;
	rtc_i2cWriteI2C(RTC_ALARM_HOUR, TO_BCD(hour));//hours
}

static void inline set_alarm_reg_minute(cyg_uint32 minute)
{	
	minute &= 0x7f;
	rtc_i2cWriteI2C(RTC_ALARM_MINUTE, TO_BCD(minute));//minutes
}

/*static void inline set_alarm_reg(cyg_uint32 mday,cyg_uint32 hour,cyg_uint32 minute)
{
	UINT32 regvalue;
	
	minute &= 0x7f;
	rtc_i2cWriteI2C(RTC_ALARM_MINUTE, TO_BCD(minute));//minutes
	hour &= 0x3f;
	rtc_i2cWriteI2C(RTC_ALARM_HOUR, TO_BCD(hour));//hours
	mday &= 0x3f;
	rtc_i2cWriteI2C(RTC_ALARM_DAY,TO_BCD(mday));//mday
}*/

void set_alarm(cyg_uint32 day,cyg_uint32 hour,cyg_uint32 minute, cyg_uint32 week_day)
{
	set_alarm_reg_day(day);
	set_alarm_reg_hour(hour);
	set_alarm_reg_minute(minute);
	set_alarm_dayofweek_reg(week_day);
}

void set_alarm_daily(cyg_uint32 hour,cyg_uint32 minute)
{
	cyg_uint32 regvalue;
	set_alarm_reg_hour(hour);
	set_alarm_reg_minute(minute);
	regvalue = rtc_i2cReadI2C(RTC_ALARM_DAY);
	regvalue |= 0x80;
	rtc_i2cWriteI2C(RTC_ALARM_DAY,regvalue);
	regvalue = rtc_i2cReadI2C(RTC_ALARM_WEEKDAY);
	regvalue |= 0x80;
	rtc_i2cWriteI2C(RTC_ALARM_WEEKDAY,regvalue);
}

void set_alarm_monthly(cyg_uint32 day,cyg_uint32 hour,cyg_uint32 minute)
{	
	cyg_uint32 regvalue;
	set_alarm_reg_day(day);
	set_alarm_reg_hour(hour);
	set_alarm_reg_minute(minute);
	regvalue = rtc_i2cReadI2C(RTC_ALARM_WEEKDAY);
	regvalue |= 0x80;
	rtc_i2cWriteI2C(RTC_ALARM_WEEKDAY,regvalue);
}

void set_alarm_weekly(cyg_uint32 hour,cyg_uint32 minute,cyg_uint32 week_day)
{
	cyg_uint32 regvalue;
	set_alarm_reg_hour(hour);
	set_alarm_reg_minute(minute);
	set_alarm_dayofweek_reg(week_day);
	regvalue = rtc_i2cReadI2C(RTC_ALARM_DAY);
	regvalue |= 0x80;
	rtc_i2cWriteI2C(RTC_ALARM_DAY,regvalue);
}

void get_alarmtime(cyg_uint32 * day,cyg_uint32 * hour,cyg_uint32 * minute,cyg_uint32 * weekday)
{
	cyg_uint32 i,j,k,w;
	
	i = rtc_i2cReadI2C(RTC_ALARM_MINUTE)&0x7f;
	j = rtc_i2cReadI2C(RTC_ALARM_HOUR)&0x3f;
	k = rtc_i2cReadI2C(RTC_ALARM_DAY)&0x3f;
	w = rtc_i2cReadI2C(RTC_ALARM_WEEKDAY)&0x7;
	
	*minute = TO_DEC(i);
	*hour = TO_DEC(j); 
	*day = TO_DEC(k);
	*weekday = TO_DEC(w);
}

void set_dayofweek(cyg_uint32 dayofweek)
{
	dayofweek &= 0x7;
	rtc_i2cWriteI2C(RTC_DAYOFWEEK,dayofweek);
}

static cyg_uint32 get_dayofweek()
{
	return rtc_i2cReadI2C(RTC_DAYOFWEEK);
}

cyg_uint32 get_control2()
{
	cyg_uint32 i;
	i = rtc_i2cReadI2C(RTC_CONTROL2);
	//i = TO_DEC(i);
	return i;
}

bool Init_RTC_INT()
{
	cyg_uint32 regvalue;
	
	if(_eI2CType.bIsGPIO == false && _eI2CType.uINTPin == 0)
	{
		//interrupt_handler();
		HAL_READ_UINT32(REG_SYS_CFG,regvalue);
		regvalue &= 0xffffffef;
		HAL_WRITE_UINT32(REG_SYS_CFG, regvalue);
		
		HAL_READ_UINT32(REG_GPIO_PE,regvalue);
		regvalue |= 0x00000001;
		HAL_WRITE_UINT32(REG_GPIO_PE, regvalue);

	   	HAL_READ_UINT32(REG_GPIO_IS,regvalue);
		regvalue |= 0x00000001;
		HAL_WRITE_UINT32(REG_GPIO_IS,regvalue);
		
		regvalue = inpw(REG_GPIO_IE );
		regvalue |= 0x00000001;
		outpw(REG_GPIO_IE,regvalue);
		return true;

	}
	else if(_eI2CType.bIsGPIO == false && _eI2CType.uINTPin == 19)
	{
		//interrupt_handler();
		HAL_READ_UINT32(REG_GPIO_PE,regvalue);
		regvalue |= 0x00008000;
		HAL_WRITE_UINT32(REG_GPIO_PE, regvalue);
		
	   	HAL_READ_UINT32(REG_GPIO_IS,regvalue);
		regvalue |= 0x00080000;
		HAL_WRITE_UINT32(REG_GPIO_IS,regvalue);
		
		regvalue = inpw(REG_GPIO_IE );
		regvalue |= 0x00080000;
		outpw(REG_GPIO_IE,regvalue);
		return true;
	}
	else if(_eI2CType.bIsGPIO == true)
	{
		return true;
	}
	else 
		return false;
}

cyg_int32 rtcSetI2CType(RTC_I2C_TYPE_T eI2CType)
{

	_eI2CType = eI2CType;
	
	Init_RTC_INT();
	
	if(eI2CType.bIsGPIO)
	{
		wSCK_MASK = 0x00000002;		//0000 0001b
		wSCK_UNMASK = 0xfffffffd; 
		wSDA_MASK = 0x00000020;		//0000 1000b
		wSDA_UNMASK = 0xffffffdf;		//0000 1000b

		rSCK_MASK = wSCK_MASK;
		rSCK_UNMASK = wSCK_UNMASK; 
		rSDA_MASK = wSDA_MASK;
		rSDA_UNMASK = wSDA_UNMASK;

	}
	if(eI2CType.ClearInterruptStatus)
	   	rtc_i2cWriteI2C(RTC_CONTROL2,0x03);
	return 0;
}

cyg_uint32 Get_RTC_time()
{
	cyg_uint32 year, month, mday, hour, minute, second;
    cyg_uint32 now;

    get_rs_hwclock(&year, &month, &mday, &hour, &minute, &second);
    if(month == 0)
    	month += 1;
    if(mday == 0)
    	mday += 1; 
	now = _simple_mktime(year, month, mday, hour, minute, second);
	TimeCounter = now-cyg_current_time()/100;
    
    return now;
}
