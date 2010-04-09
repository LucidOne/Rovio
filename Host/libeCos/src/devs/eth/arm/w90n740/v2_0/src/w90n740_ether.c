//==========================================================================
//
//      w90n740_ether.c
//
//      
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Winbond.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    clyu
// Contributors: clyu
// Date:         2004-07-20
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include "pkgconf/system.h"
#include "pkgconf/devs_eth_arm_w90n740.h"
#include "pkgconf/io_eth_drivers.h"
#include "cyg/hal/hal_io.h"

#if defined(CYGPKG_IO)
#include "pkgconf/io.h"
#include "cyg/io/io.h"
#include "cyg/io/devtab.h"
#else
// need to provide fake values for errno
#define EIO 1
#define EINVAL 2
#endif

#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
#include "cyg/infra/diag.h"
#include "cyg/io/eth/netdev.h"
#include "cyg/io/eth/eth_drv.h"
#include "cyg/io/eth/eth_drv_stats.h"
#include "cyg/hal/hal_intr.h"

#if defined(CYGPKG_REDBOOT)
#include "pkgconf/redboot.h"
#endif

#if !defined(CYGPKG_NET)
#define cyg_drv_interrupt_unmask(v) /* noop */
#define cyg_drv_interrupt_mask(v)   /* noop */
#define cyg_drv_isr_lock()          /* noop */
#define cyg_drv_isr_unlock()        /* noop */
#define cyg_drv_mutex_init(m)       /* noop */
#define cyg_drv_mutex_lock(m)       /* noop */
#define cyg_drv_mutex_unlock(m)     /* noop */
#define cyg_drv_dsr_lock()          /* noop */
#define cyg_drv_dsr_unlock()        /* noop */
#endif

#define HavePHY 1
#define HavePHYinterrupt 0

#define	USEMAC	1

#if	USEMAC
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_TX   CYGNUM_HAL_INTERRUPT_ETH_MAC_TX1
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_RX   CYGNUM_HAL_INTERRUPT_ETH_MAC_RX1
#else
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_TX   CYGNUM_HAL_INTERRUPT_ETH_MAC_TX0
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_RX   CYGNUM_HAL_INTERRUPT_ETH_MAC_RX0
#endif

#include "std.h"
#include "w740mac.h"

#if HavePHY
#include "phy.h"
#endif


void w740_WriteCam(int which,int x, unsigned char *pval);
void ResetMAC(struct eth_drv_sc * sc);
static bool w90n740_eth_init(struct cyg_netdevtab_entry *tab);

// Set up the level of debug output
#if CYGPKG_DEVS_ETH_ARM_W90N740_DEBUG_LEVEL > 0
#define debug1_printf(args...) diag_printf(args)
#define MAC_ASSERT(x)							 \
	do {									 \
		if (!(x))							 \
			diag_printf("ASSERT: %s:%i(%s)\n",				 \
			       __FILE__, __LINE__, __FUNCTION__);		 \
	} while(0);
#else
#define debug1_printf(args...) /* noop */
#define MAC_ASSERT(x)
#endif
#if CYGPKG_DEVS_ETH_ARM_W90N740_DEBUG_LEVEL > 1
#define debug2_printf(args...) diag_printf(args)
#else
#define debug2_printf(args...) /* noop */
#endif

#define EtherFramePadding 2

/* Global variables  used for MAC driver */
static unsigned long  	gMCMDR = MCMDR_SPCRC | MCMDR_EnMDC | MCMDR_ACP;//|MCMDR_LBK;

static unsigned long  	gMIEN = EnTXINTR | EnRXINTR | EnRXGD | EnTXCP |
                        EnTxBErr | EnRxBErr | EnTXABT | EnRDU;//| EnTXEMP;//EnDEN

#define Bit(n) (1<<(n))

// enable/disable software verification of rx CRC
// should be moved to user-controlled valud in CDL file

// miscellaneous data structures
#define	ETHER_ADDR_LEN	6
typedef  BYTE  ETH_ADDR[6];
#define MAX_ETH_FRAME_SIZE 1600
// number of ethernet buffers should be enough to keep both rx
// and tx queues full plus some extras for in-process packets

#if defined(CYGPKG_REDBOOT)
#define MAX_RX_FRAME_DESCRIPTORS        4     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	4     // Max number of Tx Frame Descriptors
#else
#define MAX_RX_FRAME_DESCRIPTORS        32     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	32     // Max number of Tx Frame Descriptors
#endif

#define PACKET_SIZE   1560

   
// don't have any private data, but if we did, this is where it would go
typedef struct  w740_priv
{
	RXBD volatile  rx_desc[MAX_RX_FRAME_DESCRIPTORS];//__align(16) 
	TXBD volatile  tx_desc[MAX_TX_FRAME_DESCRIPTORS];//__align(16) 
	char volatile  rx_buf[MAX_RX_FRAME_DESCRIPTORS][MAX_ETH_FRAME_SIZE];//__align(16) 
	char volatile  tx_buf[MAX_TX_FRAME_DESCRIPTORS][MAX_ETH_FRAME_SIZE];//__align(16) 
	U32 which;
	U32 rx_mode;
	U32 volatile cur_tx_entry;
	U32 volatile cur_rx_entry;
	U32 volatile is_rx_all;
	U32 volatile status;

	U32 bInit;
	U32 rx_packets;
	U32 rx_bytes;
	U32 start_time;
	
	unsigned long volatile tx_ptr;
	unsigned long tx_finish_ptr;
	unsigned long volatile rx_ptr;
	
	unsigned long start_tx_ptr;
	unsigned long start_tx_buf;
	
	unsigned long mcmdr;
	unsigned long volatile start_rx_ptr;
	unsigned long volatile start_rx_buf;
	
	char 		  mac_address[ETHER_ADDR_LEN];
}w90n740_priv_data_t;

__align(16)  w90n740_priv_data_t w90n740_priv_data0;

#if HavePHY
// functions to read/write Phy chip registers via MII interface
// on w90n740.  These need to be non-static since they're used
// by PHY-specific routines in a different file.
#define PHYREGWRITE	0x0400
#define MiiStart       0x0800

U32 MiiStationWrite(U32 num , U32 RegAddr, U32 PhyAddr, U32 PhyWrData)
{
	volatile int i = 1000;
 	int which=num;
    volatile int loop=1000*100;
#ifdef IC_PLUS1
	num = 0;
#endif	
	which=num;
	w740_WriteReg(MIID,PhyWrData,which);
	w740_WriteReg(MIIDA,RegAddr|PhyAddr|PHYBUSY|PHYWR|MDCCR1,which);
	while(i--);
	while((w740_ReadReg(MIIDA,which) &PHYBUSY))
	{
   		loop--;
   		if(loop==0)
   			return 1;
   }
   //diag_printf("MiiStationWrite 1\n");
   return 0;
  //debug1_printf("PHY Wr %x:%02x := %04x\n",PhyAddr, RegAddr, PhyWrData) ;
}

U32 MiiStationRead(U32 num, U32 RegAddr, U32 PhyAddr)
{ 
   	unsigned long PhyRdData ;
 	int which=num;
 	volatile int loop=1000*100;

#ifdef  IC_PLUS1
	num = 0;
#endif	
	which=num;
#define MDCCR1   0x00b00000  // MDC clock rating
    w740_WriteReg(MIIDA, RegAddr | PhyAddr | PHYBUSY | MDCCR1,which);
    while( (w740_ReadReg(MIIDA,which)& PHYBUSY) ) 
    {
    	loop--;
    	if(loop==0)
     		return (unsigned long)1;
    }
    
    PhyRdData = w740_ReadReg(MIID,which) ; 
 	return PhyRdData ;
}
#endif

typedef struct
{
  U8 DestinationAddr[6];
  U8 SourceAddr[6];
  U8 LengthOrType[2];
  U8 LLCData[1506];
} MAC_FRAME;

#if defined(CYGPKG_NET)
static cyg_drv_mutex_t txMutex;
struct ether_drv_stats ifStats;
#endif

// interrupt entry counters
U32 w90n740_MAC_Rx_Cnt;
U32 w90n740_MAC_Tx_Cnt;
U32 w90n740_MAC_Phy_Cnt;


static cyg_drv_mutex_t oldRxMutex;
static cyg_drv_cond_t  oldRxCond;


static bool configDone;

/*----------------------------------------------------------------------
 * Init Data structures used to send/recv package
 */
static void init_rxtx_rings(struct eth_drv_sc *dev)
{
	int i;
	struct w740_priv * w740_priv = (struct w740_priv *)dev->driver_private;
	
	w740_priv->start_tx_ptr =(unsigned long)&w740_priv->tx_desc[0];	
	w740_priv->start_tx_buf =(unsigned long)&w740_priv->tx_buf[0];
	
	w740_priv->start_rx_ptr =(unsigned long)&w740_priv->rx_desc[0];
	w740_priv->start_rx_buf =(unsigned long)&w740_priv->rx_buf[0];
	
	
	//Tx Ring
	MAC_ASSERT(w740_priv->start_tx_ptr );
	MAC_ASSERT(w740_priv->start_tx_buf );
	debug1_printf(" tx which %d start_tx_ptr %x\n",w740_priv->which,w740_priv->start_tx_ptr);

	for ( i = 0 ; i < MAX_TX_FRAME_DESCRIPTORS ; i++ )
	{
	   	w740_priv->tx_desc[i].SL=0;
	   	w740_priv->tx_desc[i].mode=0;
		w740_priv->tx_desc[i].buffer=(unsigned long)&w740_priv->tx_buf[i];	
		w740_priv->tx_desc[i].next=(unsigned long)&w740_priv->tx_desc[i+1];
	}
	w740_priv->tx_desc[i-1].next=(unsigned long)&w740_priv->tx_desc[0];	
	
	//Rx Ring
	MAC_ASSERT(w740_priv->start_rx_ptr );
	MAC_ASSERT(w740_priv->start_rx_buf );	
    
    for( i =0 ; i < MAX_RX_FRAME_DESCRIPTORS ; i++)
    {    
	    w740_priv->rx_desc[i].SL=RXfOwnership_DMA;
		w740_priv->rx_desc[i].buffer=(unsigned long)&w740_priv->rx_buf[i];	
		w740_priv->rx_desc[i].next=(unsigned long)&w740_priv->rx_desc[i+1];
	}	
	w740_priv->rx_desc[i-1].next=(unsigned long)&w740_priv->rx_desc[0];	
    	
}

//======================================================================
static int EthInit(struct eth_drv_sc *sc, U8* mac_address)
{
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;

	if(mac_address)
    	debug2_printf("EthInit(%02x:%02x:%02x:%02x:%02x:%02x)\n",
                mac_address[0],mac_address[1],mac_address[2],
                mac_address[3],mac_address[4],mac_address[5]);
 	else
    	diag_printf("EthInit(NULL)\n");

  // set up our MAC address

	if(mac_address)
    {
    	memcpy(priv->mac_address, mac_address, ETHER_ADDR_LEN);
  		w740_WriteCam(priv->which,0,mac_address);
    }
  
	return 0;
}

static void w90n740_handle_tx_complete(void)
{

	// record status and then free any buffers we're done with
	extern struct eth_drv_sc w90n740_sc;
    struct w740_priv *priv = (struct w740_priv *)w90n740_sc.driver_private;
    U32 status;  
    int which=priv->which;
    volatile TXBD  *txbd;
    static  unsigned  reset_rx_loop=0;
    
    status=w740_ReadReg(MISTA,which);   //get  interrupt status;
    //w740_WriteReg(MISTA,status&0xFFFF0000,which);  //clear interrupt
    w740_WriteReg(MISTA,status,which);  //clear interrupt
	
	++ifStats.interrupts;

#if 1
	while(((U32)&(priv->tx_desc[priv->cur_tx_entry])|NON_CACHE_FLAG) != (U32)w740_ReadReg(CTXDSA,which))
    {
        txbd =(TXBD *)&(priv->tx_desc[priv->cur_tx_entry]);
    	
    	debug1_printf("priv->cur_tx_entry =%d",priv->cur_tx_entry);
    	priv->cur_tx_entry = (priv->cur_tx_entry+1)%(MAX_TX_FRAME_DESCRIPTORS);
    	
    	debug1_printf(" priv->cur_tx_entry =%d\n",priv->cur_tx_entry);
	    debug1_printf("*txbd->SL %x\n",txbd->SL);
	    debug1_printf("priv->tx_ptr %x  cur_ptr =%x\n",priv->tx_ptr,cur_ptr);
	    if(txbd->SL &TXDS_TXCP)
	    {
	    	++ifStats.tx_good;
	    	++ifStats.tx_complete;
	    }
        
        txbd->SL=0; 
        txbd->mode=0;
	}
	
	if(status&MISTA_EXDEF)
	{
		diag_printf("MISTA_EXDEF\n");
		++ifStats.tx_deferred;
	}
    if((status & MISTA_RDU)&& ++reset_rx_loop==5)
    {
    	diag_printf("W90N740 MAC In Tx %d RX   I Have Not Any Descript Needed\n",priv->which);
        ResetMAC(&w90n740_sc);
        //reset_rx_loop=0;
    }
    if(status&MISTA_TxBErr)
    {
    	diag_printf("MISTA_TxBErr\n");
    	++ifStats.tx_carrier_loss;
    }
    if(status&MISTA_TDU)
    {
    	//debug1_printf("disable tx mac interrupt\n");
    	Disable_Int(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
    	//diag_printf("MISTA_TDU\n");
    }
#endif
}


//======================================================================
static cyg_uint32 MAC_Tx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  w90n740_handle_tx_complete();
  cyg_drv_interrupt_acknowledge(vector);  
  return CYG_ISR_HANDLED;
}

//======================================================================
//cyg_drv_interrupt_acknowledge(vector);
static cyg_uint32 MAC_Rx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
	struct w740_priv *priv = (struct w740_priv *)sc->driver_private;
	U32 status;
	int which=priv->which;
	RXBD *rxbd;
	
	rxbd=(RXBD *)priv->rx_ptr ;
	
	status=w740_ReadReg(MISTA,which);   //get  interrupt status;

    w740_WriteReg(MISTA,status,which); //clear interrupt
  	
  	cyg_drv_interrupt_acknowledge(vector);

	++w90n740_MAC_Rx_Cnt;

#if defined(CYGPKG_NET)
  
	++ifStats.interrupts;
	debug1_printf("rxbd=%x %x\n",rxbd,w740_ReadReg(CRXDSA,priv->which));

	if(status & (MISTA_RDU|MISTA_RxBErr))
    {
    	//diag_printf("No Descript available\n");
    	priv->is_rx_all=MAX_RX_FRAME_DESCRIPTORS; //receive all
        //netdev_rx(dev);    //start doing
  		if(status&MISTA_RxBErr)
  		{
  			diag_printf("MISTA_RxBErr %x\n",status);
        	ResetMAC(sc);
        	w740_WriteReg(RSDR ,0,which);
        	
        	return CYG_ISR_HANDLED;
        }
	    debug1_printf("* %d rx_interrupt MISTA %x \n",vector,status);
    }

#endif
	return CYG_ISR_HANDLED|CYG_ISR_CALL_DSR;
	
	//eth_drv_dsr(vector, 0, data);
	//return CYG_ISR_HANDLED;
}

static void eth_handle_recv_buffer(RXBD *);

static int ethernetRunning;
static cyg_handle_t  macRxIntrHandle;
static cyg_handle_t  macTxIntrHandle;
static cyg_interrupt  macRxIntrObject;
static cyg_interrupt  macTxIntrObject;

static void w90n740_eth_deliver(struct eth_drv_sc *sc)
{
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;
	RXBD *rxbd;
	U32 volatile length;
	U32 volatile status;
	U32 volatile SL;
	int which=priv->which;
	//int cnt = 0;
	
	if(!ethernetRunning)
		return;

	while(1)
	{
		rxbd=(RXBD *)priv->rx_ptr ;
		SL = rxbd->SL;
		length = (SL&0xFFFF);
		status = (SL&0x3FFF0000);

		//(w740_ReadReg(CRXDSA,priv->which)==(unsigned long)rxbd)
		if((SL & RXfOwnership_DMA)|| (SL & RXfOwnership_NAT))
		{
			// no buffers available
			debug1_printf("deliver current=%x rxbd=%x\n",w740_ReadReg(CRXDSA,priv->which), rxbd);
    	   	break;
    	}
    	if(!(status & RXDS_RXGD))
        {
        	diag_printf("Get Package Error %x SL %x rxbd %x\n",status, rxbd->SL, rxbd);
			rxbd->reserved = 0;
			priv->rx_ptr=(unsigned long)rxbd->next;
			rxbd->SL = RXfOwnership_DMA;
        	//break;
        	continue;
        }

#if defined (CYGPKG_NET)      
		++ifStats.rx_count;
		++ifStats.rx_deliver;
#endif
      	
      	//cnt++;
		if(ethernetRunning)
			eth_handle_recv_buffer(rxbd);
		else
		{
			rxbd->reserved = 0;
			priv->rx_ptr=(unsigned long)rxbd->next;
			rxbd->SL =RXfOwnership_DMA;
		}
    }//while(w740_ReadReg(CRXDSA,priv->which)!=(unsigned long)priv->rx_ptr);
	
	cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
	
	if(priv->is_rx_all)
	{
		priv->is_rx_all = 0;
		w740_WriteReg(RSDR ,0,which);
	}
	cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
	
	w740_WriteReg(RSDR ,0,which);
	//diag_printf("cnt=%d\n",cnt);
	
	return;
}

void w740_WriteCam(int which,int x, unsigned char *pval)
{
	
	unsigned int msw,lsw;
	
 	msw =  	(pval[0] << 24) |
        	(pval[1] << 16) |
        	(pval[2] << 8) |
         	pval[3];

 	lsw = (pval[4] << 24) |
           (pval[5] << 16);
    
 	w740_WriteCam1(which,0,lsw,msw);
 	
 
}

void ResetMAC(struct eth_drv_sc * sc)
{
	struct w740_priv * priv=(struct w740_priv *)sc->driver_private;
	int    which=priv->which ;
	unsigned long val=w740_ReadReg(MCMDR,which);
	
	diag_printf("ResetMAC\n");
	
	w740_WriteReg(FIFOTHD,0x10000,which); //0x10100
	w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)&~(MCMDR_TXON|MCMDR_RXON),which);
	w740_WriteReg(FIFOTHD,0x100300,which); //0x10100

   	init_rxtx_rings(sc);

   	priv->cur_tx_entry=0;
    priv->cur_rx_entry=0;
	priv->rx_ptr=priv->start_rx_ptr ;
	priv->tx_ptr=priv->start_tx_ptr ;
  	
  		//11-21
  
	priv->tx_finish_ptr=priv->tx_ptr;  	
	
	w740_WriteReg(RXDLSA,priv->start_rx_ptr,which);
	w740_WriteReg(TXDLSA,priv->start_tx_ptr,which);
	w740_WriteReg(DMARFC,PACKET_SIZE,which);
  	
	w740_WriteCam(priv->which, 0, (unsigned char *)priv->mac_address);
	
	w740_WriteReg(CAMEN,w740_ReadReg(CAMEN,priv->which) | 1,priv->which);
    
  	w740_WriteReg(CAMCMR,CAMCMR_ECMP|CAMCMR_ABP|CAMCMR_AMP,which);

    /* Configure the MAC control registers. */
    w740_WriteReg(MIEN,gMIEN,which);    	
	//w740_WriteReg(MCMDR,priv->mcmdr,priv->which);
	if(which==0)
	{
   		Enable_Int(EMCTXINT0);
   		Enable_Int(EMCRXINT0);
   	}
   	else if(which==1)
   	{
   		Enable_Int(EMCTXINT1);
   		Enable_Int(EMCRXINT1);
   	}
    	/* set MAC0 as LAN port */
    	//MCMDR_0 |= MCMDR_LAN ;
	
    {
    	
    	w740_WriteReg(MCMDR,MCMDR_TXON|MCMDR_RXON|val,which);
    	w740_WriteReg(TSDR ,0,which);
    	w740_WriteReg(RSDR ,0,which);
    }
  
    w740_WriteReg(MISTA,w740_ReadReg(MISTA,which),which); //clear interrupt
}

void ResetP(int num)
{
	MiiStationWrite(num,PHY_CNTL_REG,0x0100,RESET_PHY);
}
int  ResetPhyChip(int num)
{
#if HavePHY
 	U32 RdValue;
 	int which=num;
 	volatile int loop=1000*100;

 	//MiiStationWrite(which, PHY_ANA_REG, PHYAD, DR10_TX_HALF|IEEE_802_3_CSMA_CD);

 	if(MiiStationWrite(which, PHY_CNTL_REG, PHYAD, ENABLE_AN | RESTART_AN)==1)
 	{
 		
 		return 1;
 	}	

	diag_printf("\nWait auto-negotiation...");
 	while (1) 	/* wait for auto-negotiation complete */
   	{
    	
    	RdValue = MiiStationRead(which, PHY_STATUS_REG, PHYAD) ;
    	if(RdValue==(unsigned long)1)
    	{
    		 diag_printf("ResetPhyChip failed 1\n");
    		 return 1;
    	}	  
    	if ((RdValue & AN_COMPLETE) != 0)
    	{
      		break;
      	}
      	loop--;
      	if(loop==0)
      	{
      		 return 1;
      	}	 
   	}
	diag_printf("OK\n");

 	/* read the result of auto-negotiation */
 	RdValue = MiiStationRead(which, PHY_CNTL_REG, PHYAD) ;
 	if(RdValue==(unsigned long)1)
 		return 1;
 	if ((RdValue & DR_100MB)!=0) 	/* 100MB */
   	{
    	diag_printf("100MB - ");
		ifStats.speed = 100000000;
      	w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|MCMDR_OPMOD,which);
   	}
  	else 
   	{
      	w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)&~MCMDR_OPMOD,which);
      	ifStats.speed = 10000000;
      	diag_printf("10MB - ");	
   	}
 	if ((RdValue & PHY_FULLDUPLEX) != 0) 	/* Full Duplex */
   	{
    	diag_printf("Full Duplex\n");
    	w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|MCMDR_FDUP,which);	
   	}
  	else 
   	{ 
    	diag_printf("Half Duplex\n");
 
    	w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)&~MCMDR_FDUP,which);
   	}
   	return 0;
#endif
#ifdef IC_PLUS
{
    unsigned long RdValue,i;
 // if (!skip_reset)   
 
	static int reset_phy=0;
MiiStationWrite(num, PHY_ANA_REG, PHYAD, DR100_TX_FULL|DR100_TX_HALF|\
		      DR10_TX_FULL|DR10_TX_HALF|IEEE_802_3_CSMA_CD);
		      
MiiStationWrite(num, PHY_CNTL_REG, PHYAD, ENABLE_AN | RESET_PHY|RESTART_AN);
    
    
    //cbhuang num
    MiiStationWrite(num, 0x16, PHYAD, 0x8420);
    RdValue = MiiStationRead(num, 0x12, PHYAD);	  
    
    MiiStationWrite(num, 0x12, PHYAD, RdValue | 0x80); // enable MII registers


      
  if(num == 1) {    
    for(i=0;i<3;i++)
	   {
		 RdValue = MiiStationRead(num, PHY_STATUS_REG, PHYAD) ;
		 if ((RdValue & AN_COMPLETE) != 0)
		  {
		 	diag_printf("come  cbhuang %s   %s  %d  \n",__FILE__,__FUNCTION__,__LINE__);
			 break;
		  }	 
		}
    if(i==3)
      {
		diag_printf("come  cbhuang %s   %s  %d  \n",__FILE__,__FUNCTION__,__LINE__);
		w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,1)| MCMDR_OPMOD,1);
		w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,1)| MCMDR_FDUP,1);		
		return 0;
	  }
     } 
	  
	  {
	    w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,num)|MCMDR_OPMOD,num);	
		w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,num)|MCMDR_FDUP,num);		  
	  }
	  return 0;
 	
 }
#endif
}

static void installInterrupts(struct eth_drv_sc *sc, unsigned char *enaddr)
{
	static bool firstTime=true;
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;
	U32 which = priv->which; 

//	debug1_printf("w90n740_ether: installInterrupts()\n");
 
//	if (!firstTime)
//		return;
//	firstTime = false;
	
  	//w740_WriteReg(MCMDR,0,which);
  	w740_WriteReg(FIFOTHD,0x10000,which); //0x10100
  	
  	while(w740_ReadReg(FIFOTHD,priv->which)&0x10000);
  	
  	w740_WriteReg(FIFOTHD,0x000300,which); //0x10100
  	
  	init_rxtx_rings(sc);

	priv->cur_tx_entry=0;
    priv->cur_rx_entry=0;
	
	priv->rx_ptr=priv->start_rx_ptr ;
  	priv->tx_ptr=priv->start_tx_ptr ;
  	
  	w740_WriteReg(RXDLSA,priv->start_rx_ptr,which);
  	w740_WriteReg(TXDLSA,priv->start_tx_ptr,which);
  	w740_WriteReg(DMARFC,2000,which);
  	
	EthInit(sc,enaddr);
  	//w740_WriteCam(priv->which,0,dev->dev_addr);
	w740_WriteReg(CAMEN,w740_ReadReg(CAMEN,priv->which) | 1,priv->which);
  
  	w740_WriteReg(CAMCMR,CAMCMR_ECMP|CAMCMR_ABP|CAMCMR_AMP,which);
  	//w740_WriteReg(CAMCMR,CAMCMR_ECMP|CAMCMR_ABP|CAMCMR_AMP|CAMCMR_AUP,which);	
  	
  	w740_WriteReg(MCMDR,1<<19,which);
    ResetP(which);
    if(ResetPhyChip(which)==1)
    {	
    	diag_printf("ResetPhyChip Failed\n");
    	return;
    }	
  	
    //number interrupt  number     
    diag_printf("Interface %d\n", which);
    
    /* Configure the MAC control registers. */
    w740_WriteReg(MIEN,gMIEN,which);
    w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|gMCMDR,which);
    w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|MCMDR_RXON,which);
   	
    priv->mcmdr=w740_ReadReg(MCMDR,which);
    priv->bInit=1; 
    priv->rx_packets=0;
	priv->rx_bytes=0;
	//priv->start_time=jiffies;
	
   if(which==0){
   	/* Tx interrupt vector setup. */
    	AIC_SCR_EMCTX0 = 0x41;
    	/* Rx interrupt vector setup. */
    	AIC_SCR_EMCRX0 = 0x41;
    /* Enable MAC Tx and Rx interrupt. */
    //	Enable_Int(EMCTXINT0);
    //	Enable_Int(EMCRXINT0);
    	/* set MAC0 as LAN port */
    	//MCMDR_0 |= MCMDR_LAN ;
    }
    else
    {
    	/* Tx interrupt vector setup. */
    	AIC_SCR_EMCTX1 = 0x41;
    	/* Rx interrupt vector setup. */
    	AIC_SCR_EMCRX1 = 0x41;
    	/* Enable MAC Tx and Rx interrupt. */
    //	Enable_Int(EMCTXINT1);
    //	Enable_Int(EMCRXINT1);
    	/* set MAC0 as LAN port */
    	//MCMDR_1 |= MCMDR_LAN ;
    }
    
  
	cyg_drv_mutex_init(&txMutex);
	cyg_drv_mutex_init(&oldRxMutex);
	cyg_drv_cond_init(&oldRxCond,&oldRxMutex);


	cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX,0,(unsigned)sc,MAC_Rx_isr,eth_drv_dsr,&macRxIntrHandle,&macRxIntrObject);
	cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX,0,0,MAC_Tx_isr,NULL,&macTxIntrHandle,&macTxIntrObject);
	
	cyg_drv_interrupt_attach(macRxIntrHandle);
	cyg_drv_interrupt_attach(macTxIntrHandle);
	

	w740_WriteReg(RSDR ,0,which);
}

//======================================================================
// Driver code that interfaces to the TCP/IP stack via the common
// Ethernet interface.

 
#define eth_drv_tx_done(sc,key,retval) (sc)->funs->eth_drv->tx_done(sc,key,retval)
#define eth_drv_init(sc,enaddr)  ((sc)->funs->eth_drv->init)(sc, enaddr)
#define eth_drv_recv(sc,len)  ((sc)->funs->eth_drv->recv)(sc, len)

static unsigned char myMacAddr[6] = { CYGPKG_DEVS_ETH_ARM_W90N740_MACADDR };

static bool w90n740_eth_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
	struct w740_priv *w740_priv;
	
	sc->driver_private = (void*)((unsigned long)sc->driver_private|NON_CACHE_FLAG);
	w740_priv = (struct w740_priv *)sc->driver_private;
	w740_priv->which = USEMAC;
	
	diag_printf("MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",myMacAddr[0],myMacAddr[1],myMacAddr[2],myMacAddr[3],myMacAddr[4],myMacAddr[5]);
#if defined(CYGPKG_NET)  
	ifStats.duplex = 1;      //unknown
	ifStats.operational = 1; //unknown
	ifStats.tx_queue_len = MAX_TX_FRAME_DESCRIPTORS;
#endif
  	cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
	cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
  	
	//EthInit(sc,myMacAddr);
 
	configDone = 1;
	//ethernetRunning = 1;
	eth_drv_init(sc, myMacAddr);
	return true;
}

static void w90n740_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;
	U32 which = priv->which; 

	if (!ethernetRunning)
    {
		cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
		cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
		
		ethernetRunning = 1;
		installInterrupts(sc, enaddr);

		cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
		cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
    }
}

static void w90n740_eth_stop(struct eth_drv_sc *sc)
{
  	struct w740_priv *priv=(struct w740_priv *)sc->driver_private;
	int which=priv->which;
	
	cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
	cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
	
	ethernetRunning = 0;
	priv->bInit=0; 
	w740_WriteReg(MCMDR,0,which);
	
	cyg_drv_interrupt_detach(macRxIntrHandle);
	cyg_drv_interrupt_detach(macTxIntrHandle);
	cyg_drv_interrupt_delete(macRxIntrHandle);
	cyg_drv_interrupt_delete(macTxIntrHandle);
	//free_irq(RX_INTERRUPT+which,dev);
	//free_irq(TX_INTERRUPT+which,dev);

}
 
static int w90n740_eth_control(struct eth_drv_sc *sc, 
                          unsigned long cmd, 
                          void *data, 
                          int len)
{
  switch (cmd)
    {
#if defined(CYGPKG_NET)      
     case ETH_DRV_GET_IF_STATS_UD:
     case ETH_DRV_GET_IF_STATS:
        {
          struct ether_drv_stats *p = (struct ether_drv_stats*)data;
          *p = ifStats;
          strncpy(p->description,"description goes here",sizeof(p->description)-1);
		  p->description[sizeof(p->description)-1] = '\0';
          strncpy((char*)p->snmp_chipset,"chipset name",sizeof(p->snmp_chipset)-1);
		  p->snmp_chipset[sizeof(p->snmp_chipset)-1] = '\0';
          return 0;
        }
#endif      
     case ETH_DRV_SET_MAC_ADDRESS: {
         int act;

//debug1_printf("w90n740_eth_control len=%d\n",len);
         if (ETHER_ADDR_LEN != len)
             return -1;
//         debug1_printf("w90n740_eth_control: ETH_DRV_SET_MAC_ADDRESS.\n");
         act = ethernetRunning;
         w90n740_eth_stop(sc);
         w90n740_eth_start(sc, (unsigned char*)data, 0);
         ethernetRunning = act;
         return 0;
     }
#ifdef	ETH_DRV_GET_MAC_ADDRESS
     case ETH_DRV_GET_MAC_ADDRESS: {
         if (len < ETHER_ADDR_LEN)
             return -1;
  //       debug1_printf("w90n740_eth_control: ETH_DRV_GET_MAC_ADDRESS.\n");
         memcpy(data, priv->mac_address, ETHER_ADDR_LEN);
         return 0;
     }
#endif
     default:
      return -1;
    }
}


// In case there are multiple Tx frames waiting, we should
// return how many empty Tx spots we have.  For now we just
// return 0 or 1.
static int w90n740_eth_can_send(struct eth_drv_sc *sc)
{
#if 1
	TXBD *txbd;
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;
  
	// find the next unused spot in the queue

    txbd=( TXBD *)priv->tx_ptr;
    if((txbd->mode&TXfOwnership_DMA))
    {
    	diag_printf("send_frame failed\n");
    	return 0;
    }
#endif
	return 1;

}

int BreakTest = 0;
static void w90n740_eth_send(struct eth_drv_sc *sc,
                               struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len,
                               unsigned long key)
{
    TXBD *txbd;
    unsigned char	*dest;
	struct w740_priv *priv = (struct w740_priv *)sc->driver_private;
	U32 which = priv->which, i;

//diag_printf("w90n740_eth_send %x %d\n",__builtin_return_address(0),total_len);

	txbd=( TXBD *)priv->tx_ptr;
	
	if(total_len > 1514/*MAX_ETH_FRAME_SIZE*/ || !sg_len)
	{
		eth_drv_tx_done(sc,key,-EINVAL);
		
		return;
	}
#if 0
	if((txbd->mode&TXfOwnership_DMA))
	{
		eth_drv_tx_done(sc,key,-EIO);
		return;
	}
#endif
	dest = (unsigned char *)(txbd->buffer);
	
	for (i = 0;  i < sg_len;  i++) {
		memcpy((void *)dest, (void *)sg_list[i].buf, sg_list[i].len);
		dest += sg_list[i].len;
		//diag_printf("sg_list[i].buf = %x %x\n",sg_list[i].buf,dest,sg_list[i].len);
	}
	
    
   	txbd->SL=total_len&0xFFFF; 
	txbd->mode=(PaddingMode | CRCMode | MACTxIntEn);
   	txbd->mode|= TXfOwnership_DMA;

#if defined(CYGPKG_NET)  
	++ifStats.tx_count;
#endif

   	txbd=(TXBD *)txbd->next;
   	priv->tx_ptr=(unsigned long)txbd;
    
	{
		int val=w740_ReadReg(MCMDR,which);
   		if(!(val & MCMDR_TXON))
   		{
   	      	w740_WriteReg(MCMDR,val|MCMDR_TXON,which);
   	 	}
   		w740_WriteReg(TSDR ,0,which);

   		//	debug1_printf("enable mac tx intr\n");
   		Enable_Int(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
   		//AIC_MECR = 0x
  	}
  	// tell upper layer that we're done with this sglist  	
  	eth_drv_tx_done(sc,key,0);
}


static int w90n740_eth_rcv_count=0;
static RXBD *tcpIpRxBuffer;
extern struct eth_drv_sc w90n740_sc;

// called from DSR
static inline void eth_handle_recv_buffer(RXBD *rxbd)
{
  tcpIpRxBuffer = rxbd;
  eth_drv_recv(&w90n740_sc, tcpIpRxBuffer->SL & 0xFFFF);  // discard 32-bit CRC
}

/*
* header and data using 2 sg_list at least
*/
static void w90n740_eth_recv(struct eth_drv_sc *sc,
                        struct eth_drv_sg *sg_list,
                        int sg_len)
{
	unsigned char *source;
	struct w740_priv * priv = (struct w740_priv *)sc->driver_private;
	U32 length, i;
	U32 status;

	++w90n740_eth_rcv_count;

	if (!tcpIpRxBuffer)
    	return;  // no packet waiting, shouldn't be here!
	
    length = tcpIpRxBuffer->SL & 0xFFFF;
    
	//diag_printf("tcpIpRxBuffer=%x length=%d,sg_len=%d\n",tcpIpRxBuffer,length,sg_len);
	
    status = tcpIpRxBuffer->SL & 0x00FF0000;
	// copy data from eth buffer into scatter/gather list
	if(status & RXDS_RXGD)
		source = (unsigned char *) (tcpIpRxBuffer->buffer & ~NON_CACHE_FLAG);// + EtherFramePadding;
	else
		goto out;
	
	for (i = 0;  i < sg_len;  i++) {
		memcpy((unsigned char*)sg_list[i].buf,source,sg_list[i].len);
		source += sg_list[i].len;
	}

out:
	tcpIpRxBuffer->reserved = 0;
	priv->rx_ptr=(unsigned long)tcpIpRxBuffer->next;
	tcpIpRxBuffer->SL =RXfOwnership_DMA;
	tcpIpRxBuffer = NULL;
	return;
}

// routine called to handle ethernet controller in polled mode
static void w90n740_eth_poll(struct eth_drv_sc *sc)
{
	debug1_printf("w90n740_eth_poll %x\n",__builtin_return_address(0));
  //BDMA_Rx_isr(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX, 0); // Call ISR routine
  w90n740_eth_deliver(sc);  // handle rx frames
  //w90n740_handle_tx_complete();
}

static int w90n740_eth_int_vector(struct eth_drv_sc *sc)
{
  //return CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX;
  return CYGNUM_HAL_INTERRUPT_ETH_MAC_RX;
}

//w90n740_sc.driver_private
ETH_DRV_SC(w90n740_sc,
           &w90n740_priv_data0, // Driver specific data
           "eth0",                // Name for this interface
           w90n740_eth_start,
           w90n740_eth_stop,
           w90n740_eth_control,
           w90n740_eth_can_send,
           w90n740_eth_send,
           w90n740_eth_recv,
           w90n740_eth_deliver,
           w90n740_eth_poll,
           w90n740_eth_int_vector
           );


#pragma arm section rwdata = "netdev"
cyg_netdevtab_entry_t w90n740_netdev = {
	"w90n740",
	w90n740_eth_init,
	&w90n740_sc
};
#pragma arm section rwdata



