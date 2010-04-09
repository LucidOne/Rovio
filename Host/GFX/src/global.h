#include "gfxlib.h"

/*--- compiler options ---*/

#define DBG_LEVEL   0
#define gfx_debug   printf

/*--- constant or macro definitions used by library ---*/

#define ABS(x)      (((x)>0)?(x):-(x))
#define MAX(a,b)	(((a)>(b))?(a):(b))

/* octant code of line drawing */

#define	XpYpXl		(0<<1)
#define	XpYpYl		(1<<1)
#define XpYmXl  	(2<<1)
#define XpYmYl  	(3<<1)
#define XmYpXl  	(4<<1)
#define XmYpYl  	(5<<1)
#define XmYmXl  	(6<<1)
#define XmYmYl  	(7<<1)

/*--- global variables ---*/

extern BOOL     _gfx_bInitialized;
extern BOOL     _gfx_bClipEnabled;
extern BOOL     _gfx_bColorKeyEnabled;
extern BOOL     _gfx_bAlphaEnabled;
extern BOOL     _gfx_bPenInitialized;
extern BOOL     _gfx_bPatternInitialized;

extern INT      _gfx_nDestWidth;
extern INT      _gfx_nDestHeight;
extern INT      _gfx_nDestPitch;
extern INT      _gfx_nSrcWidth;
extern INT      _gfx_nSrcHeight;
extern INT      _gfx_nSrcPitch;

extern INT      _gfx_nColorFormat;
extern INT      _gfx_nBpp;
extern INT      _gfx_nByte;
extern INT      _gfx_nScreenSize;

extern UINT32   _gfx_uDestStartAddr;
extern UINT32   _gfx_uColorPatternStartAddr;
extern UINT32   _gfx_uSrcStartAddr;

extern GFX_RECT_T   _gfx_ClipRect;
extern UINT32       _gfx_uClipTL;
extern UINT32       _gfx_uClipBR;

extern GFX_DRAW_MODE_E      _gfx_nDrawMode;
extern GFX_PATTERN_TYPE_E   _gfx_nPatternType;

extern UINT8    _gfx_ucROP; 
extern UINT16   _gfx_usPenStyle;
extern UINT32   _gfx_uPenForeColor;
extern UINT32   _gfx_uPenBackColor;
extern UINT32   _gfx_uPatternForeColor;
extern UINT32   _gfx_uPatternBackColor;
extern UINT32   _gfx_uColorKey;
extern UINT32   _gfx_uColorKeyMask;
extern UINT32   _gfx_uWriteMask;
extern UINT32   _gfx_uAlphaKs;
extern UINT32   _gfx_uTileX;
extern UINT32   _gfx_uTileY;

/*--- internal functions ---*/

extern VOID     gfxWaitEngineReady(VOID);
extern BOOL     gfxIsValidRect(GFX_RECT_T rect);
extern BOOL     gfxIsPInROP(UINT8 rop);


