#ifndef _MFL_STREAM_H_
#define _MFL_STREAM_H_

#define CHECK_STREAM			1		/* check stream valid or not */
#define STREAM_MEM_PER_ALLOC	4096

#define STREAM_WRITE_BUF_SIZE	4096

#define STREAM_INPUT			1
#define STREAM_OUTPUT			0

#define STRM_SEEK_SET			0
#define STRM_SEEK_CUR			1
#define STRM_SEEK_END           2


/* exported stream method */
extern STRM_FUN_T _mfl_tFileStrmFun;
extern STRM_FUN_T _mfl_tMemStrmFun;

extern INT  mfl_open_stream(STREAM_T *ptStream, STRM_FUN_T *ptStrmFun, CHAR *suFileName, CHAR *szAsciiName, UINT8 *pbMemBase, UINT32 ulMemLen, INT access);
extern INT  mfl_close_stream(STREAM_T *ptStream);
extern INT  mfl_is_stream_opened(STREAM_T *ptStream);
extern INT  mfl_seek_stream(STREAM_T *ptStream, INT nOffset, INT nSeek);
extern INT  mfl_stream_pos(STREAM_T *ptStream);
extern INT  mfl_read_stream_raw(STREAM_T *ptStream, UINT8 *pucBuff, INT nCount, BOOL bIsSkip);
extern INT  mfl_peek_stream_raw(STREAM_T *ptStream, UINT8 *pucBuff, INT nCount);
extern INT  mfl_write_stream_raw(STREAM_T *ptStream, UINT8 *pucBuff, INT nCount);

extern INT  mfl_copy_stream(STREAM_T *tInStrm, STREAM_T *tOutStrm, INT nByteCnt);
extern INT  mfl_read_stream_force_byte_align(STREAM_T *ptStream);
extern INT  mfl_read_stream_bits(STREAM_T *ptStream, UINT8 *pucBuff, INT bitCnt, INT *pnReadCnt);
extern INT  mfl_read_stream_byte(STREAM_T *ptStream, UINT8 *val8);
extern INT  mfl_read_stream16(STREAM_T *ptStream, UINT16 *val16);
extern INT  mfl_read_stream24(STREAM_T *ptStream, UINT32 *val24);
extern INT  mfl_read_stream32(STREAM_T *ptStream, UINT32 *val32);
extern INT  mfl_write_stream_force_byte_align(STREAM_T *ptStream);
extern INT  mfl_write_stream_bits(STREAM_T *ptStream, UINT8 *pucBuff, INT nBitCnt);
extern INT  mfl_write_stream_byte(STREAM_T *ptStream, UINT8 val8);
extern INT  mfl_write_stream16(STREAM_T *ptStream, UINT16 val16);
extern INT  mfl_write_stream24(STREAM_T *ptStream, UINT32 val24);
extern INT  mfl_write_stream32(STREAM_T *ptStream, UINT32 val32);

#endif	/* _MFL_STREAM_H_ */