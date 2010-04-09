/*
 * OnboardKey.c: Control keys on 740 board.
 * $Id: KeyInput.c,v 1.1 2006/01/17 09:42:12 xhchen Exp $
 *
 * Copyright (C) winbond co., Ltd.
 * All rights reserved.
 */


#include "../../Inc/Platform.h"


#define GPIO_CFG		0xFFF83000
#define GPIO_DIR		0xFFF83004
#define GPIO_DATAOUT	0xFFF83008
#define GPIO_DATAIN		0xFFF8300C
#define PINT32(x)		(*(volatile UINT32 *)(x))


/*
 * Name: bkeyInit
 * Description: Initialize the keys in the 740 board.
 *
 * No parameter.
 * No return value.
 */
VOID bkeyInit(VOID)
{
	UINT32 gpio_cfg = PINT32(GPIO_CFG);
	gpio_cfg &= 0xFFFFFFF0;

	PINT32(GPIO_CFG) = gpio_cfg;
	sysPrintf("Initialize onboard keys.\n");
}


/*
 * Name: gpioGet
 * Description: Read the gpio pins.
 *
 * Parameters:
 *	read_mask[in]: a mask for reading gpio.
 *	               bit 1, read, bit 0, not read.
 * Return value:
 *  The value of gpio.
 */
static UINT32 gpioGet(UINT32 read_mask)
{
	UINT32 value;
	UINT32 dir = PINT32(GPIO_DIR);
	dir &= ~read_mask;	/* gpio 2,3,4,5,6 for readding */
	PINT32(GPIO_DIR) = dir;
	
	value = PINT32(GPIO_DATAIN);
	value &= read_mask;
	return value;
}


/* Name: bkeyGetKey
 * Description: Read keys from gpio.
 * 
 * No parameter.
 * Return value:
 *  Return the value of onboard keys. If a key is down, the
 *  corresponding bit is 0, otherwise 1.
 */
UINT32 bkeyGetKey(VOID)
{
	UINT32 gpio;
	gpio = gpioGet(ONBOARD_KEY_GPIO_MASK);
	return gpio & ONBOARD_KEY_GPIO_MASK;
}


UINT32 bkeyReadKeyTimeout(int *piMilliSecond)
{
	static UINT32 key_last = 0;
	UINT32 key;
	int i;
	while (piMilliSecond == NULL
			|| *piMilliSecond >= 0)
	{
		//loops about 51 times per millisecond.
		//for (i = 0; i < 1524; i++)
		for (i = 0; i < 51; i++)
		{
			key = bkeyGetKey();
			if (key != key_last)
			{
				sysMSleep(1);
				key_last = key;
sysPrintf("Key: %x\n", key);				
				if (IS_KEY_UP_PRESSED(key))
					return ONBOARD_KEY_UP;
				else if (IS_KEY_DOWN_PRESSED(key))
					return ONBOARD_KEY_DOWN;
				else if (IS_KEY_LEFT_PRESSED(key))
					return ONBOARD_KEY_LEFT;
				else if (IS_KEY_RIGHT_PRESSED(key))
					return ONBOARD_KEY_RIGHT;
				else if (IS_KEY_MIDDLE_PRESSED(key))
					return ONBOARD_KEY_MIDDLE;
				else if (IS_KEY_LEFT_UP_PRESSED(key))
					return ONBOARD_KEY_LEFT_UP;
				else if (IS_KEY_RIGHT_DOWN_PRESSED(key))
					return ONBOARD_KEY_RIGHT_DOWN;
			}
		}
		(*piMilliSecond)--;
	}
	return 0;
}

