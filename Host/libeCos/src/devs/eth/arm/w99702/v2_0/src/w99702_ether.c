//==========================================================================
//
//      w99702_ether.c
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
#include "cyg/error/codes.h"
#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
#include "pkgconf/devs_eth_arm_w99702.h"
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

#include "include.h"

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

#include "wblib.h"
#include "std.h"
#include "w702sdio.h"

extern cyg_handle_t  gpioIntrHandle;
extern cyg_handle_t  sdioIntrHandle;


static bool w99702_eth_init(struct cyg_netdevtab_entry *tab);

// Set up the level of debug output

#define EtherFramePadding 2

#define Bit(n) (1<<(n))

// enable/disable software verification of rx CRC
// should be moved to user-controlled valud in CDL file

// miscellaneous data structures
#define	ETHER_ADDR_LEN	6
typedef  BYTE  ETH_ADDR[6];
// number of ethernet buffers should be enough to keep both rx
// and tx queues full plus some extras for in-process packets

//#pragma arm section zidata = "nozero"
__align(32)	w99702_priv_t	w702_priv;
//#pragma arm section zidata

__align(16)  wlan_private w99702_priv_data0;

extern __weak cyg_uint8 INIT_WBFAT_FS(void);
static fmi_init_handler_t fmi_do_handle;



#if defined(CYGPKG_NET)
//static cyg_drv_mutex_t txMutex;
struct ether_drv_stats ifStats;
#endif

// interrupt entry counters
U32 w99702_MAC_Rx_Cnt;
U32 w99702_MAC_Tx_Cnt;
U32 w99702_MAC_Phy_Cnt;


//static cyg_drv_mutex_t oldRxMutex;
//static cyg_drv_cond_t  oldRxCond;


static int configDone = 0;

/*----------------------------------------------------------------------
 * Init Data structures used to send/recv package
 */
static void init_rxtx_rings(struct eth_drv_sc *dev)
{
	int i;
	wlan_private * wlan_priv = (wlan_private *)dev->driver_private;
	w99702_priv_t * w702_priv = (w99702_priv_t *)wlan_priv->priv;
	
	w702_priv->tx_head = &w702_priv->tx_desc[0];	
	w702_priv->tx_tail = w702_priv->tx_head;
	
	w702_priv->rx_head = &w702_priv->rx_desc[0];
	w702_priv->rx_tail = w702_priv->rx_head;
	
	for ( i = 0 ; i < MAX_TX_FRAME_DESCRIPTORS ; i++ )
	{
	   	w702_priv->tx_desc[i].SL=0;
	   	w702_priv->tx_desc[i].mode=0;
		w702_priv->tx_desc[i].buffer=(unsigned long)&w702_priv->tx_buf[i] | NON_CACHE_FLAG;	
		w702_priv->tx_desc[i].next=(unsigned long)&w702_priv->tx_desc[i+1];
	}
	w702_priv->tx_desc[i-1].next=(unsigned long)&w702_priv->tx_desc[0];	
	
    for( i =0 ; i < MAX_RX_FRAME_DESCRIPTORS ; i++)
    {    
	    w702_priv->rx_desc[i].SL=0;
	    w702_priv->rx_desc[i].mode=RXfOwnership_DMA;
		w702_priv->rx_desc[i].buffer=(unsigned long)&w702_priv->rx_buf[i] | NON_CACHE_FLAG;	
		w702_priv->rx_desc[i].next=(unsigned long)&w702_priv->rx_desc[i+1];
	}	
	w702_priv->rx_desc[i-1].next=(unsigned long)&w702_priv->rx_desc[0];	
	
	w702_priv->tmprx_desc.SL = 0;
	w702_priv->tmprx_desc.mode = RXfOwnership_DMA;
	w702_priv->tmprx_desc.buffer = (unsigned long)&w702_priv->tmprx_buf | NON_CACHE_FLAG;
	w702_priv->tmprx_desc.next = 0;
	
	w702_priv->tx_full = 0;
    	
}

//======================================================================
static int EthInit(struct eth_drv_sc *sc, U8* mac_address)
{
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t *priv = wlan_priv->priv;

	if(mac_address)
    	diag_printf("EthInit(%02x:%02x:%02x:%02x:%02x:%02x)\n",
                mac_address[0],mac_address[1],mac_address[2],
                mac_address[3],mac_address[4],mac_address[5]);
 	else
    	diag_printf("EthInit(NULL)\n");

  // set up our MAC address

	if(mac_address)
    {
    	memcpy(priv->mac_address, mac_address, ETHER_ADDR_LEN);
    	
  		//w740_WriteCam(priv->which,0,mac_address);
  		wlan_set_mac_address(sc, mac_address);
    }
  
	return 0;
}

static void w99702_handle_tx_complete(void)
{

	// record status and then free any buffers we're done with
	extern struct eth_drv_sc w99702_sc;
    wlan_private * wlan_priv = (wlan_private *)w99702_sc.driver_private;
	w99702_priv_t * priv = (w99702_priv_t *)wlan_priv->priv;
    U32 status;  
    volatile TXBD  *txbd;
    static  unsigned  reset_rx_loop=0;
    
    //status=w740_ReadReg(MISTA,which);   //get  interrupt status;
    //w740_WriteReg(MISTA,status&0xFFFF0000,which);  //clear interrupt
   // w740_WriteReg(MISTA,status,which);  //clear interrupt
	
	++ifStats.interrupts;

#if 0
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
    	diag_printf("w99702 MAC In Tx %d RX   I Have Not Any Descript Needed\n",priv->which);
        ResetMAC(&w99702_sc);
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
  w99702_handle_tx_complete();
  cyg_drv_interrupt_acknowledge(vector);  
  return CYG_ISR_HANDLED;
}

//======================================================================
//cyg_drv_interrupt_acknowledge(vector);
static cyg_uint32 MAC_Rx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
#if 0
	struct eth_drv_sc *sc = (struct eth_drv_sc *)data;
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * w702_priv = (w99702_priv_t *)wlan_priv->priv;
	U32 status;
	RXBD *rxbd;
	
	rxbd=(RXBD *)priv->rx_tail ;
	
	status=w740_ReadReg(MISTA,which);   //get  interrupt status;

    w740_WriteReg(MISTA,status,which); //clear interrupt
  	
  	cyg_drv_interrupt_acknowledge(vector);

	++w99702_MAC_Rx_Cnt;

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
#endif
	//eth_drv_dsr(vector, 0, data);
	//return CYG_ISR_HANDLED;
}

static void eth_handle_recv_buffer(wlan_private * wlan_priv);

int ethernetRunning;
static cyg_handle_t  macRxIntrHandle;
static cyg_handle_t  macTxIntrHandle;
static cyg_interrupt  macRxIntrObject;
static cyg_interrupt  macTxIntrObject;

static void w99702_eth_deliver(struct eth_drv_sc *sc)
{
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * priv = (w99702_priv_t *)wlan_priv->priv;
	RXBD *rxbd;
	U32 volatile length;
	U32 volatile status;
	U32 volatile SL;
	//int cnt = 0;
#if 0	
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
#else
	rxbd=(RXBD *)priv->rx_head;
	//diag_printf("w99702_eth_deliver %d, %x\n", ethernetRunning, rxbd->mode);
	if(!rxbd->mode)//data valid
		eth_handle_recv_buffer(wlan_priv);//(rxbd);
#endif
	return;
}


static void installInterrupts(struct eth_drv_sc *sc, unsigned char *enaddr)
{
	static bool firstTime=true;
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t *priv = wlan_priv->priv;

//	debug1_printf("w99702_ether: installInterrupts()\n");
 
//	if (!firstTime)
//		return;
//	firstTime = false;
	
  	//w740_WriteReg(MCMDR,0,which);
//  	w740_WriteReg(FIFOTHD,0x10000,which); //0x10100
  	
 // 	while(w740_ReadReg(FIFOTHD,priv->which)&0x10000);
  	
 // 	w740_WriteReg(FIFOTHD,0x000300,which); //0x10100
  	
 // 	init_rxtx_rings(sc);

  	
  	//w740_WriteReg(RXDLSA,priv->start_rx_ptr,which);
  	//w740_WriteReg(TXDLSA,priv->start_tx_ptr,which);
  	//w740_WriteReg(DMARFC,2000,which);
  	
	//EthInit(sc,enaddr);
  	//w740_WriteCam(priv->which,0,dev->dev_addr);
	//w740_WriteReg(CAMEN,w740_ReadReg(CAMEN,priv->which) | 1,priv->which);
  
  	//w740_WriteReg(CAMCMR,CAMCMR_ECMP|CAMCMR_ABP|CAMCMR_AMP,which);
  	//w740_WriteReg(CAMCMR,CAMCMR_ECMP|CAMCMR_ABP|CAMCMR_AMP|CAMCMR_AUP,which);	
  	
  	//w740_WriteReg(MCMDR,1<<19,which);
    //ResetP(which);
   /* if(ResetPhyChip(which)==1)
    {	
    	diag_printf("ResetPhyChip Failed\n");
    	return;
    }*/	
  	
    //number interrupt  number     
//    diag_printf("Interface %d\n", which);
    
    /* Configure the MAC control registers. */
    //w740_WriteReg(MIEN,gMIEN,which);
    //w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|gMCMDR,which);
    //w740_WriteReg(MCMDR,w740_ReadReg(MCMDR,which)|MCMDR_RXON,which);
   	
    //priv->mcmdr=w740_ReadReg(MCMDR,which);
    priv->bInit=1; 
   
	//priv->start_time=jiffies;
	
   
  
	//cyg_drv_mutex_init(&txMutex);
	//cyg_drv_mutex_init(&oldRxMutex);
	//cyg_drv_cond_init(&oldRxCond,&oldRxMutex);


	//cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX,0,(unsigned)sc,MAC_Rx_isr,eth_drv_dsr,&macRxIntrHandle,&macRxIntrObject);
	//cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX,0,0,MAC_Tx_isr,NULL,&macTxIntrHandle,&macTxIntrObject);
	
	//cyg_drv_interrupt_attach(macRxIntrHandle);
	//cyg_drv_interrupt_attach(macTxIntrHandle);
	

	//w740_WriteReg(RSDR ,0,which);
}

//======================================================================
// Driver code that interfaces to the TCP/IP stack via the common
// Ethernet interface.

 
#define eth_drv_tx_done(sc,key,retval) (sc)->funs->eth_drv->tx_done(sc,key,retval)
#define eth_drv_init(sc,enaddr)  ((sc)->funs->eth_drv->init)(sc, enaddr)
#define eth_drv_recv(sc,len)  ((sc)->funs->eth_drv->recv)(sc, len)

static unsigned char myMacAddr[6] = { CYGPKG_DEVS_ETH_ARM_W99702_MACADDR };

extern int coremodule_init(cyg_uint8);
extern cyg_int32 wlan_fmi_init();

extern sdio_ctrller *sdio_host;

sdio_ctrller fmi_sdio_ctr, *fmi_ctrller;
wlan_private fmi_wlan_priv, *fmi_priv;
int initSDIO(cyg_uint8 io_func)
{
	wlan_adapter *Adapter = fmi_priv->adapter;

	fmi_ctrller = &fmi_sdio_ctr;
	memset(fmi_ctrller, 0, sizeof(sdio_ctrller));
	
	fmi_priv = &fmi_wlan_priv;
	if (!((int)fmi_priv->adapter = malloc(sizeof(wlan_adapter)))) {
		printf("Allocate Adapter memory Fail!!\n");
		return -1;
	}
	memset(fmi_priv->adapter, 0, sizeof(wlan_adapter));

	fmi_ctrller->card_capability.num_of_io_funcs = io_func;

	fmi_priv->wlan_dev.card = fmi_ctrller->card;
	fmi_ctrller->card->ctrlr = fmi_ctrller;
	
	fmi_priv = (wlan_private *)wlan_add_card(fmi_ctrller->card);
}

static char thread_stack[8192];
static cyg_handle_t thread_handle;
static cyg_thread thread_data;
static void wlan_init_thread(cyg_addrword_t data)
{
	struct cyg_netdevtab_entry *tab = (struct cyg_netdevtab_entry *)data;
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
        wlan_private *wlan_priv;
        wlan_adapter *Adapter;	
	cyg_int32  io_func;
	
	fmi_do_handle = INIT_WBFAT_FS;
	
	if(fmi_do_handle != NULL)
		io_func = (*fmi_do_handle)();
	else
		io_func = wlan_fmi_init();
		
	if (io_func < 0)
	{/* Failed */
		configDone = 2;
		return;
	}
	
	coremodule_init(io_func);
	if(wlan_init_module(io_func) == WLAN_STATUS_FAILURE)
	{
		configDone = 2;
		return;
	}
	wlan_priv = (wlan_private *)sc->driver_private;
	Adapter = wlan_priv->adapter;	
	//EthInit(sc,myMacAddr);
	configDone = 1;
	//ethernetRunning = 1;
	eth_drv_init(sc, Adapter->CurrentAddr);
	
	diag_printf("Adapter->CurrentAddr: %2x:%2x:%2x:%2x:%2x:%2x\n",
           Adapter->CurrentAddr[0], Adapter->CurrentAddr[1],
           Adapter->CurrentAddr[2], Adapter->CurrentAddr[3],
           Adapter->CurrentAddr[4], Adapter->CurrentAddr[5]);	

}


static bool w99702_eth_init(struct cyg_netdevtab_entry *tab)
{
	struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
	wlan_private *wlan_priv;
	cyg_uint8  io_func;
	
	sc->driver_private = (void*)((unsigned long)sc->driver_private|NON_CACHE_FLAG);
	wlan_priv = (wlan_private *)sc->driver_private;
	wlan_priv->priv = &w702_priv;
	
	init_rxtx_rings(sc);

	cyg_flag_init(&w702_priv.send_flag);
	
	cyg_thread_create(1,          // Priority
                      wlan_init_thread,                   // entry
                      (cyg_addrword_t)tab,                    // entry parameter
                      "pxa270m card init",                // Name
                      &thread_stack[0],         // Stack
                      sizeof(thread_stack),                            // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);    // Start it
	
#if 0
	io_func = wlan_fmi_init();
	coremodule_init(io_func);
	wlan_init_module(io_func);
	//initSDIO(io_func);

  	//cyg_drv_interrupt_mask(WAKEUP_GPIO_IRQ);
//	cyg_drv_interrupt_mask(IRQ_FMI);
  	
	EthInit(sc,myMacAddr);

 
	configDone = 1;
	//ethernetRunning = 1;
	eth_drv_init(sc, myMacAddr);
#endif
	if(configDone == 1)
		return true;
	else
		return false;
}

static void w99702_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * w702_priv = (w99702_priv_t *)wlan_priv->priv;
	cyg_uint8  io_func;

	if (!ethernetRunning)
    {
		//cyg_drv_interrupt_mask(WAKEUP_GPIO_IRQ);
//		cyg_drv_interrupt_mask(IRQ_FMI);
		
#if 0
		io_func = wlan_fmi_init();
		coremodule_init(io_func);
		wlan_init_module(io_func);

		EthInit(sc,myMacAddr);
#endif
		ethernetRunning = 1;
		
		wlan_open(sc);
		w702_priv->bInit=1;

		// yachen	
		wlan_priv->adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_PROMISCUOUS_ENABLE;
		wlan_priv->adapter->CurrentPacketFilter |= HostCmd_ACT_MAC_ALL_MULTICAST_ENABLE;
		wlan_priv->adapter->CurrentPacketFilter &= ~HostCmd_ACT_MAC_MULTICAST_ENABLE;
		SetMacPacketFilter(wlan_priv); 		
		
		//installInterrupts(sc, enaddr);

		//cyg_drv_interrupt_unmask(WAKEUP_GPIO_IRQ);
//		cyg_drv_interrupt_unmask(IRQ_FMI);
    }
}

static void w99702_eth_stop(struct eth_drv_sc *sc)
{
  	wlan_private *wlan_priv=(wlan_private *)sc->driver_private;
  	w99702_priv_t *priv = wlan_priv->priv;
	
	//cyg_drv_interrupt_mask(WAKEUP_GPIO_IRQ);
//	cyg_drv_interrupt_mask(IRQ_FMI);
	
	ethernetRunning = 0;
	priv->bInit=0; 
	//w740_WriteReg(MCMDR,0,which);
	wlan_close(sc);
	
//	cyg_drv_interrupt_detach(gpioIntrHandle);
//	cyg_drv_interrupt_detach(sdioIntrHandle);
	
	//free_irq(RX_INTERRUPT+which,dev);
	//free_irq(TX_INTERRUPT+which,dev);

}
 
static int w99702_eth_control(struct eth_drv_sc *sc, 
                          unsigned long cmd, 
                          void *data, 
                          int len)
{
	//wlan_handler_def
	//wlan_do_ioctl(sc, (struct ifreq *)data,cmd);

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

//debug1_printf("w99702_eth_control len=%d\n",len);
         if (ETHER_ADDR_LEN != len)
             return -1;
//         debug1_printf("w99702_eth_control: ETH_DRV_SET_MAC_ADDRESS.\n");
         act = ethernetRunning;
         //w99702_eth_stop(sc);
         //w99702_eth_start(sc, (unsigned char*)data, 0);
         EthInit(sc,data);
         ethernetRunning = act;
         return 0;
     }
#ifdef	ETH_DRV_GET_MAC_ADDRESS
     case ETH_DRV_GET_MAC_ADDRESS: {
         if (len < ETHER_ADDR_LEN)
             return -1;
         diag_printf("w99702_eth_control: ETH_DRV_GET_MAC_ADDRESS.\n");
         memcpy(data, priv->mac_address, ETHER_ADDR_LEN);
         return 0;
     }
#endif
     default:
     		return wlan_do_ioctl(sc, (struct ifreq *)data,cmd);
      //return -1;
    }
}


// In case there are multiple Tx frames waiting, we should
// return how many empty Tx spots we have.  For now we just
// return 0 or 1.
static int w99702_eth_can_send(struct eth_drv_sc *sc)
{
	TXBD *txbd;
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * priv = (w99702_priv_t *)wlan_priv->priv;
  
	// find the next unused spot in the queue
	if (0 && !ethernetRunning)//xh disable it, wpa need send packet event through eth is not configured OK.
	{
		diag_printf("ethernet not Running\n");
    	return 0;
	}

	if (!priv->bInit)
	{
		diag_printf("ethernet not init\n");
    	return 0;
	}
	if(wlan_priv->adapter->IsDeepSleep)
	{
		diag_printf("SDIO in Deep sleep\n");
    	return 0;
	}
	
    txbd=( TXBD *)priv->tx_tail;

#if 1
//    while((txbd->mode & TXfOwnership_DMA) || !ethernetRunning)
    while((txbd->mode & TXfOwnership_DMA))
    {
    	priv->tx_full = 1;
    	//diag_printf("tx full\n");
    	cyg_flag_maskbits(&priv->send_flag, 0x0);
    	cyg_flag_wait(
            &priv->send_flag,
            -1,
            CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR );
    	
    	//priv->tx_full = 0;
    	//return 1;
    	//return 0;
    }
#else	
    if((txbd->mode&TXfOwnership_DMA))
    {
    	priv->tx_full = 1;
    	diag_printf("tx full\n");
    	return 0;
    }
#endif

    priv->tx_full = 0;
	return 1;

}

int BreakTest = 0;
static void w99702_eth_send(struct eth_drv_sc *sc,
                               struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len,
                               unsigned long key)
{

	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;

	//diag_printf("begin send\n");
	wlan_tx_packet(wlan_priv, sg_list, sg_len,  total_len);


#if defined(CYGPKG_NET)  
	++ifStats.tx_count;
#endif

  	// tell upper layer that we're done with this sglist  	
  	eth_drv_tx_done(sc,key,0);
}


static int w99702_eth_rcv_count=0;
static RXBD *tcpIpRxBuffer;
extern struct eth_drv_sc w99702_sc;

// called from DSR
int udpcount = 0;
static inline void eth_handle_recv_buffer(wlan_private *wlan_priv)//w99702_priv_t * priv)//(RXBD *rxbd)
{
	w99702_priv_t * priv = (w99702_priv_t *)wlan_priv->priv;
	RXBD *rxbd;
	int count = 0;
	rxbd=(RXBD *)priv->rx_head;
	
	if(priv->rx_head == priv->rx_tail)
	{
		diag_printf("receive buffer full\n");
	}
	//diag_printf("reve buffer %x, %x\n", priv->rx_head, priv->rx_tail);
	do
	{
		tcpIpRxBuffer = rxbd;
		if(tcpIpRxBuffer->SL == 94)
			udpcount++;
		if(ethernetRunning)
			eth_drv_recv(&w99702_sc, tcpIpRxBuffer->SL & 0xFFFF);  // discard 32-bit CRC
		else
		{
			priv->rx_head= (RXBD*)tcpIpRxBuffer->next;
			tcpIpRxBuffer->SL = 0;
			tcpIpRxBuffer->mode =RXfOwnership_DMA;
			tcpIpRxBuffer = NULL;
		}
			
		rxbd=(RXBD *)priv->rx_head;
		count++;
	}while((priv->rx_head != priv->rx_tail) && (!rxbd->mode));
	
	
	//diag_printf("count = %d, udpcount = %d\n", count, udpcount);
	if(rxbd->mode && (priv->rx_head != priv->rx_tail))
	{
		diag_printf("*****head mode = %x, but head!=tail******\n", rxbd->mode);
		priv->rx_head = priv->rx_tail;
	}
}

/*
* header and data using 2 sg_list at least
*/
static void w99702_eth_recv(struct eth_drv_sc *sc,
                        struct eth_drv_sg *sg_list,
                        int sg_len)
{
	unsigned char *source;
	wlan_private * wlan_priv = (wlan_private *)sc->driver_private;
	w99702_priv_t * priv = (w99702_priv_t *)wlan_priv->priv;
	U32 length, i;
	U32 status;
	int dropLen;
	
	
	
	++w99702_eth_rcv_count;

	if (!tcpIpRxBuffer)
    	return;  // no packet waiting, shouldn't be here!
	
    length = tcpIpRxBuffer->SL & 0xFFFF;
    
	//diag_printf("tcpIpRxBuffer=%x length=%d,sg_len=%d\n",tcpIpRxBuffer,length,sg_len);
	
	// copy data from eth buffer into scatter/gather list
	
	//source = (unsigned char *) (tcpIpRxBuffer->buffer & ~NON_CACHE_FLAG);// + EtherFramePadding;
	source = (unsigned char *)tcpIpRxBuffer->buffer;
	source += SDIO_HEADER_LEN;
	length -= SDIO_HEADER_LEN;
	
	ProcessRxedPacket(wlan_priv, (char*)source, (int *)&length);
	dropLen = (tcpIpRxBuffer->SL&0xFFFF) - length - SDIO_HEADER_LEN;
	source += dropLen;
	
	for (i = 0;  i < sg_len;  i++) {
		if(sg_list[i].buf)
		{
			memcpy((unsigned char*)sg_list[i].buf,source,sg_list[i].len);
			source += sg_list[i].len;
		}
	}

out:
	priv->rx_head= (RXBD*)tcpIpRxBuffer->next;
	tcpIpRxBuffer->SL = 0;
	tcpIpRxBuffer->mode =RXfOwnership_DMA;
	tcpIpRxBuffer = NULL;
	return;
}

// routine called to handle ethernet controller in polled mode
static void w99702_eth_poll(struct eth_drv_sc *sc)
{
//	debug1_printf("w99702_eth_poll %x\n",__builtin_return_address(0));
  //BDMA_Rx_isr(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX, 0); // Call ISR routine
  w99702_eth_deliver(sc);  // handle rx frames
  //w99702_handle_tx_complete();
}

static int w99702_eth_int_vector(struct eth_drv_sc *sc)
{
  //return CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX;
  return IRQ_FMI;
}

void force_net_dev_linked(void)
{
	diag_printf("link wlan driver\n");
}

/* Return:
	0, Configuring
	1, OK
	2, Failed
 */
int IsWirelessConfigDone()
{
	return configDone;
}

//w99702_sc.driver_private
ETH_DRV_SC(w99702_sc,
           &w99702_priv_data0, // Driver specific data
           "wlan0",//"wlan0",                // Name for this interface
           w99702_eth_start,
           w99702_eth_stop,
           w99702_eth_control,
           w99702_eth_can_send,
           w99702_eth_send,
           w99702_eth_recv,
           w99702_eth_deliver,
           w99702_eth_poll,
           w99702_eth_int_vector,
           &wlan_handler_def,
           wlan_get_wireless_stats
           );


#pragma arm section rwdata = "netdev"
cyg_netdevtab_entry_t w99702_netdev = {
	"w99702",
	w99702_eth_init,
	&w99702_sc
};
#pragma arm section rwdata



