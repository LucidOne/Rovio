/*
 * OnboardKey.h: Control keys on 740 board.
 * $Id: KeyInput.h,v 1.1 2006/01/17 09:42:12 xhchen Exp $
 *
 * Copyright (C) winbond co., Ltd.
 * All rights reserved.
 */


VOID bkeyInit(VOID);
UINT32 bkeyGetKey(VOID);
UINT32 bkeyReadKeyTimeout(int *piMilliSecond);


#define ONBOARD_KEY_GPIO_MASK 0x000040FC
#define ONBOARD_KEY_UP 0x00000004
#define ONBOARD_KEY_DOWN 0x00000008
#define ONBOARD_KEY_LEFT 0x00000010
#define ONBOARD_KEY_RIGHT 0x00000020
#define ONBOARD_KEY_MIDDLE 0x0000040
#define ONBOARD_KEY_LEFT_UP 0x00004000
#define ONBOARD_KEY_RIGHT_DOWN 0x00000080


#define IS_KEY_UP_PRESSED(gpio)	\
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_UP) == 0)

#define IS_KEY_DOWN_PRESSED(gpio)	\
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_DOWN) == 0)

#define IS_KEY_LEFT_PRESSED(gpio)	\
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_LEFT) == 0)

#define IS_KEY_RIGHT_PRESSED(gpio)	\
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_RIGHT) == 0)

#define IS_KEY_MIDDLE_PRESSED(gpio)	\
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_MIDDLE) == 0)

#define IS_KEY_LEFT_UP_PRESSED(gpio) \
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_LEFT_UP) == 0)	
#define IS_KEY_RIGHT_DOWN_PRESSED(gpio) \
	(((gpio) & ONBOARD_KEY_GPIO_MASK & ONBOARD_KEY_RIGHT_DOWN) == 0)	


