#ifndef INC_H
#define INC_H
#ifndef ECOS
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#else
#include "stdio.h"
#include "stdlib.h"

#include "assert.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "time.h"
#include "cyg/infra/diag.h"
#include "wb_syslib_addon.h"
#endif

/* MAX is needed by vp_buffer.h */
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#include "../../custom_config.h"

#include "define.h"

#include "../../../libTinyThread/Inc/tt_thread.h"
#include "../../../VPE/Inc/vpelib.h"
#include "../../../VPOST/Inc/VPOST.H"
#include "../../../Audio/Inc/W99702_Audio.h"
#ifndef ECOS
#include "../../../AECEEC/rom/inc/AECEEClib.h"
#include "../../../AMR/inc/amrdec.h"
#include "../../../AMR/inc/amrenc.h"
#else
#include "../../../AECEEC/rom/inc/AECEEClib.h"
#include "../../../AMR/Include/amrdec.h"
#include "../../../AMR/Include/amrenc.h"
#endif
#include "../../../I2C/inc/i2clib.h"
#include "../../../DSP/inc/dsplib.h"
#include "../../../VCE/inc/caplib.h"
#include "../../../GFX/inc/gfxlib.H"
#include "../../../MP4/inc/mp4lib.h"
#include "../../../libCmpImg/Inc/cmp_img.h"
#include "../../../libIMA_ADPCM/Inc/ima_adpcm.h"
//#include "../../../libUsbVCom/Inc/gpio.h"
//#include "../../../libUsbVCom/Inc/usb.h"
#include "../../../JPEG/Inc/Jpeglib.h"
#include "../../../libFont/inc/font.h"

#include "../../../../Baseband/Libs/HIC/Inc/InInc/HICErrorCode.h"
//#include "../../../../Baseband/Libs/HIC/Inc/InInc/HIC_FIFO.h"
//#include "../../../../Baseband/Libs/WString/Inc/WString.h"
#ifndef ECOS
#include "../../../../Baseband/Libs/CmdSet/Inc/InInc/Commands_VideoPhone.h"
#include "../../../../Baseband/Libs/CmdSet/Inc/InInc/Commands_CameraModule.h"	//For compatibility
#else
#include "../../../SysLib/Inc/Commands_VideoPhone.h"
#include "../../../SysLib/Inc/Commands_CameraModule.h"	//For compatibility
#endif
#include "../../../libAULaw/Inc/aulaw_interface.h"
#include "../../../powerctrl/inc/702clk.h"
#include "../../../LibCamera/Inc/PeriodTasks.h"

#ifndef ECOS
#include "debug.h"
#endif
#include "pll.h"
#include "clk.h"
#include "scaling.h"
#ifndef ECOS
#include "my_time.h"
#endif

#include "vp_test.h"
#include "vp_buffer.h"
#include "vp_lcm.h"
#include "vp_vpe.h"
#include "vp_gfx.h"
#include "vp_audio.h"
#include "vp_video.h"
#include "vp_mp4.h"
#include "vp_thread_hic.h"
#include "vp_thread_capture.h"
#include "vp_thread_vcom.h"
#include "vp_jpeg.h"
#include "vp_info.h"
#include "hic_commands.h"
#ifdef ECOS
#include "vp_interfaces.h"
#include "vp_bitrate_control.h"
#endif

#define SWAP(a,b,TYPE) \
do { \
	TYPE __a_tmp_varible = (a); \
	(a) = (b); \
	(b) = __a_tmp_varible; \
} while (0)


extern const char *g_apcRevision[];

#ifndef C_INLINE
#define C_INLINE inline
#endif

/* Convert USHORT to UCHAR.*/
C_INLINE UCHAR *USHORT_2_UCHAR (UINT32 usIn, UCHAR *pucOut)
{
    pucOut[1] = (UCHAR) (usIn >> 8);
    pucOut[0] = (UCHAR) usIn;
    return pucOut;
}

/* Convert UCHAR to USHORT. */
C_INLINE USHORT UCHAR_2_USHORT (CONST UCHAR *pcucIn)
{
	return (USHORT)(((USHORT) pcucIn[0])
			| (((USHORT) pcucIn[1]) << 8));
}


/* Convert UINT32 to UCHAR. */
C_INLINE UCHAR *UINT32_2_UCHAR (UINT32 uIn, UCHAR *pucOut)
{
	pucOut[3] = (UCHAR) (uIn >> 24);
	pucOut[2] = (UCHAR) (uIn >> 16);
	pucOut[1] = (UCHAR) (uIn >> 8);
	pucOut[0] = (UCHAR) uIn;
	return pucOut;
}


/* Convert UCHAR to UINT32. */
C_INLINE UINT32 UCHAR_2_UINT32 (CONST UCHAR *pcucIn)
{
	return ((UINT32) pcucIn[0])
			| (((UINT32) pcucIn[1]) << 8)
			| (((UINT32) pcucIn[2]) << 16)
			| (((UINT32) pcucIn[3]) << 24);
}


#endif