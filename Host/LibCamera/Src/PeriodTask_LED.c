#include "../Inc/CommonDef.h"



#if 0
/////////////////////////////////////////////////////////////////
//
// LED
//
/////////////////////////////////////////////////////////////////

enum LED_STYLE_E
{
	LED_OFF,
	LED_ON,
	LED_BLINK_LOW,
	LED_BLINK_HIGH,
	LED_BLINK_VERY_HIGH
};





#define BLINK_LOW_TICKS			40
#define BLINK_HIGH_TICKS		20
#define BLINK_VERY_HIGH_TICKS	5


typedef struct
{
	enum LED_STYLE_E	eStyle;
	BOOL				bIsOn;
	UINT32				uGPIO;
	UINT8				ucStopCount;
	UINT8				ucBlinkCount;
	UINT16				usStopTicks;
	UINT16				usShowTicks;
	PRD_TASK_T			*pTask;
} LED_TASK_STATUS_T;

static PRD_TASK_T g_prdtskUpdateLED_Red;
static PRD_TASK_T g_prdtskUpdateLED_Green;
static LED_TASK_STATUS_T g_status_Red;
static LED_TASK_STATUS_T g_status_Green;



static void prdTask_LED_Show(void *pArg);
static void prdTask_LED_Stop(void *pArg)
{
	LED_TASK_STATUS_T *pStatus = (LED_TASK_STATUS_T *) pArg;
	pStatus->ucBlinkCount = 0;

	/* Resume blinking */
	prdLock();
	prdDelTask(pStatus->pTask);
	prdAddTask(pStatus->pTask, &prdTask_LED_Show, (cyg_tick_count_t) pStatus->usShowTicks, pArg);
	prdUnlock();
}

static void prdTask_LED_Show(void *pArg)
{
	LED_TASK_STATUS_T *pStatus = (LED_TASK_STATUS_T *) pArg;
	
	switch (pStatus->eStyle)
	{
		case LED_BLINK_LOW:
		case LED_BLINK_HIGH:
		case LED_BLINK_VERY_HIGH:
			pStatus->bIsOn = (pStatus->bIsOn ? FALSE : TRUE);
			if (!pStatus->bIsOn)
				++pStatus->ucBlinkCount;
			
			if (pStatus->ucStopCount > 0 && pStatus->ucBlinkCount >= pStatus->ucStopCount)
			{ /* Stop blink for a while */
				prdLock();
				prdDelTask(pStatus->pTask);
				prdAddTask(pStatus->pTask, &prdTask_LED_Stop, (cyg_tick_count_t ) pStatus->usStopTicks, pArg);
				prdUnlock();
			}
			break;
		case LED_ON:
		case LED_OFF:
		default:
			;
	}

	if (pStatus->bIsOn)
	{
		outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~pStatus->uGPIO);
	}
	else
	{
		outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) | pStatus->uGPIO);
	}
}



static void ledSetSingleMethod (PRD_TASK_T *pTask,
	LED_TASK_STATUS_T *pStatus,
	UINT32 uGPIO,
	enum LED_STYLE_E eStyle,
	int nStopCount,
	int nStopTicks)
{
	cyg_tick_count_t tTimeout;

	prdLock();

	/*if (pStatus->eStyle	!= eStyle
		|| pStatus->uGPIO != uGPIO
		|| pStatus->ucStopCount	!= (UINT8) nStopCount
		|| pStatus->usStopTicks	!= (UINT16) nStopTicks
		|| pStatus->pTask != pTask)
	*/
	{

		pStatus->eStyle	= eStyle;
		pStatus->uGPIO	= uGPIO;
		pStatus->bIsOn	= FALSE;
		pStatus->ucStopCount	= (UINT8) nStopCount;
		pStatus->ucBlinkCount	= 0;
		pStatus->usStopTicks	= (UINT16) nStopTicks;
		pStatus->pTask			= pTask;

		switch (pStatus->eStyle)
		{
			case LED_BLINK_LOW:
				tTimeout = BLINK_LOW_TICKS;
				break;			
			case LED_BLINK_HIGH:
				tTimeout = BLINK_HIGH_TICKS;
				break;			
			case LED_BLINK_VERY_HIGH:
				tTimeout = BLINK_VERY_HIGH_TICKS;
				break;			
			case LED_ON:
				pStatus->bIsOn = TRUE;
			case LED_OFF:
			default:
				tTimeout = 60 * 100;
				break;					
		}
	
		pStatus->usShowTicks	= (UINT16) tTimeout;

		if (pStatus->bIsOn)
		{
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~pStatus->uGPIO);
		}
		else
		{
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) | pStatus->uGPIO);
		}

		prdDelTask(pTask);
		prdAddTask(pTask, &prdTask_LED_Show, (cyg_tick_count_t) pStatus->usShowTicks, (void *)pStatus);
	}
	
	prdUnlock();	
}


static void ledSetMethod(
	enum LED_STYLE_E eRed,
	int nRedStopCount,
	int nRedStopTicks,
	enum LED_STYLE_E eGreen,
	int nGreenStopCount,
	int nGreenStopTicks
	)
{
	prdLock();

	
	if (   g_status_Red.eStyle != eRed
		|| g_status_Red.ucStopCount != (UINT8) nRedStopCount
		|| g_status_Red.usStopTicks	!= (UINT16) nRedStopTicks
		|| g_status_Red.pTask != &g_prdtskUpdateLED_Red
		
		|| g_status_Green.eStyle != eGreen
		|| g_status_Green.ucStopCount != (UINT8) nGreenStopCount
		|| g_status_Green.usStopTicks	!= (UINT16) nGreenStopTicks
		|| g_status_Green.pTask != &g_prdtskUpdateLED_Green
		)
	{
	
		ledSetSingleMethod (&g_prdtskUpdateLED_Red,
			&g_status_Red,
			LED_GPIO_RED,
			eRed,
			nRedStopCount,
			nRedStopTicks);

		ledSetSingleMethod (&g_prdtskUpdateLED_Green,
			&g_status_Green,
			LED_GPIO_GREEN,
			eGreen,
			nGreenStopCount,
			nGreenStopTicks);
	}

	prdUnlock();
}





void ledInit()
{
	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~(UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); //set gpiob24 and gpio25 output mode
	outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | (UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); // set gpiob24 and gpio25 high(not light)
	
	diag_printf("set led not light\n");
	
	prdLock();
	memset(&g_status_Red, 0, sizeof(g_status_Red));
	memset(&g_status_Green, 0, sizeof(g_status_Green));
	prdAddTask(&g_prdtskUpdateLED_Red, &prdNothing, 60000, NULL);
	prdAddTask(&g_prdtskUpdateLED_Green, &prdNothing, 60000, NULL);
	prdUnlock();
}

void ledUninit()
{
	prdLock();
	prdDelTask(&g_prdtskUpdateLED_Green);
	prdDelTask(&g_prdtskUpdateLED_Red);
	prdUnlock();
}


/* Streaming / in used */
void ledShowState_Streaming()
{
	ledSetMethod(LED_ON, 0, 0, LED_OFF, 0, 0);
}

void ledShowState_InUsed()
{
	ledShowState_Streaming();
}

/* Streaming, low battery */
void ledShowState_Streaming_LowBattery()
{
	ledSetMethod(LED_BLINK_LOW, 0, 0, LED_OFF, 0, 0);
}

/* Powered on / Ready / Connect to network */
void ledShowState_PoweredOn()
{
	ledSetMethod(LED_ON, 0, 0, LED_ON, 0, 0);
}

void ledShowState_Ready()
{
	ledSetMethod(LED_OFF, 0, 0, LED_ON, 0, 0);
}

void ledShowState_NetworkReady()
{
	ledShowState_Ready();
}

/* Connecting / Booting / Attempt to connect */
void ledShowState_Connecting()
{
	ledSetMethod(LED_ON, 0, 0, LED_ON, 0, 0);
}

void ledShowState_Booting()
{
	//ledShowState_Connecting();
	ledSetMethod(LED_ON, 0, 0, LED_ON, 0, 0);
}

void ledShowState_TryConnect()
{
	ledShowState_Connecting();
}

/* Connected to network and low battery */
void ledShowState_NetworkReady_LowBattery()
{
	ledSetMethod(LED_OFF, 0, 0, LED_BLINK_LOW, 0, 0);
}

/* Try to connect and low battery */
void ledShowState_TryConnect_LowBattery()
{
	ledSetMethod(LED_BLINK_LOW, 0, 0, LED_BLINK_LOW, 0, 0);
}


/* Error message */
void ledShowState_Error(enum LED_ERROR_E eErrorCode)
{
	ledSetMethod(LED_BLINK_LOW,	eErrorCode, 400, LED_BLINK_VERY_HIGH, 0, 0);
}


/* LED status contains:
   Network
   Battery
   Error
 */


static UINT32 g_uLED_STATUS = 0;


static void ledUpdateStatus()
{
	prdLock();

	if ((g_uLED_STATUS & LED_ST_ERROR) != 0)
		ledShowState_Error( g_uLED_STATUS & LED_ST_MASK_ERROR_CODE );
	else if ((g_uLED_STATUS & LED_ST_LOW_BATTERY ) != 0)
	{
		switch (g_uLED_STATUS & LED_ST_MASK_NETWORK)
		{
			case LED_NETWORK_TRY_CONNECT:
				ledShowState_TryConnect_LowBattery();
				break;
			case LED_NETWORK_READY:
				ledShowState_NetworkReady_LowBattery();
				break;
			case LED_NETWORK_CONNECTED:
				ledShowState_NetworkReady_LowBattery();
				break;
			case LED_NETWORK_STREAMING:
				ledShowState_Streaming_LowBattery();
				break;
			case LED_NETWORK_INIT:
				ledShowState_Booting();
				break;			
			default:
				ledShowState_TryConnect_LowBattery();
		}
	}
	else
	{
		switch (g_uLED_STATUS & LED_ST_MASK_NETWORK)
		{
			case LED_NETWORK_TRY_CONNECT:
				ledShowState_TryConnect();
				break;
			case LED_NETWORK_READY:
				ledShowState_Ready();
				break;
			case LED_NETWORK_CONNECTED:
				ledShowState_NetworkReady();
				break;
			case LED_NETWORK_STREAMING:
				ledShowState_Streaming();
				break;
			case LED_NETWORK_INIT:
				ledShowState_Booting();
				break;
			default:
				ledShowState_TryConnect();
		}	
	}	
	
	prdUnlock();
}



void ledError(enum LED_ERROR_E eErrorCode)
{
	prdLock();
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_MASK_ERROR_CODE ) | ( eErrorCode & LED_ST_MASK_ERROR_CODE )
		| LED_ST_ERROR;
	ledUpdateStatus();	
	prdUnlock();
}


void ledSetLowBattery(BOOL bLowBattery)
{
	prdLock();
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_LOW_BATTERY ) | ( bLowBattery ? LED_ST_LOW_BATTERY : 0 );
	ledUpdateStatus();	
	prdUnlock();
}


void ledSetNetwork(enum LED_STATUS_E eNetworkState)
{
	prdLock();
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_MASK_NETWORK ) | ( eNetworkState & LED_ST_MASK_NETWORK );
	ledUpdateStatus();	
	prdUnlock();	
}


UINT32 ledSuspend()
{
	UINT32 uLED_GPIO;
	prdLock();
	
	uLED_GPIO = inpw(REG_GPIOB_DAT);
	//outpw(REG_GPIOB_DAT, (uLED_GPIO | (UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)));
	
	return uLED_GPIO;
}

void ledWakeup(UINT32 uLED_GPIO)
{
	//outpw(REG_GPIOB_DAT, uLED_GPIO);
	prdUnlock();
}


void ledGetResult(int *pnLedStatus, int *pnLedErrorCode)
{
	prdLock();
	*pnLedStatus = g_uLED_STATUS & ~(UINT32)LED_ST_MASK_ERROR_CODE;
	*pnLedErrorCode = g_uLED_STATUS & (UINT32)LED_ST_MASK_ERROR_CODE;
	prdUnlock();
}




#else
////////////////////////////////////////////////////////////////////////////




#define LED_BLACK	((unsigned short)0)
#define LED_RED		((unsigned short)1)
#define LED_GREEN	((unsigned short)2)
#define LED_ORANGE	((unsigned short)3)

typedef struct
{
	unsigned short usColor;
	unsigned short usTime;	/* 0 for never change */
} LED_DES_T;



#define LED_DES_RESET_DEFAULT \
{ {LED_RED, 20}, {LED_GREEN, 20}, {LED_ORANGE, 20} }

#define LED_DES_BOOTING \
{ {LED_ORANGE, 0} }
//{ {LED_RED, 20}, {LED_GREEN, 20} }

#define LED_DES_BOOTING_LOW_BATTERY \
{ {LED_ORANGE, 40}, {LED_BLACK, 40} }
//{ {LED_RED, 20}, {LED_GREEN, 20}, {LED_BLACK, 20} }

#define LED_DES_STREAMING \
{ {LED_RED, 0} }

#define LED_DES_STREAMING_LOW_BATTERY \
{ {LED_RED, 40}, {LED_BLACK, 40} }

#define LED_DES_READY \
{ {LED_GREEN, 0} }

#define LED_DES_READY_LOW_BATTERY \
{ {LED_GREEN, 40}, {LED_BLACK, 40} }

#define LED_DES_CONNECTING \
{ {LED_ORANGE, 0} }

#define LED_DES_CONNECTING_LOW_BATTERY \
{ {LED_ORANGE, 40}, {LED_BLACK, 40} }




typedef struct
{
	LED_DES_T		aDescriptor[128];
	unsigned char	ucDesNum;
	unsigned char	ucDesCurrent;
	PRD_TASK_T			*pTask;
} LED_TASK_STATUS_T;


static PRD_TASK_T g_prdtskUpdateLED;
static LED_TASK_STATUS_T g_status_LED;
static UINT32 g_uLED_STATUS = 0;
static TT_RMUTEX_T g_rmLED;


static void prdTask_LED(void *pArg)
{
	LED_TASK_STATUS_T *pStatus = (LED_TASK_STATUS_T *) pArg;

	LED_DES_T *pDescriptor = &pStatus->aDescriptor[pStatus->ucDesCurrent];
	if (++pStatus->ucDesCurrent >= pStatus->ucDesNum)
		pStatus->ucDesCurrent = 0;

	prdLock();
	prdDelTask(pStatus->pTask);
	prdAddTask(pStatus->pTask, &prdTask_LED,
		(cyg_tick_count_t) (pDescriptor->usTime != 0 ? pDescriptor->usTime : 60000),
		pArg);
	prdUnlock();
	
	switch (pDescriptor->usColor)
	{
		case LED_BLACK:
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) | LED_GPIO_RED | LED_GPIO_GREEN);
			break;
		case LED_RED:
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~LED_GPIO_RED | LED_GPIO_GREEN);
			break;
		case LED_GREEN:
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~LED_GPIO_GREEN | LED_GPIO_RED);
			break;
		case LED_ORANGE:
		default:
			outpw(REG_GPIOB_DAT, inpw(REG_GPIOB_DAT) & ~LED_GPIO_RED & ~LED_GPIO_GREEN);
			break;
			;
	}
}


static void ledShowState(LED_DES_T const *pDescriptor, size_t szDesLen)
{
	tt_rmutex_lock(&g_rmLED);
	if (g_status_LED.ucDesNum != (unsigned char)szDesLen
		|| memcmp(g_status_LED.aDescriptor, pDescriptor, sizeof(LED_DES_T) * szDesLen) != 0
		|| g_status_LED.pTask != &g_prdtskUpdateLED
		)
	{
		if (szDesLen < sizeof(g_status_LED.aDescriptor) / sizeof(g_status_LED.aDescriptor[0]))
		{
			memcpy(g_status_LED.aDescriptor, pDescriptor, sizeof(LED_DES_T) * szDesLen);
			g_status_LED.ucDesNum = (unsigned char)szDesLen;
			g_status_LED.ucDesCurrent = 0;
			g_status_LED.pTask = &g_prdtskUpdateLED;
			prdLock();
			prdDelTask(&g_prdtskUpdateLED);
			prdAddTask(&g_prdtskUpdateLED, &prdTask_LED, (cyg_tick_count_t)0, (void *)&g_status_LED);
			prdUnlock();
		}
	}
	tt_rmutex_unlock(&g_rmLED);
}





static void ledUpdateStatus();
void ledInit()
{
	outpw(REG_GPIOB_OE,inpw(REG_GPIOB_OE) & ~(UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); //set gpiob24 and gpio25 output mode
	outpw(REG_GPIOB_DAT,inpw(REG_GPIOB_DAT) | (UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)); // set gpiob24 and gpio25 high(not light)
	
	diag_printf("set led not light\n");
	
	tt_rmutex_init(&g_rmLED);
	memset(&g_status_LED, 0, sizeof(g_status_LED));
	
	//prdLock();
	prdAddTask(&g_prdtskUpdateLED, &prdNothing, 60000, NULL);
	//prdUnlock();
}


void ledUninit()
{
	//prdLock();
	prdDelTask(&g_prdtskUpdateLED);
	//prdUnlock();
}



/* Set to default */
void ledShowState_SetDefault()
{
	const static LED_DES_T alDes[] = LED_DES_RESET_DEFAULT;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}


/* Streaming / in used */
void ledShowState_Streaming()
{
	const static LED_DES_T alDes[] = LED_DES_STREAMING;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

void ledShowState_InUsed()
{
	ledShowState_Streaming();
}

/* Streaming, low battery */
void ledShowState_Streaming_LowBattery()
{
	const static LED_DES_T alDes[] = LED_DES_STREAMING_LOW_BATTERY;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

/* Powered on / Ready / Connect to network */
void ledShowState_PoweredOn()
{
	const static LED_DES_T alDes[] = LED_DES_BOOTING;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

void ledShowState_Ready()
{
	const static LED_DES_T alDes[] = LED_DES_READY;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

void ledShowState_NetworkReady()
{
	ledShowState_Ready();
}

/* Connecting / Booting / Attempt to connect */
void ledShowState_Connecting()
{
	const static LED_DES_T alDes[] = LED_DES_CONNECTING;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

void ledShowState_Booting()
{
	const static LED_DES_T alDes[] = LED_DES_BOOTING;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));	
}

void ledShowState_Booting_LowBattery()
{
	const static LED_DES_T alDes[] = LED_DES_BOOTING_LOW_BATTERY;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));	
}

void ledShowState_TryConnect()
{
	ledShowState_Connecting();
}

/* Connected to network and low battery */
void ledShowState_NetworkReady_LowBattery()
{
	const static LED_DES_T alDes[] = LED_DES_READY_LOW_BATTERY;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}

/* Try to connect and low battery */
void ledShowState_TryConnect_LowBattery()
{
	const static LED_DES_T alDes[] = LED_DES_CONNECTING_LOW_BATTERY;
	ledShowState(alDes, sizeof(alDes) / sizeof(alDes[0]));
}


void ledShowState_Error(enum LED_ERROR_E eErrorCode)
{
	LED_DES_T alDes[128];
	int i, j, n;
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	
	n = 0;
	for (i = 0; i < (int)eErrorCode; ++i)
	{
		for (j = 0; j < 6; j++)
		{
			if (n < sizeof(alDes) / sizeof(alDes[0]))
			{
				alDes[n].usColor	= LED_GREEN;
				alDes[n].usTime	= 5;
				n++;
			}
			if (n < sizeof(alDes) / sizeof(alDes[0]))
			{
				alDes[n].usColor	= LED_RED;
				alDes[n].usTime	= 5;
				n++;
			}
		}

		if (n < sizeof(alDes) / sizeof(alDes[0]))
		{
			alDes[n].usColor	= LED_ORANGE;
			alDes[n].usTime	= 150;
			n++;
		}
	}
	if (n < sizeof(alDes) / sizeof(alDes[0]))
	{
		alDes[n].usColor	= LED_BLACK;
		alDes[n].usTime	= 150;
		n++;
	}

	ledShowState(alDes, n);	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}






/* LED status contains:
   Network
   Battery
   Error
 */


static void ledUpdateStatus()
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);

	if ((g_uLED_STATUS & LED_ST_ERROR) != 0)
		ledShowState_Error( g_uLED_STATUS & LED_ST_MASK_ERROR_CODE );
	else if ((g_uLED_STATUS & LED_ST_SET_DEFAULT ) != 0)
		ledShowState_SetDefault();
	else if ((g_uLED_STATUS & LED_ST_LOW_BATTERY ) != 0)
	{
		switch (g_uLED_STATUS & LED_ST_MASK_NETWORK)
		{
			case LED_NETWORK_TRY_CONNECT:
				ledShowState_TryConnect_LowBattery();
				break;
			case LED_NETWORK_READY:
				ledShowState_NetworkReady_LowBattery();
				break;
			case LED_NETWORK_CONNECTED:
				ledShowState_NetworkReady_LowBattery();
				break;
			case LED_NETWORK_STREAMING:
				ledShowState_Streaming_LowBattery();
				break;
			case LED_NETWORK_INIT:
				ledShowState_Booting_LowBattery();
				break;			
			default:
				ledShowState_TryConnect_LowBattery();
		}
	}
	else
	{
		switch (g_uLED_STATUS & LED_ST_MASK_NETWORK)
		{
			case LED_NETWORK_TRY_CONNECT:
				ledShowState_TryConnect();
				break;
			case LED_NETWORK_READY:
				ledShowState_Ready();
				break;
			case LED_NETWORK_CONNECTED:
				ledShowState_NetworkReady();
				break;
			case LED_NETWORK_STREAMING:
				ledShowState_Streaming();
				break;
			case LED_NETWORK_INIT:
				ledShowState_Booting();
				break;
			default:
				ledShowState_TryConnect();
		}	
	}	
	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}



void ledError(enum LED_ERROR_E eErrorCode)
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_MASK_ERROR_CODE ) | ( eErrorCode & LED_ST_MASK_ERROR_CODE )
		| LED_ST_ERROR;
	ledUpdateStatus();	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}



void ledClearError(enum LED_ERROR_E eErrorCode)
{
	tt_rmutex_lock(&g_rmLED);
	if ((g_uLED_STATUS | LED_ST_ERROR) != 0
		&& (g_uLED_STATUS & LED_ST_MASK_ERROR_CODE) == (eErrorCode & LED_ST_MASK_ERROR_CODE))
	{
		//prdLock();
		/* Recheck again */
		if ((g_uLED_STATUS | LED_ST_ERROR) != 0
			&& (g_uLED_STATUS & LED_ST_MASK_ERROR_CODE) == (eErrorCode & LED_ST_MASK_ERROR_CODE))
		{
			g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_MASK_ERROR_CODE & ~LED_ST_ERROR );
			ledUpdateStatus();
		}
		//prdUnlock();	
	}
	tt_rmutex_unlock(&g_rmLED);
}


void ledSetFactoryDefault()
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	g_uLED_STATUS = ( g_uLED_STATUS | LED_ST_SET_DEFAULT );
	ledUpdateStatus();	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}


void ledClearFactoryDefault()
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_SET_DEFAULT );
	ledUpdateStatus();	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}



void ledSetLowBattery(BOOL bLowBattery)
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_LOW_BATTERY ) | ( bLowBattery ? LED_ST_LOW_BATTERY : 0 );
	ledUpdateStatus();	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}


void ledSetNetwork(enum LED_STATUS_E eNetworkState)
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	g_uLED_STATUS = ( g_uLED_STATUS & ~LED_ST_MASK_NETWORK ) | ( eNetworkState & LED_ST_MASK_NETWORK );
	ledUpdateStatus();	
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}


UINT32 ledSuspend()
{
	UINT32 uLED_GPIO;
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	
	uLED_GPIO = inpw(REG_GPIOB_DAT);
	//outpw(REG_GPIOB_DAT, (uLED_GPIO | (UINT32)(LED_GPIO_RED | LED_GPIO_GREEN)));
	
	return uLED_GPIO;
}

void ledWakeup(UINT32 uLED_GPIO)
{
	//outpw(REG_GPIOB_DAT, uLED_GPIO);
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}


void ledGetResult(int *pnLedStatus, int *pnLedErrorCode)
{
	//prdLock();
	tt_rmutex_lock(&g_rmLED);
	*pnLedStatus = g_uLED_STATUS & ~(UINT32)LED_ST_MASK_ERROR_CODE;
	*pnLedErrorCode = g_uLED_STATUS & (UINT32)LED_ST_MASK_ERROR_CODE;
	//prdUnlock();
	tt_rmutex_unlock(&g_rmLED);
}


#endif
