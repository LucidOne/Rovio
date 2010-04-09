#ifndef __DATA_ENG_11X11_H__
#define __DATA_ENG_11X11_H__


#define FONT11X11_MAP_ENG_SIZE 1792
extern EF_UCHAR g_ucFont11x11MapEng[FONT11X11_MAP_ENG_SIZE];
EF_INT font11x11engCharWidth1(const FONT_T *pFont, const EF_UCHAR *pucBitMap);
EF_INT font11x11engCharHeight1(const FONT_T *pFont, const EF_UCHAR *pucBitMap);
const EF_UCHAR *font11x11engGetMapAddr1(const FONT_T *pFont, EF_UCHAR ucChar);



extern FONT_T g_tFont11x11Eng;

#endif
