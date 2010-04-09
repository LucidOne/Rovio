;/******************************************************************************
; *
; * Copyright (c) 2003 Windond Electronics Corp.
; * All rights reserved.
; *
; * $Workfile: bin.s $
; *
; * $Author: yyang $
; ******************************************************************************/
;/*
; * $History: bin.s $
; 
; *****************  Version 3  *****************
; User: Wschang0     Date: 03/08/27   Time: 11:32a
; Updated in $/W90N740/FIRMWARE/updater
; Modify the source path to VSSWORK
; 
; *****************  Version 2  *****************
; User: Wschang0     Date: 03/08/20   Time: 1:33p
; Updated in $/W90N740/FIRMWARE/updater
; Add VSS header
; */

	EXPORT	stream_buffer_start
	EXPORT	stream_buffer_end

	AREA bin_section, CODE, READONLY
	KEEP
	ALIGN 4
stream_buffer_start
	INCBIN ../../../pattern/dec/input/T10_122.COD
	;INCBIN ../../../pattern/dec/input/T10_475.COD
stream_buffer_end	
	END