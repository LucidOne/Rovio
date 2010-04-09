#ifndef C_MULTIPART_H
#define C_MULTIPART_H

typedef struct
{
	LIST *plPartDiscription;	//LIST of MULTIPART_DESCRIPTION_T;
	char *pcPartBody;
	int	iPartBodyLength;
} INNERPART_T;

typedef struct
{
	char *pcSegStart;//闭区间
	char *pcSegEnd;	//开区间
} SPLIT_ITEM_T;

/* 分割pcBuffer, (iBufferLength - pBuffer长度)
   iMaxReturn, 最大的分割数目

	分割判定函数
	char *pcSeperator(char *pcBeing, char *pcEnd, char *pcCur, void *pSeperatorParam);
		pcBegin - pBuffer
		pcEnd - pBuffer + iBufferLength
		pcCur - 位于[pcBegin, pcEnd)之间的某个位置
		如果pcCur处符合分割判定条件, 返回下一段的位置
		否则, 返回NULL
*/

LIST *httpSplitBuffer(char *pcBuffer, int iBufferLength, int iMaxReturn, char *pcSeperator(char *pcBeing, char *pcEnd, char *pcCur, void *pSeperatorParam), void *pSeperatorParam);
void httpDeleteSplitBuffer(LIST *pList);

LIST *httpParseDescriptionLine(char *pcDescription, int iDescriptionLength);
void httpDeleteDescriptionLine(LIST *pList);

LIST *httpParseDescription(char *pcDescription, int iDescriptionLength);
void httpDeleteDescription(LIST *pList);

INNERPART_T *httpParseSinglePart(char *pcSinglePartSource, int iSinglePartSourceLength);
void httpDeleteSinglePart(INNERPART_T *pInnerPart);

LIST *httpParseMultiPart(char *pcMultiPartSource, int iMultiPartSourceLength, char *pcBoundary);
void httpDeleteMultiPart(LIST *pList);

#endif
