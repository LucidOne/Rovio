#ifndef __W99685CMDS_H__
#define __W99685CMDS_H__

#define SDSUPPORT

#define DUMMY 				((UINT16)0xff)
#define GENERAL_CMD 			((UINT16)0x57)
#define DEBUG_CMD			((UINT16)0x58)

#ifdef SDSUPPORT
	#define SD_CMD			((UINT16)0x59)
	
	#define CMD_CARDINSERTED	0x00
#endif

#define REG_DEBUG			0x01
#define REG_READ			0x00
#define REG_WRITE			0x01
////////////////////////////////
// Cmd ID
#define CMD_GETVERSION			0x00
#define CMD_INITIALIZE			0x01
#define CMD_GETJPGBITS			0x02
#define CMD_STANDBY			0x03
#define CMD_SETAEC			0x04
#define CMD_GETJPGFMT			0x05
#define CMD_SETJPGFMT			0x06
#define CMD_RSTJPGENG			0x07
#define CMD_GETMAXRES			0x08
#define CMD_SETIMGRES			0x09
#define CMD_GETIMGRES			0x0A
#define CMD_GETRSTIVL			0x0B
#define CMD_I2CWRITE			0x0C
#define CMD_I2CREAD			0x0D
#define CMD_SETQTBLIDX			0x0E
#define CMD_GETQRANGE			0x0F
// ...                  	
#define CMD_SETOPMODE			0x40
#define CMD_SETPRVSIZ			0x41
#define CMD_GETPRVDAT			0x42
#define CMD_SNAPSHOT			0x43
#define CMD_SNDJPGBITS			0x44
#define CMD_DECJPGBITS			0x45
#define CMD_GETDJPGINF			0x46
#define CMD_GETDJPGDAT			0x47
#define CMD_SETDECSIZE			0x48
#define CMD_GETROTCAP			0x49
#define CMD_SETROT			0x4A
#define CMD_DOWNSIZE			0x4B
#define CMD_SETZOOM			0x4C
// ...
#define CMD_SETLCMNUM			0x50
#define CMD_SELECTLCM			0x51
#define CMD_SETLCMRES			0x52
#define CMD_SETLCMWNDPOS		0x53
#define CMD_SETLCMWNDSIZ		0x54
#define CMD_WRITELCMWBUF		0x55
#define CMD_SUSPENDLCM			0x56
#define CMD_SELECTMODE			0x57
#define CMD_PREVIEW_CTRL		0x58
#define CMD_GETSNAPSTATUS		0x59
#define CMD_SETFRAMERATE		0x5A
#define CMD_SETGRID			0x5B
#define CMD_GETFRAMESNUM		0x5C
#define CMD_SENDVCLIPHI			0x5D
// ...
#define CMD_SETIMGEFFECT		0x5F
// ...
#define CMD_SETPKTMSKRES		0x62
#define CMD_SETPLNMSKRES		0x63
#define CMD_SENDPKTMSK			0x64
#define CMD_SENDPLNMSK			0x65
#define CMD_SENDPATTERN			0x66
#define CMD_DISSTICKMAKER		0x67
#define CMD_GRAPHICCTRL			0x68
#define CMD_SETOSDKPATMSK		0x69
#define CMD_SETOSDPATTERN		0x6A
#define CMD_BLINKINGRATE		0x6B
#define CMD_BLINKINGCTRL		0x6C
#define CMD_BLENDINGWEIGHT		0x6D
#define CMD_BLENDINGCTRL		0x6E

#define CMD_DUMMY			0xFF	// reserved it, just for read data
////////////////////////////////
// Cmd parameters 
#define FMT_YUV420			0
#define FMT_YUV422			1
                        	
#define AEC_50HZ			2
#define AEC_60HZ			1
#define AEC_NIGHT			3
                        	
// Image resolution     	
#define RES_SXGA2			0
#define RES_SXGA			1
#define RES_VGA				2
#define RES_CIF				3
#define RES_QVGA			4
#define RES_QCIF			5
#define RES_SQVGA			6
#define RES_12896			7
                        	
// Rotation type        	
#define ROT_FLIP			0x20
#define ROT_MIRROR			0x10
#define ROT_270				0x04
#define ROT_180				0x02
#define ROT_90				0x01
                        	
// op mode              	
#define OP_SINGLE			0x00
#define OP_MULTICAP			0x01
#define OP_MOVIE			0x02
#define OP_PLAYBACK			0x03
#define OP_STREAMING			0x06
                        	
#define PPC_OFF				0x00
#define PPC_YUV422			0x01
#define PPC_RGB565			0x02
#define PPC_RGB555			0x03
#define PPC_RGB444			0x04
#define PPC_RGB332			0x05

//LCM
#define LCD_ON_HOST			0x00
#define LCD1_ON_685			0x01
#define LCD2_ON_685			0x02

#define LCD_BUS_16			0x00
#define LCD_BUS_8			0x01

#define LCM_1ST				0x01
#define LCM_2ND				0x02

#define PREVIEW_START			0x01
#define PREVIEW_STOP			0x00

#define EFFECT_NORMAL			0x00
#define EFFECT_BW			0x01
#define EFFECT_SEPIA			0x02
#define EFFECT_NEGATIVE			0x03
#define EFFECT_SOLARIZE			0x04
#define EFFECT_TEXT			0x05

#endif
