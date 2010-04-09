/******************************************************************************
 *
 *  Copyright (c) 2004 by Winbond Electronics Corp. All rights reserved.       
 *  Winbond W99702 Graphics Low-Level Library
 *
 *  FILENAME
 *      SLIDE.C   
 *          
 *  VERSION
 *      1.0
 *                                                                                                                    
 *  DESCRIPTION
 *      This is module is used to do slide show.
 * 
 *  HISTORY
 *      2004/10/15  Created by PC50 Walter Tseng
 *
 *  REMARK
 *      None
 *                                                             
 ******************************************************************************/
#ifdef ECOS
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "gfxlib.h"
#include "global.h"

#include "wbio.h"
#include "w99702_reg.h"


static GFX_SLIDE_E  _gfx_slide_opt = GFX_SLIDE_NO_TRANSITION;
static INT _gfx_slide_width, _gfx_slide_height;


///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSetSlideShowTransition()
//
//  DESCRIPTION
//      This function is used to set slide transition option for 
//      in current buffer of gfxSlideShow() function.
//
//  INPUTS
//      opt:        transition option 
//      size:       slide size
//
//  OUTPUTS
//      None
//
//  RETURN
//      >0:     sliding steps (>=1) of the specified transition option 
//      others: errors
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSetSlideShowTransition(GFX_SLIDE_E opt, GFX_SIZE_T size)
{
    INT steps, image_width, image_height;
    
    
    image_width = size.nWidth;
    image_height = size.nHeight;
    
    _gfx_slide_width = image_width;
    _gfx_slide_height = image_height;
    
    _gfx_slide_opt = opt;
    
    switch (opt)
    {
        case GFX_SLIDE_NO_TRANSITION:
            steps = 1;
            break;
            
        case GFX_SLIDE_COVER_DOWN:
            steps = image_height / 8;
            break;
            
        case GFX_SLIDE_COVER_LEFT:
            steps = image_width / 8;
            break;    

        case GFX_SLIDE_COVER_RIGHT:
            steps = image_width / 8;
            break;    
                        
        case GFX_SLIDE_COVER_UP:
            steps = image_height / 8; 
            break;        
        
        case GFX_SLIDE_COVER_LEFT_DOWN:
            steps = 8;
            break;
            
        case GFX_SLIDE_COVER_LEFT_UP:
            steps = 8;
            break;
            
        case GFX_SLIDE_COVER_RIGHT_DOWN:
            steps = 8;
            break;
            
        case GFX_SLIDE_COVER_RIGHT_UP:
            steps = 8;
            break;

        case GFX_SLIDE_UNCOVER_DOWN:
            steps = image_height / 8;
            break;
            
        case GFX_SLIDE_UNCOVER_LEFT:
            steps = image_width / 8;
            break;    

        case GFX_SLIDE_UNCOVER_RIGHT:
            steps = image_width / 8;
            break;    
                        
        case GFX_SLIDE_UNCOVER_UP:
            steps = image_height / 8; 
            break;        
        
        case GFX_SLIDE_UNCOVER_LEFT_DOWN:
            steps = 8;
            break;
            
        case GFX_SLIDE_UNCOVER_LEFT_UP:
            steps = 8;
            break;
            
        case GFX_SLIDE_UNCOVER_RIGHT_DOWN:
            steps = 8;
            break;
            
        case GFX_SLIDE_UNCOVER_RIGHT_UP:
            steps = 8;
            break;
            
        case GFX_SLIDE_BLINDS_VERTICAL:
            steps = 8;
            break;
            
        case GFX_SLIDE_BLINDS_HORIZONTAL:
            steps = 8;
            break;
            
        case GFX_SLIDE_SPLIT_HORIZONTAL_IN:
            steps = image_height / 8; 
            break;
            
        case GFX_SLIDE_SPLIT_HORIZONTAL_OUT:
            steps = image_height / 8; 
            break;        

        case GFX_SLIDE_SPLIT_VERTICAL_IN:
            steps = image_width / 8; 
            break;
            
        case GFX_SLIDE_SPLIT_VERTICAL_OUT:
            steps = image_width / 8; 
            break;  
        
        case GFX_SLIDE_CHECKERBOARD_ACROSS:
            steps = 8;
            break;
            
        case GFX_SLIDE_CHECKERBOARD_DOWN:
            steps = 8;
            break;
                    
        case GFX_SLIDE_BOX_IN: 
            steps = 8;
            break;

        case GFX_SLIDE_BOX_OUT: 
            steps = 8;
            break;
                        
        case GFX_SLIDE_FADE:
            steps = 16;
            break;
            
        case GFX_SLIDE_FADE_FROM_BLACK:
            steps = 16;
            break;  

        case GFX_SLIDE_WIPE_DOWN:
            steps = image_height / 8;
            break;

        case GFX_SLIDE_WIPE_LEFT:
            steps = image_width / 8;
            break;

        case GFX_SLIDE_WIPE_RIGHT:
            steps = image_width / 8;
            break;

        case GFX_SLIDE_WIPE_UP:
            steps = image_height / 8;
            break;
                                                
        default:
            _gfx_slide_opt = GFX_SLIDE_NO_TRANSITION;
            steps = 1; // set to GFX_SLIDE_NO_TRANSITION if not supported transition                  
            
            return ERR_GFX_INVALID_SLIDE_OPT;
    }
    
    return steps;
}
 
    
///////////////////////////////////////////////////////////////////////////////
//
//  FUNCTION
//      gfxSlideShow()
//
//  DESCRIPTION
//      This function is used to do image slide show.
//
//  INPUTS
//      current_slide:  pointer to current frame buffer 
//      previous_slide: pointer to previous frame buffer
//      output_slide:   pointer to output frame buffer
//
//  OUTPUTS
//      None
//
//  RETURN
//      0:      slide show transistion OK 
//      others: errors  
// 
///////////////////////////////////////////////////////////////////////////////
INT gfxSlideShow(PVOID current_slide, PVOID previous_slide, PVOID output_slide, INT step_no)
{
    INT image_width, image_height;
    PVOID buf565_current, buf565_previous, buf565_output;
    UINT16 *ptr16_current, *ptr16_previous, *ptr16_output;
    VOID *ptr16_start;
    VOID *ptr16_startA, *ptr16_startB;
    int image_size, stride_size, trans_size, inc_offset, trans_width, trans_height, offset, sx, sy;
    INT R_1, G_1, B_1, R_2, G_2, B_2, R, G, B;
    INT idx, i, j, i1, i2;
    
    if (! _gfx_bInitialized) return ERR_GFX_ENV_NOT_INITIALIZED;

    image_width = _gfx_slide_width;
    image_height = _gfx_slide_height;
        
    buf565_current = current_slide;
    buf565_previous = previous_slide;
    buf565_output = output_slide;
    
    ptr16_current = (UINT16 *)current_slide;
    ptr16_previous = (UINT16 *)previous_slide;
    ptr16_output = (UINT16 *)output_slide;
     
    switch (_gfx_slide_opt)
    {
        case GFX_SLIDE_NO_TRANSITION:
            memcpy(buf565_output, buf565_current, image_width*image_height*2);
            break;
            
        case GFX_SLIDE_COVER_DOWN:
            step_no++;
            image_size = image_width * image_height * 2;
            trans_size = image_width * 2 * step_no * 8;
            ptr16_start = (void *)((unsigned long)buf565_current+(image_size-trans_size));
            memcpy(buf565_output, ptr16_start, trans_size);
            break;
            
        case GFX_SLIDE_COVER_LEFT:
            step_no++;
            stride_size = image_width * 2;
            trans_size = step_no * 8 * 2; // line transfer size
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size-trans_size));
            ptr16_startB = buf565_current;
            for (idx=0; idx<image_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_size);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }
            break;
            
        case GFX_SLIDE_COVER_RIGHT:
            step_no++;
            stride_size = image_width * 2;
            trans_size = step_no * 8 * 2; // line transfer size
            ptr16_startA = buf565_output;
            ptr16_startB = buf565_current;
            for (idx=0; idx<image_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_size);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }        
            break;        
            
        case GFX_SLIDE_COVER_UP:
            step_no++;
            image_size = image_width * image_height * 2;
            trans_size = image_width * 2 * step_no * 8;
            ptr16_start = (void *)((unsigned long)buf565_output+(image_size-trans_size));
            memcpy(ptr16_start, ptr16_current, trans_size);
            break;     
               
        case GFX_SLIDE_COVER_LEFT_DOWN:
            step_no++;
            stride_size = image_width * 2;
            trans_width = (image_width / 8) * step_no * 2;
            trans_height = (image_height / 8) * step_no;
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size-trans_width));
            ptr16_startB = (void *)((unsigned long)buf565_current+(stride_size*(image_height-trans_height)));
            for (idx=0; idx<trans_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_width);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }
            break;
            
        case GFX_SLIDE_COVER_LEFT_UP: 
            step_no++;
            stride_size = image_width * 2;
            trans_width = (image_width / 8) * step_no * 2;
            trans_height = (image_height / 8) * step_no;
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size*(image_height-trans_height))+(stride_size-trans_width));
            ptr16_startB = buf565_current;
            for (idx=0; idx<trans_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_width);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }        
            break;
            
        case GFX_SLIDE_COVER_RIGHT_DOWN:
            step_no++;
            stride_size = image_width * 2;
            trans_width = (image_width / 8) * step_no * 2;
            trans_height = (image_height / 8) * step_no;
            ptr16_startA = buf565_output;
            ptr16_startB = (void *)((unsigned long)buf565_current+(stride_size*(image_height-trans_height))+(stride_size-trans_width));
            for (idx=0; idx<trans_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_width);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }
            break;
            
        case GFX_SLIDE_COVER_RIGHT_UP:
            step_no++;
            stride_size = image_width * 2;
            trans_width = (image_width / 8) * step_no * 2;
            trans_height = (image_height / 8) * step_no;
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size*(image_height-trans_height)));
            ptr16_startB = (void *)((unsigned long)buf565_current+(stride_size-trans_width));
            for (idx=0; idx<trans_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_width);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }        
            break;
        
        /*--- UNCOVER group ---*/
            
        case GFX_SLIDE_UNCOVER_DOWN:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_height/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = stride_size * step_no * 8;
                trans_size = image_size - offset;
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = ptr16_previous;
                memcpy(ptr16_startA, ptr16_startB, trans_size);
            }    
            break;
            
        case GFX_SLIDE_UNCOVER_LEFT:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_width/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = step_no * 8 * 2;
                trans_size = stride_size - offset;
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = buf565_output;
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<image_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }    
            }              
            break;
            
        case GFX_SLIDE_UNCOVER_RIGHT:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_width/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = step_no * 8 * 2;
                trans_size = stride_size - offset;
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)ptr16_previous;
                for (idx=0; idx<image_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }    
            }              
            break;        
            
        case GFX_SLIDE_UNCOVER_UP:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_height/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = stride_size * step_no * 8;
                trans_size = image_size - offset;
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = buf565_output;
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                memcpy(ptr16_startA, ptr16_startB, trans_size);
            }    
            break;     
               
        case GFX_SLIDE_UNCOVER_LEFT_DOWN:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                memcpy(buf565_output, buf565_current, image_size); 
                stride_size = image_width * 2;
                trans_width = (image_width / 8) * (8-step_no) * 2;
                trans_height = (image_height / 8) * (8-step_no);
                offset = (image_height - trans_height) * stride_size;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                offset = stride_size - trans_width;
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_width);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
                }
            }            
            break;
            
        case GFX_SLIDE_UNCOVER_LEFT_UP: 
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                memcpy(buf565_output, buf565_current, image_size); 
                stride_size = image_width * 2;
                trans_width = (image_width / 8) * (8-step_no) * 2;
                trans_height = (image_height / 8) * (8-step_no);
                ptr16_startA = buf565_output;
                offset = (image_height - trans_height) * stride_size + (stride_size - trans_width);
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_width);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
                }
            }          
            break;
            
        case GFX_SLIDE_UNCOVER_RIGHT_DOWN:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                memcpy(buf565_output, buf565_current, image_size); 
                stride_size = image_width * 2;
                trans_width = (image_width / 8) * (8-step_no) * 2;
                trans_height = (image_height / 8) * (8-step_no);
                offset = (image_height - trans_height) * stride_size + (stride_size - trans_width);
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = ptr16_previous;
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_width);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
                }
            }     
            break;
            
        case GFX_SLIDE_UNCOVER_RIGHT_UP:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                memcpy(buf565_output, buf565_current, image_size); 
                stride_size = image_width * 2;
                trans_width = (image_width / 8) * (8-step_no) * 2;
                trans_height = (image_height / 8) * (8-step_no);
                offset = stride_size - trans_width;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                offset = (image_height - trans_height) * stride_size;
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_width);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
                }
            }            
            break;
                        
        case GFX_SLIDE_BLINDS_HORIZONTAL:
            stride_size = image_width * 2;
            trans_size = stride_size;
            inc_offset = stride_size*8;
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size*step_no));
            ptr16_startB = (void *)((unsigned long)buf565_current+(stride_size*step_no));
            for (idx=0; idx<(image_height/8); idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_size);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+inc_offset);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+inc_offset);
            }            
            break;
            
        case GFX_SLIDE_BLINDS_VERTICAL:
            stride_size = image_width * 2;
            trans_size = 2;
            inc_offset = 8*trans_size;
            for (i=0; i<image_height; i++)
            {
                ptr16_startA = (void *)((unsigned long)buf565_output+step_no*trans_size+i*stride_size);
                ptr16_startB = (void *)((unsigned long)buf565_current+step_no*trans_size+i*stride_size);
                for (j=0; j<(image_width)/8; j++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+inc_offset);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+inc_offset);
                }    
            }        
            break;

        case GFX_SLIDE_SPLIT_HORIZONTAL_IN:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_height/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = stride_size * step_no * 4; // half size
                trans_size = image_size - (offset * 2); // top and bottom
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                memcpy(ptr16_startA, ptr16_startB, trans_size);
            }               
            break;
            
        case GFX_SLIDE_SPLIT_HORIZONTAL_OUT:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_height/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                trans_size = stride_size * step_no * 8;
                offset = (image_size - trans_size) / 2;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)buf565_current+offset);
                memcpy(ptr16_startA, ptr16_startB, trans_size);
            }                     
            break;        

        case GFX_SLIDE_SPLIT_VERTICAL_IN:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_width/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                offset = step_no * 8; // half size
                trans_size = stride_size - (offset * 2); // left and right
                memcpy(buf565_output, buf565_current, image_size); 
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<image_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }    
            }                
            break;
            
        case GFX_SLIDE_SPLIT_VERTICAL_OUT:
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==(image_width/8))
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else
            {        
                stride_size = image_width * 2;
                trans_size = step_no * 8 * 2;
                offset = (stride_size - trans_size) / 2;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)ptr16_current+offset);
                for (idx=0; idx<image_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }    
            }                    
            break;  

        case GFX_SLIDE_CHECKERBOARD_ACROSS:
            stride_size = image_width * 2;
            for (i1=0; i1<(image_height/8); i1++)
            {
                for (i2=0; i2<8; i2++)
                {   
                    if ((i1%2)==0) 
                    {
                        offset = stride_size * (i1*8+i2) + step_no * 2;
                    }
                    else
                    {
                        if (step_no < 4)
                        {
                            offset = stride_size * (i1*8+i2) + (step_no+4) * 2;
                        }
                        else
                        {
                            offset = stride_size * (i1*8+i2) + (step_no-4) * 2;
                        }
                    }
                    
                    ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                    ptr16_startB = (void *)((unsigned long)ptr16_current+offset); 
                    for (j=0; j<(image_width/8); j++)
                    {
                        memcpy(ptr16_startA, ptr16_startB, 2);
                        ptr16_startA = (void *)((unsigned long)ptr16_startA+16); // 8 pixels
                        ptr16_startB = (void *)((unsigned long)ptr16_startB+16); // 8 pixels
                    }
                }      
            }
            break;
            
        case GFX_SLIDE_CHECKERBOARD_DOWN:
            stride_size = image_width * 2;
            for (i1=0; i1<(image_width/8); i1++)
            {
                for (i2=0; i2<8; i2++)
                {   
                    if ((i1%2)==0) 
                    {
                        offset = stride_size * step_no + (i1*8+i2) * 2;
                    }
                    else
                    {
                        if (step_no < 4)
                        {
                            offset = stride_size * (step_no+4) + (i1*8+i2) * 2;
                        }
                        else
                        {
                            offset = stride_size * (step_no-4) + (i1*8+i2) * 2;
                        }
                    }
                    
                    ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                    ptr16_startB = (void *)((unsigned long)ptr16_current+offset); 
                    for (j=0; j<(image_height/8); j++)
                    {
                        memcpy(ptr16_startA, ptr16_startB, 2);
                        ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size*8); // 8 lines
                        ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size*8); // 8 lines
                    }
                }      
            }            
            break;
                  
        case GFX_SLIDE_BOX_IN: 
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else 
            {    
                memcpy(buf565_output, buf565_current, image_size); 
                trans_width = (image_width / 8) * (8-step_no);
                trans_height = (image_height / 8) * (8-step_no);   
                trans_size = trans_width * 2;
                stride_size = image_width * 2;
                sx = (image_width - trans_width) / 2;
                sy = (image_height - trans_height) / 2;
                offset = stride_size * sy + sx * 2;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)ptr16_previous+offset);
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }
            }            
            break;

        case GFX_SLIDE_BOX_OUT: 
            step_no++;
            image_size = image_width * image_height * 2;
            if (step_no==8)
            {
                memcpy(buf565_output, buf565_current, image_size); 
            }
            else 
            {    
                memcpy(buf565_output, buf565_previous, image_size); 
                trans_width = (image_width / 8) * step_no;
                trans_height = (image_height / 8) * step_no;   
                trans_size = trans_width * 2;
                stride_size = image_width * 2;
                sx = (image_width - trans_width) / 2;
                sy = (image_height - trans_height) / 2;
                offset = stride_size * sy + sx * 2;
                ptr16_startA = (void *)((unsigned long)buf565_output+offset);
                ptr16_startB = (void *)((unsigned long)ptr16_current+offset);
                for (idx=0; idx<trans_height; idx++)
                {
                    memcpy(ptr16_startA, ptr16_startB, trans_size);
                    ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                    ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);                    
                }
            }            
            break;        
                              
        case GFX_SLIDE_FADE:
            if (step_no==15)
            {
                memcpy(buf565_output, buf565_current, image_width*image_height*2);
            }
            else
            {
                step_no++;
                for (idx=0; idx<(image_width*image_height); idx++)
                {
                    R_1 = (*ptr16_current & 0xf800) >> 8;
                    G_1 = (*ptr16_current & 0x07e0) >> 3;
                    B_1 = (*ptr16_current & 0x001f) << 3;
                    ptr16_current++;
                    R_2 = (*ptr16_previous & 0xf800) >> 8;
                    G_2 = (*ptr16_previous & 0x07e0) >> 3;
                    B_2 = (*ptr16_previous & 0x001f) << 3;
                    ptr16_previous++;
                    R = (R_1 * step_no + R_2 * (16-step_no))>> 4;
                    G = (G_1 * step_no + G_2 * (16-step_no)) >> 4;
                    B = (B_1 * step_no + B_2 * (16-step_no)) >> 4;
                    *ptr16_output++ = (unsigned short)(((R & 0x00f8) << 8) | ((G & 0x00fc) << 3) | ((B & 0x00f8) >> 3));     
                }
            }            
            break;
            
        case GFX_SLIDE_FADE_FROM_BLACK:
            if (step_no==15) 
            {
                memcpy(buf565_output, buf565_current, image_width*image_height*2);
            }    
            else
            {
                step_no++;
                for (idx=0; idx<(image_width*image_height); idx++)
                {
                    R_1 = (*ptr16_current & 0xf800) >> 8;
                    G_1 = (*ptr16_current & 0x07e0) >> 3;
                    B_1 = (*ptr16_current & 0x001f) << 3;
                    ptr16_current++;
                    R = (R_1 * step_no) >> 4;
                    G = (G_1 * step_no) >> 4;
                    B = (B_1 * step_no) >> 4;
                    *ptr16_output++ = (unsigned short)(((R & 0x00f8) << 8) | ((G & 0x00fc) << 3) | ((B & 0x00f8) >> 3));     
                }
            }    
            break;  

        case GFX_SLIDE_WIPE_DOWN:
            step_no++;
            trans_size = image_width * 2 * step_no * 8;
            ptr16_startA = buf565_output;
            ptr16_startB = buf565_current;  
            memcpy(ptr16_startA, ptr16_startB, trans_size);            
            break;

        case GFX_SLIDE_WIPE_LEFT:
            step_no++;
            stride_size = image_width * 2;
            trans_size = step_no * 8 * 2; // line transfer size
            ptr16_startA = (void *)((unsigned long)buf565_output+(stride_size-trans_size));
            ptr16_startB = (void *)((unsigned long)buf565_current+(stride_size-trans_size));
            for (idx=0; idx<image_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_size);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }    
            break;

        case GFX_SLIDE_WIPE_RIGHT:
            step_no++;
            stride_size = image_width * 2;
            trans_size = step_no * 8 * 2; // line transfer size
            ptr16_startA = buf565_output;
            ptr16_startB = buf565_current;
            for (idx=0; idx<image_height; idx++)
            {
                memcpy(ptr16_startA, ptr16_startB, trans_size);
                ptr16_startA = (void *)((unsigned long)ptr16_startA+stride_size);
                ptr16_startB = (void *)((unsigned long)ptr16_startB+stride_size);
            }                
            break;

        case GFX_SLIDE_WIPE_UP:
            step_no++;
            image_size = image_width * image_height * 2;
            trans_size = image_width * 2 * step_no * 8;
            ptr16_startA = (void *)((unsigned long)buf565_output+(image_size-trans_size));
            ptr16_startB = (void *)((unsigned long)buf565_current+(image_size-trans_size)); 
            memcpy(ptr16_startA, ptr16_startB, trans_size);     
            break;
                    
        default:
            memcpy(buf565_output, buf565_current, image_width*image_height*2);                 
    }
    
    return GFX_OK;
}



