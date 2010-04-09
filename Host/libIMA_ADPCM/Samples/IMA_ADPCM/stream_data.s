;/******************************************************************************
; *
; * Copyright (c) 2003 Windond Electronics Corp.
; * All rights reserved.
; *
; * $Workfile: bin.s $
; *
; * $Author: ypxin $
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
stream_buffer_start
	;INCBIN	e:/pc30vss/source/mp3/pattern/l3bit/B_3_44_128.mp3
	;INCBIN	e:/pc30vss/source/mp3/pattern/l3bit/B_3_48_320.mp3
	;INCBIN	e:/pc30vss/source/mp3/pattern/l3bit/B_3_48_320_1S.mp3
	;INCBIN E:/PC30VSS/Source/mp3/pattern/songs/10sec_64k.mp3
	INCBIN E:/PC30VSS/Source/mp3/pattern/songs/castanets_441khz_128kbps.mp3
stream_buffer_end	
	END