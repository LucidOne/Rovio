
#ifndef _W702MAC_H_
#define _W702MAC_H_

#include "include.h"
#include "plf_io.h"
#include "w99702_reg.h"
#include "wblib.h"

#define NON_CACHE_FLAG		0x10000000
	
#define  _SYS_BASE_MAC  		0xFFF03000
#define  MAC_OFFSET  	0x0
#define  MAC_0_OFFSET  	MAC_OFFSET
#define  MAC_1_OFFSEST  (0x800+MAC_OFFSET)


//Descriptor
typedef struct 
{
	volatile unsigned long	SL;
	volatile unsigned long	buffer;
	volatile unsigned long	mode;
	volatile unsigned long	next;
}RXBD;


typedef struct 
{
	volatile unsigned long mode;
	volatile unsigned long buffer;
	volatile unsigned long SL;
	volatile unsigned long next;
}TXBD;


#if defined(CYGPKG_REDBOOT)
#define MAX_RX_FRAME_DESCRIPTORS        4     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	4     // Max number of Tx Frame Descriptors
#else
#define MAX_RX_FRAME_DESCRIPTORS        32     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	32     // Max number of Tx Frame Descriptors
#endif

#define 	PACKET_SIZE		1560
#define 	ETHER_ADDR_LEN	6
#define 	ETH_HLEN		14
//#define 	IWEVCUSTOM		1
#define HZ	100

#ifndef outpw
#define outpw(port,value)     (*((cyg_uint32 volatile *) (port))=value)
#endif
#ifndef inpw
#define inpw(port)            (*((cyg_uint32 volatile *) (port)))
#endif

extern cyg_uint32 volatile g_IMASK;

typedef cyg_uint8 (*fmi_init_handler_t)(void);

void register_fmi_init(fmi_init_handler_t handle);

#if 1
#define SAVE_IRQ(irq)	irq = inpw(AIC_IMR); \
						cyg_interrupt_mask(IRQ_FMI);
#define RESTORE_IRQ(irq) outpw(AIC_MECR, irq)
#else
#define SAVE_IRQ(irq)	irq = inpw(REG_SDIER);\
						outpw(REG_SDIER, inpw(REG_SDIER) & ~0x10);
#define RESTORE_IRQ(irq) outpw(REG_SDIER, irq)

#endif

#define OS_INT_DISABLE(save_irq)	SAVE_IRQ(save_irq)
#define OS_INT_RESTORE(save_irq)	RESTORE_IRQ(save_irq)
#define	TX_DISABLE
#define TX_RESTORE


 //      char mac_address[ETH_ALEN];
// don't have any private data, but if we did, this is where it would go
typedef struct  w702_priv
{
	TXBD tx_desc[MAX_TX_FRAME_DESCRIPTORS];//__align(16) 
	RXBD rx_desc[MAX_RX_FRAME_DESCRIPTORS];//__align(16) 
	char tx_buf[MAX_TX_FRAME_DESCRIPTORS][WLAN_UPLD_SIZE];//__align(16) 
	char rx_buf[MAX_RX_FRAME_DESCRIPTORS][WLAN_UPLD_SIZE];//__align(16) //MAX_ETH_FRAME_SIZE
	
	RXBD tmprx_desc;
	char tmprx_buf[WLAN_UPLD_SIZE];

	cyg_uint32 bInit;	
	TXBD *tx_head, *tx_tail;
	RXBD *rx_head, *rx_tail;
	char 		  mac_address[ETHER_ADDR_LEN];
	int tx_full;
	cyg_flag_t	send_flag;
}w99702_priv_t;


#define RXfOwnership_DMA 0x80000000  // 10 = DMA

// TX Frame Descriptor's Owner bit
#define TXfOwnership_DMA 0x80000000  // 1 = DMA

#endif 
