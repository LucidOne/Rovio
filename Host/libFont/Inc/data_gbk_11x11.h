#ifndef __DATA_GBK_11X11_H
#define __DATA_GBK_11X11_H


#define FONT11X11_INDEX_GBK_SIZE 980
extern FONT_INDEX_T g_tFont11x11IndexGBK[FONT11X11_INDEX_GBK_SIZE / sizeof (FONT_INDEX_T)];
#define FONT11X11_MAP_GBK_SIZE 352736
extern EF_UCHAR g_ucFont11x11MapGBK[FONT11X11_MAP_GBK_SIZE / sizeof (EF_UCHAR)];
const EF_UCHAR *font11x11gbkGetMapAddr2(const FONT_T *pFont, EF_UCHAR ucChar1, EF_UCHAR ucChar2, EF_UCHAR aucConvert[4], EF_INT *pnLenConvert, EF_INT *pnLenUsed);



extern FONT_T g_tFont11x11GBK;
#endif
