///////////////////////////////////////////////////////////////////////////////
//
//  This is an internal used function to initialize the LCM for testing
//  graphics drivers. 
//
//  For real implementation, the application should call the VPOST library.
//
//  2004/12/02  Add option for Samsung 18-bit panel
//
///////////////////////////////////////////////////////////////////////////////
#include "wbio.h"
#include "gfxlib.h"
#include "global.h"

#include "w99702_reg.h"

/*--- definition of output device ---*/

#define SAMSUNG_16  1
#define SAMSUNG_18  2

#define OPT_LCM_TYPE    SAMSUNG_16


///////////////////////////////////////////////////////////////////////////////
//
//  delay_long()
//
///////////////////////////////////////////////////////////////////////////////
static VOID delay_long(UINT16 delay_cnt)
{
  UINT16 i=delay_cnt; 
  
  while (i--); 
}


///////////////////////////////////////////////////////////////////////////////
//
//  LCDWriteCMDAddrWORD()
//
///////////////////////////////////////////////////////////////////////////////
VOID LCDWriteCMDAddrWORD(UINT16 cmd)
{
  UINT32 temp32;
  
  temp32 = inpw(REG_LCM_MPU_CMD) & 0xbfff0000; // RS=0
  outpw(REG_LCM_MPU_CMD, temp32);
  // 0xdfff0000 ?
  temp32 = inpw(REG_LCM_MPU_CMD) & 0xdfffffff; // R/W
  outpw(REG_LCM_MPU_CMD, temp32);
  
  /* Samsung LCM */
  
  temp32 = inpw(REG_LCM_DEV_CTRL) | 0x20000000; // 16-bit command
  outpw(REG_LCM_DEV_CTRL, temp32);
  
  temp32 = inpw(REG_LCM_DCCS) | 0x00000020; // command mode
  outpw(REG_LCM_DCCS, temp32);
  
  temp32 = inpw(REG_LCM_MPU_CMD) | cmd;
  outpw(REG_LCM_MPU_CMD, temp32);
  
  while ((inpw(REG_LCM_MPU_CMD) & 0x80000000) != 0); // check busy bit
  
  temp32 = inpw(REG_LCM_DCCS) & 0xffffffdf; // display mode
  outpw(REG_LCM_DCCS, temp32);  
}


///////////////////////////////////////////////////////////////////////////////
//
//  LCDWriteCMDRegWORD()
//
///////////////////////////////////////////////////////////////////////////////
VOID LCDWriteCMDRegWORD(UINT16 cmd)
{
  UINT32 temp32;
  
  temp32 = inpw(REG_LCM_MPU_CMD) | 0x40000000; // RS=1
  outpw(REG_LCM_MPU_CMD, temp32);
  temp32 = inpw(REG_LCM_MPU_CMD) & 0xdfff0000; // R/W
  outpw(REG_LCM_MPU_CMD, temp32);
  
  temp32 = inpw(REG_LCM_DEV_CTRL) | 0x20000000; // 16-bit command
  outpw(REG_LCM_DEV_CTRL, temp32);
  
  temp32 = inpw(REG_LCM_DCCS) | 0x00000020; // command mode
  outpw(REG_LCM_DCCS, temp32);
  
  temp32 = inpw(REG_LCM_MPU_CMD) | cmd;
  outpw(REG_LCM_MPU_CMD, temp32);
  
  while ((inpw(REG_LCM_MPU_CMD) & 0x80000000) != 0); // check busy bit
  
  temp32 = inpw(REG_LCM_DCCS) & 0xffffffdf; // display mode
  outpw(REG_LCM_DCCS, temp32);  
}


///////////////////////////////////////////////////////////////////////////////
//
//  gfxInitLCM()
//
//  This is an internal used function.
//  For real application, it must be supported by VPOST library.
//
///////////////////////////////////////////////////////////////////////////////
#if (OPT_LCM_TYPE==SAMSUNG_16)

VOID gfxInitLCM(UINT32 addr, GFX_COLOR_FORMAT_E fmt, INT pitch)
{
    UINT32 temp32;
    UINT32 stride;

    // change to bridge mode
    *((UINT32 *)0x7ff0000c) = 0; // select panel  
  
    /* 
    ** VA Only: 
    */
    switch (fmt)
    {
        case GFX_BPP_888: 
            outpw(REG_LCM_DCCS, 0x00000200); // display OFF   
            break;
        
        case GFX_BPP_666:
            outpw(REG_LCM_DCCS, 0x00000300); // display OFF   
            break;
                        
        case GFX_BPP_565:
            outpw(REG_LCM_DCCS, 0x00000400); // display OFF   
            break;
          
        case GFX_BPP_444L:
            outpw(REG_LCM_DCCS, 0x00000500); // display OFF 
            break;   

        case GFX_BPP_332:
            outpw(REG_LCM_DCCS, 0x00000600); // display OFF 
            break;
            
        case GFX_BPP_444H:
            outpw(REG_LCM_DCCS, 0x00000700); // display OFF 
            break;            
    }            
  
  outpw(REG_LCM_DEV_CTRL, 0xa50000e0);
  
  /* CRTC programming */
  
  outpw(REG_LCM_CRTC_SIZE,  0x00e000c0);
  outpw(REG_LCM_CRTC_DEND,  0x00a10080); // why 161 lines?
  outpw(REG_LCM_CRTC_HR,    0x008f008a);
  outpw(REG_LCM_CRTC_HSYNC, 0x00bb00b6);
  outpw(REG_LCM_CRTC_VR,    0x00c000a0);
  
  outpw(REG_LCM_VA_BADDR0, addr);
  outpw(REG_LCM_VA_BADDR1, addr);
  //outpw(REG_LCM_VA_FBCTRL, 0x00400040); // offset
  stride = pitch >> 2;
  outpw(REG_LCM_VA_FBCTRL, (stride << 16) | stride);
  outpw(REG_LCM_VA_SCALE, 0x04000400); // VA scaling
  
  outpw(REG_LCM_OSD_WIN_S, 0x00000000);
  outpw(REG_LCM_OSD_WIN_E, 0x00000000);
  outpw(REG_LCM_OSD_BADDR, 0x00000000);
  outpw(REG_LCM_OSD_FBCTRL, 0x00000000);
  
  outpw(REG_LCM_OSD_OVERLAY, 0x00000000);
  outpw(REG_LCM_OSD_CKEY, 0x00000000);
  outpw(REG_LCM_OSD_CMASK, 0x00000000);

  outpw(REG_LCM_DCCS, inpw(REG_LCM_DCCS) | 0x00000008); // display ON

  delay_long(4000);
  
  /* Samsung LCM initialization */
  
  LCDWriteCMDAddrWORD(0x01);    
  LCDWriteCMDRegWORD(0x0115);
  LCDWriteCMDAddrWORD(0x02);	
  LCDWriteCMDRegWORD(0x0700);	        
  LCDWriteCMDAddrWORD(0x05);	
  LCDWriteCMDRegWORD(0x1230);	        
  LCDWriteCMDAddrWORD(0x06);	
  LCDWriteCMDRegWORD(0x0000);        	
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0104);	    
  LCDWriteCMDAddrWORD(0x0b);	
  LCDWriteCMDRegWORD(0x0000);	
  
  LCDWriteCMDAddrWORD(0x0c);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x0d);	
  LCDWriteCMDRegWORD(0x0401);	    
  LCDWriteCMDAddrWORD(0x0e);	
  LCDWriteCMDRegWORD(0x0d18);
  delay_long(4000);
  LCDWriteCMDAddrWORD(0x03);	
  LCDWriteCMDRegWORD(0x0214);	    
  LCDWriteCMDAddrWORD(0x04);	
  LCDWriteCMDRegWORD(0x8000);	
  delay_long(4000);	
  LCDWriteCMDAddrWORD(0x0e);	
  LCDWriteCMDRegWORD(0x2910);	
  delay_long(4000);
  LCDWriteCMDAddrWORD(0x0d);	
  LCDWriteCMDRegWORD(0x0512);  
  
  LCDWriteCMDAddrWORD(0x21);	
  LCDWriteCMDRegWORD(0x0100);	    
  LCDWriteCMDAddrWORD(0x30);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x31);	
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x32);	
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x33);	
  LCDWriteCMDRegWORD(0x0000);	    	
  LCDWriteCMDAddrWORD(0x34);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x35);	
  LCDWriteCMDRegWORD(0x0707);		
  LCDWriteCMDAddrWORD(0x36);	
  LCDWriteCMDRegWORD(0x0707);		
  LCDWriteCMDAddrWORD(0x37);	
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x0f);	
  LCDWriteCMDRegWORD(0x0000);	    	
  LCDWriteCMDAddrWORD(0x11);	
  LCDWriteCMDRegWORD(0x0000);	    	
  LCDWriteCMDAddrWORD(0x14);	
  LCDWriteCMDRegWORD(0x5c00);	    
  LCDWriteCMDAddrWORD(0x15);	
  LCDWriteCMDRegWORD(0xa05d);		
  LCDWriteCMDAddrWORD(0x16);	
  LCDWriteCMDRegWORD(0x7f00);		
  LCDWriteCMDAddrWORD(0x17);	
  LCDWriteCMDRegWORD(0xA000);	    
  LCDWriteCMDAddrWORD(0x3A);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x3B);	
  LCDWriteCMDRegWORD(0x0000);  
  
  /* Samsung display ON */
   
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0105);	
  delay_long(4000);
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0125);	    
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0127);	
  delay_long(4000);
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0137);	    
  LCDWriteCMDAddrWORD(0x21);    
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x22);  
  
  temp32 = inpw(REG_LCM_DCCS) | 0x00000002; // VA ON
  outpw(REG_LCM_DCCS, temp32);
}

#endif /* SAMSUNG_16 */


#if (OPT_LCM_TYPE==SAMSUNG_18)

VOID gfxInitLCM(UINT32 addr, GFX_COLOR_FORMAT_E fmt, INT pitch)
{
    UINT32 temp32;
    int stride;

    // change to bridge mode
    *((UINT32 *)0x7ff0000c) = 0; // select panel  
  
    /* 
    ** VA Only: 
    */
    switch (fmt)
    {
        case GFX_BPP_888: 
            outpw(REG_LCM_DCCS, 0x00000200); // display OFF   
            break;
        
        case GFX_BPP_666:
            outpw(REG_LCM_DCCS, 0x00000300); // display OFF   
            break;
                        
        case GFX_BPP_565:
            outpw(REG_LCM_DCCS, 0x00000400); // display OFF   
            break;
          
        case GFX_BPP_444L:
            outpw(REG_LCM_DCCS, 0x00000500); // display OFF 
            break;   

        case GFX_BPP_332:
            outpw(REG_LCM_DCCS, 0x00000600); // display OFF 
            break;
            
        case GFX_BPP_444H:
            outpw(REG_LCM_DCCS, 0x00000700); // display OFF 
            break;            
    }   
  
    outpw(REG_LCM_DEV_CTRL, 0xa60000e0); // 18-bit 262K
  
    /* CRTC programming */ 
  
    outpw(REG_LCM_CRTC_SIZE,  0x00e000c0);
    outpw(REG_LCM_CRTC_DEND,  0x00a00080); // 160 lines
    outpw(REG_LCM_CRTC_HR,    0x008f008a);
    outpw(REG_LCM_CRTC_HSYNC, 0x00bb00b6);
    outpw(REG_LCM_CRTC_VR,    0x00c000a0);
  
    outpw(REG_LCM_VA_BADDR0, addr);
    outpw(REG_LCM_VA_BADDR1, addr);
    //outpw(REG_LCM_VA_FBCTRL, 0x00400040); // offset
    stride = pitch >> 2;
    outpw(REG_LCM_VA_FBCTRL, (stride << 16) | stride);
    outpw(REG_LCM_VA_SCALE, 0x04000400); // VA scaling
  
    outpw(REG_LCM_OSD_WIN_S, 0x00000000);
    outpw(REG_LCM_OSD_WIN_E, 0x00000000);
    outpw(REG_LCM_OSD_BADDR, 0x00000000);
    outpw(REG_LCM_OSD_FBCTRL, 0x00000000);
  
    outpw(REG_LCM_OSD_OVERLAY, 0x00000000);
    outpw(REG_LCM_OSD_CKEY, 0x00000000);
    outpw(REG_LCM_OSD_CMASK, 0x00000000);

    outpw(REG_LCM_DCCS, inpw(REG_LCM_DCCS) | 0x00000008); // display ON

    delay_long(4000);
  
    /* Samsung LCM initialization */
  
    LCDWriteCMDAddrWORD(0x01);    
    LCDWriteCMDRegWORD(0x0200);
    LCDWriteCMDAddrWORD(0x02);	
    LCDWriteCMDRegWORD(0x0700);	        
    LCDWriteCMDAddrWORD(0x03);	
    LCDWriteCMDRegWORD(0x0220);	        
    LCDWriteCMDAddrWORD(0x04);	
    LCDWriteCMDRegWORD(0x0000);        	
    LCDWriteCMDAddrWORD(0x08);	
    LCDWriteCMDRegWORD(0x0626);	    
    LCDWriteCMDAddrWORD(0x09);	
    LCDWriteCMDRegWORD(0x0000);	
    LCDWriteCMDAddrWORD(0x0b);	
    LCDWriteCMDRegWORD(0x0000);	    
    LCDWriteCMDAddrWORD(0x0c);	
    LCDWriteCMDRegWORD(0x0000);	    
    LCDWriteCMDAddrWORD(0x0f);	
    LCDWriteCMDRegWORD(0x0000);
    delay_long(4000);
 
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0008);	    
  LCDWriteCMDAddrWORD(0x11);	
  LCDWriteCMDRegWORD(0x0067);	
  LCDWriteCMDAddrWORD(0x12);	
  LCDWriteCMDRegWORD(0x0f9);	
  LCDWriteCMDAddrWORD(0x13);	
  LCDWriteCMDRegWORD(0x121b);	
  LCDWriteCMDAddrWORD(0x10);	
  LCDWriteCMDRegWORD(0x0010);	
  LCDWriteCMDAddrWORD(0x12);	
  LCDWriteCMDRegWORD(0x0f19);	
  delay_long(8000);
  LCDWriteCMDAddrWORD(0x13);	
  LCDWriteCMDRegWORD(0x321b);	
  LCDWriteCMDAddrWORD(0x11);	
  LCDWriteCMDRegWORD(0x0297);	
  LCDWriteCMDAddrWORD(0x10);	
  LCDWriteCMDRegWORD(0x0210);	
  LCDWriteCMDAddrWORD(0x12);	
  //LCDWriteCMDRegWORD(0x3f19);	
  LCDWriteCMDRegWORD(0x3f1e); // changed on 2005-01-07	
  delay_long(12000);

  LCDWriteCMDAddrWORD(0x23);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x24);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x50);	
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x51);	
  LCDWriteCMDRegWORD(0x007f);		
  LCDWriteCMDAddrWORD(0x52);	
  LCDWriteCMDRegWORD(0x0000);	    	
  LCDWriteCMDAddrWORD(0x53);	
  LCDWriteCMDRegWORD(0x009f);	    
  LCDWriteCMDAddrWORD(0x60);	
  LCDWriteCMDRegWORD(0x9300);		
  LCDWriteCMDAddrWORD(0x61);	
  LCDWriteCMDRegWORD(0x0001);		
  LCDWriteCMDAddrWORD(0x68);	
  LCDWriteCMDRegWORD(0x0000);		
  LCDWriteCMDAddrWORD(0x69);	
  LCDWriteCMDRegWORD(0x009f);	    	
  LCDWriteCMDAddrWORD(0x6a);	
  LCDWriteCMDRegWORD(0x0000);	    	
  LCDWriteCMDAddrWORD(0x70);	
  LCDWriteCMDRegWORD(0x8b14);	    
  LCDWriteCMDAddrWORD(0x71);	
  LCDWriteCMDRegWORD(0x0001);		
  LCDWriteCMDAddrWORD(0x78);	
  LCDWriteCMDRegWORD(0x00a0);		
  LCDWriteCMDAddrWORD(0x79);	
  LCDWriteCMDRegWORD(0x00ff);	    
  LCDWriteCMDAddrWORD(0x7a);	
  LCDWriteCMDRegWORD(0x0000);	    
  LCDWriteCMDAddrWORD(0x80);	
  LCDWriteCMDRegWORD(0x0000);  
  LCDWriteCMDAddrWORD(0x81);	
  LCDWriteCMDRegWORD(0x0000); 
  LCDWriteCMDAddrWORD(0x82);	
  LCDWriteCMDRegWORD(0x0000);  
  LCDWriteCMDAddrWORD(0x83);	
  LCDWriteCMDRegWORD(0x0000);
  LCDWriteCMDAddrWORD(0x84);	
  LCDWriteCMDRegWORD(0x0000);
  LCDWriteCMDAddrWORD(0x85);	
  LCDWriteCMDRegWORD(0x0000);
  LCDWriteCMDAddrWORD(0x86);	
  LCDWriteCMDRegWORD(0x0000);
  LCDWriteCMDAddrWORD(0x87);	
  LCDWriteCMDRegWORD(0x0000);    
  LCDWriteCMDAddrWORD(0x88);	
  LCDWriteCMDRegWORD(0x0000);  
  delay_long(8000);
  LCDWriteCMDAddrWORD(0x30);	
  LCDWriteCMDRegWORD(0x0700);  
  LCDWriteCMDAddrWORD(0x31);	
  LCDWriteCMDRegWORD(0x0007);  
  LCDWriteCMDAddrWORD(0x32);	
  LCDWriteCMDRegWORD(0x0000);  
  LCDWriteCMDAddrWORD(0x33);	
  LCDWriteCMDRegWORD(0x0100);  
  LCDWriteCMDAddrWORD(0x34);	
  LCDWriteCMDRegWORD(0x0707);  
  LCDWriteCMDAddrWORD(0x35);	
  LCDWriteCMDRegWORD(0x0007);  
  LCDWriteCMDAddrWORD(0x36);	
  LCDWriteCMDRegWORD(0x0700);  
  LCDWriteCMDAddrWORD(0x37);	
  LCDWriteCMDRegWORD(0x0001);  
  LCDWriteCMDAddrWORD(0x38);	
  LCDWriteCMDRegWORD(0x1800);  
  LCDWriteCMDAddrWORD(0x39);	
  LCDWriteCMDRegWORD(0x0007);  
  delay_long(400);
  LCDWriteCMDAddrWORD(0x40);	
  LCDWriteCMDRegWORD(0x0700);  
  LCDWriteCMDAddrWORD(0x41);	
  LCDWriteCMDRegWORD(0x0007);  
  LCDWriteCMDAddrWORD(0x42);	
  LCDWriteCMDRegWORD(0x0000);  
  LCDWriteCMDAddrWORD(0x43);	
  LCDWriteCMDRegWORD(0x0100);  
  LCDWriteCMDAddrWORD(0x44);	
  LCDWriteCMDRegWORD(0x0707);  
  LCDWriteCMDAddrWORD(0x45);	
  LCDWriteCMDRegWORD(0x0007);  
  LCDWriteCMDAddrWORD(0x46);	
  LCDWriteCMDRegWORD(0x0700);  
  LCDWriteCMDAddrWORD(0x47);	
  LCDWriteCMDRegWORD(0x0001);  
  LCDWriteCMDAddrWORD(0x48);	
  LCDWriteCMDRegWORD(0x1800);  
  LCDWriteCMDAddrWORD(0x49);	
  LCDWriteCMDRegWORD(0x0007);  
  delay_long(400);
  
  
  /* Samsung display ON */
   
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0301);	
  delay_long(12000);
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0321);	    
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0323);	
  delay_long(12000);
  LCDWriteCMDAddrWORD(0x07);	
  LCDWriteCMDRegWORD(0x0333);	
  delay_long(8000);
  LCDWriteCMDAddrWORD(0x20);    
  LCDWriteCMDRegWORD(0x007f);		
  LCDWriteCMDAddrWORD(0x21);  
  LCDWriteCMDRegWORD(0x0000);
  LCDWriteCMDAddrWORD(0x22);  
   
  temp32 = inpw(REG_LCM_DCCS) | 0x00000002; // VA ON
  outpw(REG_LCM_DCCS, temp32);
}

#endif /* SAMSUNG_18 */
