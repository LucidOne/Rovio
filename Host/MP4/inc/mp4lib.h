#ifndef _MP4LIB_H_
#define _MP4LIB_H_

#define	MP4_NO_ERR				0 					// No Error
#define	MP4_ERR_TRIGGER_DECODER	(MP4_ERR_ID | 0x01)	// Double trigger MP4 Decoder befoer decode complete
#define	MP4_ERR_IMG_SIZE		(MP4_ERR_ID | 0x02)	// Image size error in VOL bit-stream
#define	MP4_ERR_VOL_NOT_FOUND	(MP4_ERR_ID | 0x03)	// VOL Not found in 4K buffer
#define	MP4_ERR_TRIGGER_ENCODER	(MP4_ERR_ID | 0x04)	// Double trigger MP4 Encoder befoer Encode complete
#define	MP4_ERR_H263_SIZE	    (MP4_ERR_ID | 0x05)	// H.263 source format not found

typedef struct MP4DECINFO {
	UINT8   verid;
	UINT8   short_header;                           // 0 : H.263, 1 : MPEG4 bitstream
	UINT8   vol_shape;           
	UINT16  vti_res;                                // vop_time_increment_resolution
	UINT16  vti_size;                               // bit number for VTI
	UINT32  uPixels;                                // Image width
	UINT32  uLines;                                 // Image height
	UINT8   interlaced;
	UINT8   obmc_disable;
	UINT8   not_8_bit;
	UINT8   quant_type;                          
	UINT8   load_intra_quant_mat;
	UINT8   load_nonintra_quant_mat;
	UINT8   data_partitioned;	
	UINT8   reversible_vlc;
	UINT8   scalability;	
	UINT8   quant_mat[128];
	UINT32  uBitstreamaddr;	                        // Bit-stream buffer addr
	UINT32  uDecodedbufsize;		                // bit-stream buffer size                
} *pMP4DECINFO_T,MP4DECINFO_T;

typedef struct MP4ENCINFO {
    	/* Codec control */
	UINT8   ucBaserate;                             // fps
   	UINT8   qintra;                                 // Initial Q for Intra
   	UINT8   qinter;                                 // Initial Q for Inter
   	UINT32  bitrate;                                // Target bit rate, 0:fixed Q, others fixed bit rate control
   	UINT8   allow_framedrop;                        // Allow bit rate control to skip frame
   	    /* Buffer allocate */
	UINT32  uReferenceaddr;                         // Reference buffer addr
	UINT32  uReconstructaddr;                       // Reconstruct buffer addr
	UINT32  uBitstreamheaderaddr;	                // header bit-stream buffer addr
	UINT32  uEncodedbufsize;	                        // reserve bit-stream buffer size
		/* Bit stream information */
	UINT8   codectype;	                            // 0:H.263, 1:MPEG4 bit-stream
	UINT16  vop_time_increment_resolution;			// vop_time_increment_resolution;
	INT32   PframenumbeteweenIfram;			        // P frame number between I frame. 0 Means all are I frame
	UINT32  video_packet_interval;		            // Video Packet Interval, 0 :disable, others : length of byte to insert resync_marker
	UINT32  uPixels;                                // image width
	UINT32  uLines;                                 // image height
	UINT32  quant_type;                             // 0 : disable quant matrix, , others : specify user defined quant matrix
	UINT8   quant_mat[128];                         // user defined quant matrix 
} *pMP4ENCINFO_T,MP4ENCINFO_T;


#define		RETURN_ERROR	            1
#define		RETURN_SUCCESS 	            0
#define	    H263_SHORT_HEADER_BITSTREAM	0
#define     MPEG4_BITSTREAM			    1

typedef enum mp4_interrupt_source_e
{
    MP4_ENCODE_COMPLETE_CALL_BACK=0,
    MP4_ENCODE_ERROR_CALL_BACK,
    MP4_DECODE_COMPLETE_CALL_BACK,
    MP4_DECODE_WAIT_CALL_BACK,
    MP4_DECODE_ERROR_CALL_BACK
} MP4_INTERRUPT_SRC_E;


//----------------------MPEG 4 CallBack export Function----------------------------------------
void    mp4InstallISR(void);
PVOID   mp4InstallCallBackFunction(INT32 uSource, PVOID pvNewISR);
//----------------------MPEG 4 Encoder export Function----------------------------------------
UINT32 	mp4SetEncoder(MP4ENCINFO_T* pEncinfo);
INT	mp4SetEncoderBitrate(UINT32 bitrate, UINT32 lines);
INT	mp4SetEncoderFramerate(UINT32 framerate, UINT32 lines);
UINT32 	mp4GetEncBitStreamLength(void);
UINT32 	mp4GetCurrentBitStreamAddr(void);
void 	mp4SetNewEncBitStreamAddr(UINT32 newaddr);
UINT8   mp4SetNewYUVEncoderAddr(UINT32 uYuvaddr);
INT 	mp4StartEncoder(void);
void 	mp4ResetEncoder(void);
UINT8   mp4SetEncQuantmatrix(PUINT8 pucMatrix);
//----------------------MPEG 4 Decoder export Function----------------------------------------
INT     mp4SetDecoder(MP4DECINFO_T *pDecinfo);
INT     mp4StartDecoder(void);
UINT32  mp4GetDecBitstreamLength(void);
UINT32  mp4GetCurrentDisplaybufAddr(void);
UINT8	mp4SetNewDecBitStreamAddr(UINT32 uStreamaddr);
UINT32  mp4GetCurrentVTI(void);
void 	mp4ResetDecoder(void);
UINT8   mp4SetDecBufferAddr(UINT32 uReferenceaddr,UINT32 uOutputaddr);
void    mp4ResumeDecoder(void);
#endif  /* _MP4LIB_H_ */


