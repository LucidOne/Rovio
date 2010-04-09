#include "fontbypass.h"

int ConfigureLCM(int fd)
{
	printf("mwConfigureLCM++\n");
	if(wb702ConfigureLCM(128, 160, CMD_LCM_COLOR_WIDTH_18,
		CMD_LCM_CMD_BUS_WIDTH_16, CMD_LCM_DATA_BUS_WIDTH_18, 0,
		TRUE, CMD_LCM_80_MPU)<0)
	{
		printf("mwConfigureLCM error \n");
		return 1;
	}
	return 0;
}

int SetOSDColor(int fd)
{
	printf("mwSetOSDColor++\n");
	if(wb702SetOSDColor(CMD_LCM_OSD_RGB565, 0x00, 0x00, 0x00,0xF8,0xFC,0xF8)<0)
	{
		printf("mwSetOSDColor error\n");
		return 1;
	}
	return 0;
}

int SetLocalVideoSource(int fd)
{
	printf("mwSetLocalVideoSource++\n");

	if(wb702SetLocalVideoSource(176, 144, CMD_ROTATE_R180)<0)
	{
		printf("mwSetLocalVideoSource error\n");
		return 1;
	}
	return 0;
}

int SetFrame(int fd,WB_FRAME_TYPE frame)
{
	printf("mwEnableOldMode++\n");
	if(wb702SetFrame(frame)<0)
	{
		printf("mwEnableOldMode error\n");
		return 1;
	}
	return 0;
}

int SetVideoFormat(int fd,CMD_VIDEO_FORMAT_E format)
{
	printf("mwSetVideoFormat++\n");
	if(wb702SetVideoFormat(format)<0)
	{
		printf("mwSetVideoFormat error\n");
		return 1;
	}
	return 0;
}
int wb702Init(int fd)
{	
	SetLocalVideoSource(fd);

	wb702EnableMP4Encoder(TRUE);
	wb702EnableAudioEncoder(TRUE);
	
	printf("init complete!!!\n");
	return 0;
}

void drawLCDRGB(UCHAR *buf, int size)
{

	/* Draw 1st LCD screen as:
	----------------
	|    RED        |
	|--------------|
	|    GREEN   |
	|--------------|
	|    BLUE      |
	----------------
	*/
	int i = 0;
	USHORT usColor;
	while (i < size * 1 / 3)
	{/* 1/3 buffer => red, RGB(0xFF, 0x00, 0x00). */
		usColor = 0x0000;
		buf[i++] = (UCHAR) usColor;
		buf[i++] = (UCHAR) (usColor >> 8);
	}
	while (i < size* 2 / 3)
	{/* following 1/3 buffer => green, RGB(0x00, 0xFF, 0x00). */
		usColor = 0x0000;
		buf[i++] = (UCHAR) usColor;
		buf[i++] = (UCHAR) (usColor >> 8);
	}
	while (i < size* 3 / 3)
	{/* following 1/3 buffer => blue, RGB(0x00, 0x00, 0xFF). */
		usColor = 0x0000;
		buf[i++] = (UCHAR) usColor;
		buf[i++] = (UCHAR) (usColor >> 8);
	}
	/* Draw "g_aucLCM1_Buffer" to 1st LCD screen. */
}

void drawLCDYP(UCHAR *buf, int size)
{
	int i = 0;
	USHORT usColor;
	
	while (i < size * 1 / 2)
	{/* 1/3 buffer => red, RGB(0xFF, 0xFF, 0x00). */
		usColor = 0xFFE0;
		buf[i++] = (UCHAR) usColor;
		buf[i++] = (UCHAR) (usColor >> 8);
	}
	while (i < size * 2 / 2)
	{/* following 1/3 buffer => purple, RGB(0xFF, 0x00, 0xFF). */
		usColor = 0xF81F;
		buf[i++] = (UCHAR) usColor;
		buf[i++] = (UCHAR) (usColor >> 8);
	}
}
