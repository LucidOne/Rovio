#ifndef __CLK_H__
#define __CLK_H__




#define PLL_OUT_OFF			0xE220
#define PLL_OUT_BYPASS		0x16220




/************************
   define for CLKCON
 ************************/
#define DACCLK_EN		0x40000000
#define SECLK_EN		0x20000000
#define ACLK_EN			0x10000000

#define VCLK_EN			0x08000000
#define USBCLK_EN		0x04000000
#define CPUCLK_EN		0x02000000
#define HCLK2_EN		0x01000000

#define JPEGCLK_EN		0x00080000
#define _2DCLK_EN		0x00040000
#define VPECLK_EN		0x00020000
#define DSPCLK_EN		0x00010000

#define DISCLK_EN		0x00004000
#define FMICLK_EN		0x00002000
#define APBCLK_EN		0x00001000

//#define MP4ENC_EN		0x00000800
//#define MP4DEC_EN		0x00000400
#define MP4_EN			0x00000400

#define TCLK_EN			0x00000080
#define FSBCLK_EN		0x00000040
#define UR1CLK_EN_MASK	0x00000030
#define UR1CLK_EN		0x00000010	// assume crystal in

#define UR2CLK_EN_MASK	0x0000000C
#define UR2CLK_EN		0x00000004	// assume crystal in
#define WDCLK_EN_MASK	0x00000003
#define WDCLK_EN		0x00000003	// assume crystal in/256




/************************
   define for CLKSEL
 ************************/
#define SYSTEM_CLK_MASK			0x0300
#define SYSTEM_CLK_FROM_NONE	0x0000
#define SYSTEM_CLK_FROM_XTAL	0x0100
#define SYSTEM_CLK_FROM_APLL	0x0200
#define SYSTEM_CLK_FROM_UPLL	0x0300
#define SENSOR_CLK_MASK			0x00C0
#define SENSOR_CLK_FROM_NONE	0x0000
#define SENSOR_CLK_FROM_XTAL	0x0040
#define SENSOR_CLK_FROM_APLL	0x0080
#define SENSOR_CLK_FROM_UPLL	0x00C0
#define AUDIO_CLK_MASK			0x0030
#define AUDIO_CLK_FROM_NONE		0x0000
#define AUDIO_CLK_FROM_XTAL		0x0010
#define AUDIO_CLK_FROM_APLL		0x0020
#define AUDIO_CLK_FROM_UPLL		0x0030
#define VIDEO_CLK_MASK			0x000C
#define VIDEO_CLK_FROM_NONE		0x0000
#define VIDEO_CLK_FROM_XTAL		0x0004
#define VIDEO_CLK_FROM_APLL		0x0008
#define VIDEO_CLK_FROM_UPLL		0x000C
#define USB_CLK_MASK			0x0003
#define USB_CLK_FROM_NONE		0x0000
#define USB_CLK_FROM_XTAL		0x0001
#define USB_CLK_FROM_APLL		0x0002
#define USB_CLK_FROM_UPLL		0x0003





#define TURN_ON_TIMER_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | TCLK_EN);} while (0)
#define TURN_OFF_TIMER_CLOCK	do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~TCLK_EN)
#define TURN_ON_UART_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | (UR1CLK_EN | UR2CLK_EN));} while (0)
#define TURN_OFF_UART_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~(UR1CLK_EN | UR2CLK_EN));} while (0)
#define TURN_ON_JPEG_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | JPEGCLK_EN);} while (0)
#define TURN_OFF_JPEG_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~JPEGCLK_EN);} while (0)
#define TURN_ON_DSP_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | DSPCLK_EN);} while (0)
#define TURN_OFF_DSP_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~DSPCLK_EN);} while (0)
#define TURN_ON_VPE_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | VPECLK_EN);} while (0)
#define TURN_OFF_VPE_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & (~VPECLK_EN));} while (0)
#define TURN_ON_CAP_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | SECLK_EN);} while (0)
#define TURN_OFF_CAP_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & (~SECLK_EN));} while (0)
#define TURN_ON_MPEG4_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | MP4_EN);} while (0)
#define TURN_OFF_MPEG4_CLOCK	do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~MP4_EN);} while (0)
#define TURN_ON_VPOST_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | VCLK_EN);} while (0)
#define TURN_OFF_VPOST_CLOCK	do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~VCLK_EN);} while (0)
#define TURN_ON_GFX_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | _2DCLK_EN); } while (0)
#define TURN_OFF_GFX_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~_2DCLK_EN); } while (0)
#define TURN_ON_AUDIO_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) | ACLK_EN); } while (0)
#define TURN_OFF_AUDIO_CLOCK		do {outpw (REG_CLKCON, inpw (REG_CLKCON) & ~ACLK_EN); } while (0)




#endif
