#ifndef __VP_INFO_H__
#define __VP_INFO_H__


void vinfoDrawTitle (void *pYUVBuffer, VP_SIZE_T size);
void vinfoSetTime (const struct tm *pTms);
void vinfoGetTime (struct tm *pTms);

void vinfoDrawContent(void *pYUVBuffer, VP_SIZE_T size);
void vinfoSetContent(int index, char *pcContent, int x, int y);
void vinfoClearContent(int index);
void vinfoInit(void);


#endif

