/***************************************************************************
 *                                                                         *
 * Copyright (c) 2000 - 2005 Winbond Electronics Corp. All rights reserved.*
 *                                                                         *
 ***************************************************************************/
 
/****************************************************************************
 * 
 * FILENAME
 *		$RCSfile: CmdSet.h,v $
 *
 * VERSION
 *		$Revision: 1.1 $
 *
 * DESCRIPTION
 *		W99702 camera module protocol commands.
 *
 * HISTORY
 *     $Log: CmdSet.h,v $
 *     Revision 1.1  2006/01/17 09:42:11  xhchen
 *     Add B.B. testing applications.
 *
 *     Revision 1.1.2.8  2002/01/31 15:06:39  xhchen
 *     Add media parsing functions to High Level API.
 *
 *     Revision 1.1.2.7  2005/09/07 03:31:58  xhchen
 *     1. Update command set API to WBCM_702_0.82pre14.doc.
 *     2. Add the function to playback audio by sending audio bitstream.
 *     3. Add "wbhicRescue()" to clear BUSY, DRQ an ERROR.
 *     4. Merget WYSun's testing code to "Samples/FlowTest".
 *
 *     Revision 1.1.2.6  2005/08/30 04:14:38  xhchen
 *     Makefile and create_project.js support searching dependence header file automatically.
 *     Add ywsun's filebrowser demo (for virtual com).
 *     Add audio test in "FlowTest".
 *
 *     Revision 1.1.2.5  2005/08/03 06:01:01  xhchen
 *     Add a simple virtual com demo.
 *
 *     Revision 1.1.2.4  2005/07/21 07:52:42  xhchen
 *     ...
 *
 *     Revision 1.1.2.3  2005/01/07 08:41:34  xhchen
 *     Winbond Camera Module High Level API first version released.
 *
 *
 * REMARK
 *     None
 *
 **************************************************************************/


#include "./InInc/Commands_test.h"

