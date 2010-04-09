#include "global.h"

BOOL    _gfx_bInitialized           = FALSE;
BOOL    _gfx_bClipEnabled           = FALSE;
BOOL    _gfx_bColorKeyEnabled       = FALSE;
BOOL    _gfx_bAlphaEnabled          = FALSE;
BOOL    _gfx_bPenInitialized        = FALSE;
BOOL    _gfx_bPatternInitialized    = FALSE;

INT     _gfx_nDestWidth             = 0;
INT     _gfx_nDestHeight            = 0;
INT     _gfx_nDestPitch             = 0;
INT     _gfx_nSrcWidth              = 0;
INT     _gfx_nSrcHeight             = 0;
INT     _gfx_nSrcPitch              = 0;

INT     _gfx_nColorFormat           = 0;
INT     _gfx_nBpp                   = 0;
INT     _gfx_nByte                  = 0;
INT     _gfx_nScreenSize            = 0;

UINT32  _gfx_uDestStartAddr         = 0;
UINT32  _gfx_uColorPatternStartAddr = 0;
UINT32  _gfx_uSrcStartAddr          = 0;

GFX_RECT_T  _gfx_ClipRect;
UINT32      _gfx_uClipTL            = 0;
UINT32      _gfx_uClipBR            = 0;


GFX_DRAW_MODE_E     _gfx_nDrawMode  = GFX_OPAQUE;
GFX_PATTERN_TYPE_E  _gfx_nPatternType;

UINT8   _gfx_ucROP                  = SRCCOPY;  // default ROP
UINT16  _gfx_usPenStyle;
UINT32  _gfx_uPenForeColor;
UINT32  _gfx_uPenBackColor;
UINT32  _gfx_uPatternForeColor;
UINT32  _gfx_uPatternBackColor;
UINT32  _gfx_uColorKey;
UINT32  _gfx_uColorKeyMask;
UINT32  _gfx_uWriteMask;
UINT32  _gfx_uAlphaKs;
UINT32  _gfx_uTileX                 = 1;
UINT32  _gfx_uTileY                 = 1;


