/******************************************************************************
 *
 *  Copyright (c) 2004-2005 by Winbond Electronics Corp. All rights reserved.
 *  
 *  Winbond W9972 Graphics Engine Low-Level Library       
 * 
 *  FILENAME
 *      GFXLIB.H    
 *          
 *  VERSION
 *      1.4
 *                                                                                                                    
 *  DESCRIPTION
 *      This is the include file of W99702 graphics engine low-level library.     
 *
 *  HISTORY
 *      2004/09/13  Created by PC50 Walter Tseng
 *                  (follows coding rule version 1.3)
 *      2004/09/20  Added support of font library.
 *      2004/09/27  Added gfxMemcpy() and gfxImageFilter()
 *      2004/10/14  Added gfxSlideShow()
 *      2004/11/06  Changed GFX_DRAW_MODE_E for GFX_DEST_TRANSPARENT 
 *      2004/12/22  Added slide show transition definition
 *      2005/03/25  Export gfxWaitEngineReady() for application
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#ifndef _GFXLIB_H_
#define _GFXLIB_H_
                
#ifndef _WBTYPES_H_
#include "wbtypes.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Definition of data types
//
///////////////////////////////////////////////////////////////////////////////
typedef UINT32      GFX_COLOR;  // 0x00RRGGBB logical color
typedef UINT32      GFX_ENTRY;  // 0x00xxxxxx color palette entry number
                                // (physical color)
typedef UINT32      GFX_MASK;                                    
                                    

///////////////////////////////////////////////////////////////////////////////
//
// Definition of constants
//
///////////////////////////////////////////////////////////////////////////////

/* graphics BPP */

typedef enum gfx_color_format_e
{
    GFX_BPP_332,
    GFX_BPP_444H,
    GFX_BPP_444L,
    GFX_BPP_565,
    GFX_BPP_666,
    GFX_BPP_888
}   GFX_COLOR_FORMAT_E; 

/* pixel drawing mode */

typedef enum gfx_draw_rop_e
{
    GFX_DRAW_ROP_COPY,
    GFX_DRAW_ROP_XOR,
    GFX_DRAW_ROP_OR, 
    GFX_DRAW_ROP_AND
}   GFX_DRAW_ROP_E;

/* drawing mode */

typedef enum gfx_draw_mode_e
{
    GFX_OPAQUE,
    GFX_SRC_TRANSPARENT,
    GFX_DEST_TRANSPARENT
}   GFX_DRAW_MODE_E;

/* 8x8 pattern type */

typedef enum gfx_pattern_type_e
{
    GFX_MONO_PATTERN,
    GFX_COLOR_PATTERN    
}   GFX_PATTERN_TYPE_E;    

/* font language */

typedef enum gfx_lang_e
{
    GFX_LANG_ARABIC,
    GFX_LANG_CHINA,
    GFX_LANG_TAIWAN,
    GFX_LANG_ENGLISH,
    GFX_LANG_FRENCH,
    GFX_LANG_GERMAN,
    GFX_LANG_ITALIAN,
    GFX_LANG_JAPAN,
    GFX_LANG_KOREAN,
    GFX_LANG_SPANISH,
    GFX_LANG_THAI
}   GFX_LANG_E;

/* slide transition type */

typedef enum gfx_slide_e
{
    GFX_SLIDE_NO_TRANSITION,        
    GFX_SLIDE_COVER_DOWN,           
    GFX_SLIDE_COVER_LEFT,           
    GFX_SLIDE_COVER_RIGHT,          
    GFX_SLIDE_COVER_UP,             
    GFX_SLIDE_COVER_LEFT_DOWN,      
    GFX_SLIDE_COVER_LEFT_UP,        
    GFX_SLIDE_COVER_RIGHT_DOWN,     
    GFX_SLIDE_COVER_RIGHT_UP,       
    GFX_SLIDE_UNCOVER_DOWN,         
    GFX_SLIDE_UNCOVER_LEFT,         
    GFX_SLIDE_UNCOVER_RIGHT,        
    GFX_SLIDE_UNCOVER_UP,           
    GFX_SLIDE_UNCOVER_LEFT_DOWN,    
    GFX_SLIDE_UNCOVER_LEFT_UP,      
    GFX_SLIDE_UNCOVER_RIGHT_DOWN,   
    GFX_SLIDE_UNCOVER_RIGHT_UP,     
    GFX_SLIDE_BLINDS_VERTICAL,      
    GFX_SLIDE_BLINDS_HORIZONTAL,    
    GFX_SLIDE_SPLIT_HORIZONTAL_IN,  
    GFX_SLIDE_SPLIT_HORIZONTAL_OUT, 
    GFX_SLIDE_SPLIT_VERTICAL_IN,    
    GFX_SLIDE_SPLIT_VERTICAL_OUT,   
    GFX_SLIDE_CHECKERBOARD_ACROSS,  
    GFX_SLIDE_CHECKERBOARD_DOWN,    
    GFX_SLIDE_BOX_IN,               
    GFX_SLIDE_BOX_OUT,              
    GFX_SLIDE_FADE,                 
    GFX_SLIDE_FADE_FROM_BLACK,      
    GFX_SLIDE_WIPE_DOWN,            
    GFX_SLIDE_WIPE_LEFT,            
    GFX_SLIDE_WIPE_RIGHT,           
    GFX_SLIDE_WIPE_UP              
}   GFX_SLIDE_E;

/* some ROP definition (frequently used) */

#define BLACKNESS       0x00  // 0
#define DSTINVERT       0x55  // Dn
#define MERGECOPY       0xC0  // PSa
#define MERGEPAINT      0xBB  // DSno
#define NOTSRCCOPY      0x33  // Sn
#define NOTSRCERASE     0x11  // DSon
#define PATCOPY         0xF0  // P
#define PATINVERT       0x5A  // DPx
#define PATPAINT        0xFB  // DPSnoo
#define SRCAND          0x88  // DSa
#define SRCCOPY         0xCC  // S
#define SRCERASE        0x44  // SDna
#define SRCINVERT       0x66  // DSx
#define SRCPAINT        0xEE  // DSo
#define WHITENESS       0xFF  // 1

/* definition of function SUCESS */

#define GFX_OK          0   

/* definition of returned error code */

#define ERR_GFX_ENV_NOT_INITIALIZED         0x01
#define ERR_GFX_ENV_OPENED                  0x02
#define ERR_GFX_INVALID_COLOR_FORMAT        0x03
#define ERR_GFX_INVALID_RECT                0x04
#define ERR_GFX_INVALID_THICK               0x05
#define ERR_GFX_PEN_NOT_INITIALIZED         0x06
#define ERR_GFX_PATTERN_NOT_INITIALIZED     0x07
#define ERR_GFX_COLOR_KEY_NOT_INITIALIZED   0x08
#define ERR_GFX_INVALID_PATTERN_TYPE        0x09
#define ERR_GFX_INVALID_ALPHA               0x0a
#define ERR_GFX_FONT_NOT_INITIALIZED        0x10
#define ERR_GFX_INVALID_FONT                0x11
#define ERR_GFX_INVALID_SLIDE_OPT           0x12


///////////////////////////////////////////////////////////////////////////////
//
// Definition of data structures
//
///////////////////////////////////////////////////////////////////////////////
typedef struct gfx_info_t
{ 
    INT                 nDestWidth;             // destination surface width
    INT                 nDestHeight;            // destination surface height
    INT                 nDestPitch;             // destination surface pitch
    INT                 nSrcWidth;              // source surface width
    INT                 nSrcHeight;             // source surface height
    INT                 nSrcPitch;              // source surface pitch in bytes
    GFX_COLOR_FORMAT_E  nColorFormat;           // surface color format
    UINT32              uDestStartAddr;         // destination start address
    UINT32              uColorPatternStartAddr; // color pattern start address 
    UINT32              uSrcStartAddr;          // source surface start address
}   GFX_INFO_T;

typedef struct gfx_surface_t
{
    INT     nWidth;
    INT     nHeight;
    INT     nPitch;
    UINT32  uStartAddr;
}   GFX_SURFACE_T;

typedef struct gfx_pnt_t
{
    INT     nX;
    INT     nY;
}   GFX_PNT_T;

typedef union gfx_rect_t
{
    struct
    { 
        INT     nLeft;
        INT     nTop;
        INT     nRight;
        INT     nBottom;
    }   fC;
    struct  
    {
        GFX_PNT_T   pntLeftTop;
        GFX_PNT_T   pntRightBottom;
    }   fP;
}   GFX_RECT_T;

typedef struct gfx_size_t
{
    INT     nWidth;
    INT     nHeight;
}   GFX_SIZE_T;

typedef struct gfx_filter_control_t
{
    UINT32  uFilter0;   // central pixel and divisor
    UINT32  uFilter1;   // around pixels
    UINT32  uMaxClamp;  // clamping maximum value
    UINT32  uOffset;    // offset value
    UINT32  uThreshold; // threshold value
}   GFX_FILTER_CONTROL_T;

typedef struct gfx_filter_block_t
{
    UINT32  uStartAddr;
    UINT32  uPitch;
}   GFX_FILTER_BLOCK_T;

typedef	struct gfx_font_info_t
{ 
    PSTR                szName;     // font name
    INT                 nLangId;    // language identifier
    INT                 nCodeLen;   // # of bytes to specify a character
    INT                 nWidth;     // character box width in pixels
    INT                 nHeight;    // character box height in pixels
    INT                 nPitch;     // the size of character row in bytes
    INT                 nTotalChar; // total # of characters in the font
}   GFX_FONT_INFO_T;

struct tag_font_list
{
    GFX_FONT_INFO_T *         gfxFontInfo;
    struct tag_font_list *    pNextFontInfo;
};

typedef struct tag_font_list GFX_FONT_LIST_T ;


///////////////////////////////////////////////////////////////////////////////
//
// Declaration of function protypes (API)
//
///////////////////////////////////////////////////////////////////////////////
extern INT          gfxOpenEnv(GFX_INFO_T *in_param);
extern INT          gfxCloseEnv(VOID);
extern VOID         gfxWaitEngineReady(VOID);

extern GFX_ENTRY    gfxMakeColor(GFX_COLOR color);

extern INT          gfxSetDestSurface(GFX_SURFACE_T *surface);
extern INT          gfxSetSrcSurface(GFX_SURFACE_T *surface);

extern INT          gfxClearScreen(GFX_ENTRY color);
extern INT          gfxDrawFrame(GFX_PNT_T p1, GFX_PNT_T p2, INT thick, GFX_ENTRY color);
extern INT          gfxDrawLine(GFX_PNT_T p1, GFX_PNT_T p2);
extern INT          gfxDrawSolidLine(GFX_PNT_T p1, GFX_PNT_T p2, GFX_ENTRY color);
extern INT          gfxDrawPixel(GFX_PNT_T pnt, GFX_ENTRY color, GFX_DRAW_ROP_E draw_rop); 
extern INT          gfxFillRect(GFX_RECT_T rect);
extern INT          gfxFillSolidRect(GFX_RECT_T rect, GFX_ENTRY color); 

extern INT          gfxColorExpansionBlt(GFX_RECT_T rect, GFX_ENTRY fore_color, GFX_ENTRY back_color, PVOID bit_buf);
extern INT          gfxGetImage(GFX_RECT_T src_rect, GFX_PNT_T dest_pnt);
extern INT          gfxPutImage(GFX_RECT_T dest_rect, GFX_PNT_T src_pnt);
extern INT          gfxScreenToScreenBlt(GFX_RECT_T src_rect, GFX_PNT_T dest_pnt);

extern INT          gfxSetAlpha(BOOL enable, INT alpha);
extern INT          gfxSetClip(BOOL enable, GFX_RECT_T rect);
extern INT          gfxSetColorKey(BOOL enable, GFX_ENTRY key_color, GFX_MASK key_mask);
extern INT          gfxSetDrawMode(GFX_DRAW_MODE_E draw_mode);
extern INT          gfxSetPattern(GFX_PATTERN_TYPE_E type, GFX_ENTRY fore_color, GFX_ENTRY back_color, PVOID pat_buf);
extern INT          gfxSetPen(UINT16 style, GFX_ENTRY fore_color, GFX_ENTRY back_color);
extern INT          gfxSetROP(UINT8 rop_code);
extern INT          gfxSetTile(INT tile_x, INT tile_y);
extern INT          gfxSetWriteMask(GFX_MASK color_mask);

extern INT          gfxMemcpy(PVOID dest, PVOID src, INT count);
extern INT          gfxImageFilter(GFX_FILTER_BLOCK_T *dest, GFX_FILTER_BLOCK_T *src, GFX_SIZE_T size, GFX_FILTER_CONTROL_T *ctl);
extern INT          gfxSetSlideShowTransition(GFX_SLIDE_E opt, GFX_SIZE_T size);
extern INT          gfxSlideShow(PVOID current_slide, PVOID previous_slide, PVOID output_slide, INT step_no);

/* font library functions */

extern GFX_FONT_LIST_T *    gfxGetFontList(VOID);
extern INT                  gfxGetTextWidth(PSTR str, INT len);
extern INT                  gfxGetTextHeight(PSTR str, INT len);
extern INT                  gfxSetFont(GFX_FONT_INFO_T *font_info);
extern INT                  gfxTextOutput(GFX_PNT_T p, PSTR str, INT len, GFX_ENTRY fore_color, GFX_ENTRY back_color);

/*
** Internal used functions
*/
extern VOID         gfxInitLCM(UINT32 addr, GFX_COLOR_FORMAT_E fmt, INT pitch);

/*--- font library function ---*/

extern INT          gfxRealizeText(PVOID strbmp, PSTR str, INT len);


#endif /* _GFXLIB_H_ */

