#ifndef INC__PERIODTASKS_H__
#define INC__PERIODTASKS_H__

void prdAddTask_CheckBattery(void);
void prdDelTask_CheckBattery(void);

void prdAddTask_CheckSuspend(void);
void prdDelTask_CheckSuspend(void);
void prdReset_CheckSuspend(void);

void prdAddTask_CheckNetwork(void);
void prdDelTask_CheckNetwork(void);

void prdAddTask_CheckUSB(void);
void prdDelTask_CheckUSB(void);


#ifndef LED_H
#define LED_H

enum LED_GPIO_E
{
	LED_GPIO_RED	= 0x01000000,
	LED_GPIO_GREEN	= 0x02000000
};


enum LED_ERROR_E
{
	LED_ERROR_WIFI			= 2,
	LED_ERROR_VIDEO_SENSOR	= 3,
	LED_ERROR_AUDIO_CODEC	= 4,
	LED_ERROR_MCU			= 5,
	LED_ERROR_NO_MEM		= 6
};


void ledInit(void);
void ledUninit(void);



/* Streaming / in used */
void ledShowState_Streaming(void);
void ledShowState_InUsed(void);
/* Streaming, low battery */
void ledShowState_Streaming_LowBattery(void);
/* Powered on / Ready / Connect to network */
void ledShowState_PoweredOn(void);
void ledShowState_Ready(void);
void ledShowState_NetworkReady(void);
/* Connecting / Booting / Attempt to connect */
void ledShowState_Connecting(void);
void ledShowState_Booting(void);
void ledShowState_TryConnect(void);
/* Connected to network and low battery */
void ledShowState_NetworkReady_LowBattery(void);
/* Try to connect and low battery */
void ledShowState_TryConnect_LowBattery(void);
/* Error message */
void ledShowState_Error(enum LED_ERROR_E nErrorCode);



enum LED_STATUS_E
{
	LED_ST_MASK_NETWORK		= 0x0F000000UL,
	LED_ST_MASK_ERROR_CODE	= 0x0000FFFFUL,

	LED_NETWORK_TRY_CONNECT	= 0x01000000UL,
	LED_NETWORK_READY		= 0x02000000UL,
	LED_NETWORK_CONNECTED	= 0x03000000UL,
	LED_NETWORK_STREAMING	= 0x04000000UL,
	LED_NETWORK_INIT		= 0x08000000UL,

	LED_ST_SET_DEFAULT		= 0x80000000UL,
	LED_ST_LOW_BATTERY		= 0x40000000UL,

	LED_ST_ERROR			= 0x00100000UL
};



void ledError(enum LED_ERROR_E nErrorCode);
void ledClearError(enum LED_ERROR_E eErrorCode);
void ledSetFactoryDefault(void);
void ledClearFactoryDefault(void);
void ledSetLowBattery(BOOL bLowBattery);
void ledSetNetwork(enum LED_STATUS_E eNetworkState);
UINT32 ledSuspend(void);
void ledWakeup(UINT32 uLED_GPIO);
void ledGetResult(int *pnLedStatus, int *pnLedErrorCode);


#endif

#endif
