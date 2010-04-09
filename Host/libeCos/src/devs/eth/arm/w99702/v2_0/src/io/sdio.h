/*File sdio.h
 * This file contains the structure definations for the low level driver
 * And the error response related code
 *
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */

#ifndef __SDIO_H__
#define __SDIO_H__

//#include	<linux/spinlock.h>      /* For read write semaphores */
//#include	<asm/semaphore.h>
//#include	<linux/completion.h>
//#include	<asm/dma.h>

//#include	<asm/arch/pxa-regs.h>
//#include	"include.h"
#include "pkgconf/system.h"
#include "cyg/error/codes.h"
#include "sys/types.h"
#include "cyg/infra/cyg_type.h"  // Common type definitions and support
                                 // including endian-ness
#include	"kapi.h"
#include 	"sdio_spec.h"
#include 	"sdio_error.h"
#include 	"sdio_defs.h"

#include 	"os_defs.h"

//clyu 
/* GPIO number used for WAKEUP */
#define WAKEUP_GPIO		22

/* Map it to the Bulverde IRQ Table */
#define	WAKEUP_GPIO_IRQ		WAKEUP_GPIO
#define IRQ_MMC				12//MultiMediaCard
#define MAX_UDELAY_MS 2
#define mdelay(a)              udelay((a) * 1000)
#if 0
#define udelay(n)							\
	  ((n) > (MAX_UDELAY_MS * 1000) ? __bad_udelay() :		\
			__const_udelay((n) * ((2199023U*HZ)>>11)))
#endif
//end


#ifdef	DEBUG_SDIO_LEVEL2
#ifndef DEBUG_LEVEL1
#define	DEBUG_LEVEL1
#endif
#define	_ENTER() diag_printf("Enter: %s, %s linux %i\n", __FUNCTION__, \
							__FILE__, __LINE__)
#define	_LEAVE() diag_printf("Leave: %s, %s linux %i\n", __FUNCTION__, \
							__FILE__, __LINE__)
#else
#define _ENTER()
#define _LEAVE()
#endif

#ifdef	DEBUG_SDIO_LEVEL1
#define	_DBGMSG(x...)		diag_printf(x)
#define	_WARNING(x...)		diag_printf(x)
#else
#define	_DBGMSG(x...)
#define	_WARNING(x...)
#endif

#ifdef DEBUG_SDIO_LEVEL0
#define	_PRINTK(x...)		diag_printf(x)
#define	_ERROR(x...)		diag_printf(x)
#else
#define	_PRINTK(x...)
#define	_ERROR(x...)		diag_printf(x)
#endif

#define MAJOR_NUMBER	242

#ifdef DEBUG_USEC
static inline unsigned long
get_usecond(void)
{
    struct timeval t;
    unsigned long usec;

    do_gettimeofday(&t);
    usec = (unsigned long) t.tv_sec * 1000000 + ((unsigned long) t.tv_usec);
    return usec;
}
#endif

typedef struct _card_capability
{
    cyg_uint8 num_of_io_funcs;         /* Number of i/o functions */
    cyg_uint8 memory_yes;              /* Memory present ? */
    cyg_uint16 rca;                    /* Relative Card Address */
    cyg_uint32 ocr;                    /* Operation Condition register */
    cyg_uint16 fnblksz[8];
    cyg_uint32 cisptr[8];
} card_capability;

typedef struct _dummy_tmpl
{
    int irq_line;
} dummy_tmpl;

typedef struct _mmc_card_rec
{
    cyg_uint8 chiprev;
    cyg_uint8 block_size_512;
    cyg_uint8 async_int_mode;
    card_capability info;
    struct _sdio_host *ctrlr;
} mmc_card_rec;

typedef struct _mmc_card_rec *mmc_card_t;

typedef struct _sdio_host *mmc_controller_t;

typedef enum _sdio_fsm
{
    SDIO_FSM_IDLE = 1,
    SDIO_FSM_CLK_OFF,
    SDIO_FSM_END_CMD,
    SDIO_FSM_BUFFER_IN_TRANSIT,
    SDIO_FSM_END_BUFFER,
    SDIO_FSM_END_IO,
    SDIO_FSM_END_PRG,
    SDIO_FSM_ERROR
} sdio_fsm_state;

typedef struct _sdio_host
{
    int usage;
    int slot;
    cyg_uint16 manf_id;
    cyg_uint16 dev_id;

    int card_int_ready;
    int busy;                   /* atomic busy flag               */
    cyg_uint32 mmc_i_reg;              /* interrupt last requested       */

    int bus_width;
    int dma_init;
    int irq_line;
   // int card_int_ready;
    cyg_uint32 num_ofcmd52;
    cyg_uint32 num_ofcmd53;

#ifdef IRQ_DEBUG
    int irqcnt;
    int timeo;
#endif
   // int busy;                   /* atomic busy flag               */
   // cyg_uint32 mmc_i_reg;              /* interrupt last requested       */
    cyg_uint32 mmc_i_mask;             /* mask to be set by intr handler */
    cyg_uint32 mmc_stat;               /* status register at last intr   */
    cyg_uint32 mmc_cmdat;              /* MMC_CMDAT at the last inr      */
    cyg_uint32 saved_mmc_clkrt;
    cyg_uint32 saved_mmc_i_mask;
    cyg_uint32 saved_mmc_resto;
    cyg_uint32 saved_mmc_spi;
    cyg_uint32 saved_drcmrrxmmc;
    cyg_uint32 saved_drcmrtxmmc;
    cyg_uint32 suspended;
    cyg_uint32 clkrt;                  /* current bus clock rate         */
    cyg_uint8 mmc_res[8];              /* Allignment                     */

    /*
     * DMA Related 
     */
    cyg_int32 blksz;              /* current block size in bytes     */
    cyg_int32 bufsz;              /* buffer size for each transfer   */
    cyg_int32 nob;                /* number of blocks pers buffer    */

    int chan;                   /* dma channel no                  */
#if 0
    cyg_addrword_t phys_addr;       /* iodata physical address         */

    pxa_dma_desc *read_desc;    /* input descriptor array         */

    pxa_dma_desc *write_desc;   /* output descriptor 
                                   array virtual address */
    cyg_addrword_t read_desc_phys_addr;     /* descriptor array 
                                           physical address      */
    cyg_addrword_t write_desc_phys_addr;    /* descriptor array 
                                           physical address      */
    pxa_dma_desc *last_read_desc;       /* last input descriptor 
                                           used by the previous transfer 
                                         */
    pxa_dma_desc *last_write_desc;      /* last output descriptor
                                           used by the previous 
                                           transfer              */
#endif

    sdio_fsm_state state;

    card_capability card_capability;
    char *iodata;               /* I/O data buffer           */
    struct _dummy_tmpl *tmpl;
    //struct completion completion;       /* completion                */
    cyg_mutex_t io_sem;
    //struct rw_semaphore rw_semaphore;
    mmc_card_t card;
} sdio_ctrller;

typedef struct _sdio_operations
{
    char name[16];
} sdio_operations;

typedef struct _iorw_extended_t
{
    cyg_uint8 rw_flag;          /** If 0 command is READ; else if 1 command is WRITE */
    cyg_uint8 func_num;
    cyg_uint8 blkmode;
    cyg_uint8 op_code;
    cyg_uint32 reg_addr;
    cyg_uint32 byte_cnt;
    cyg_uint32 blk_size;
    cyg_uint8 *tmpbuf;
} iorw_extended_t;

typedef struct _ioreg
{
    cyg_uint8 read_or_write;
    cyg_uint8 function_num;
    cyg_uint32 reg_addr;
    cyg_uint8 dat;
} ioreg_t;

typedef struct _mmc_notifier_rec mmc_notifier_rec_t;
typedef struct _mmc_notifier_rec *mmc_notifier_t;

typedef int (*mmc_notifier_fn_t) (mmc_card_t);

struct _mmc_notifier_rec
{
    mmc_notifier_fn_t add;
    mmc_notifier_fn_t remove;
};

typedef enum _mmc_response
{
    MMC_NORESPONSE = 1,
    MMC_R1,
    MMC_R2,
    MMC_R3,
    MMC_R4,
    MMC_R5,
    MMC_R6
} mmc_response;

enum _cmd53_rw
{
    IOMEM_READ = 0,
    IOMEM_WRITE = 1
};

enum _cmd53_mode
{
    BLOCK_MODE = 1,
    BYTE_MODE = 0
};

enum _cmd53_opcode
{
    FIXED_ADDRESS = 0,
    INCREMENTAL_ADDRESS = 1
};

#define	MMC_TYPE_HOST		1
#define	MMC_TYPE_CARD		2
#define	MMC_REG_TYPE_USER	3

#define	SDIO_READ		0
#define	SDIO_WRITE		1
#define	MRVL_MANFID		0x2df
#define	MRVL_DEVID		0x9103
#define	NO			0
#define	YES			1
#define	TRUE			1
#define	FALSE			0
#define	ENABLED 		1
#define	DISABLED 		0
#define	SIZE_OF_TUPLE 		255

/* Functions exported out for WLAN Layer */
void *sdio_register(int type, void *ops, int memory);
int sdio_unregister(int type, void *ops);
//int sdio_release(struct inode *in, struct file *file);
int stop_clock(sdio_ctrller * ctrller);
int start_clock(sdio_ctrller * ctrller);
int sdio_write_ioreg(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 dat);
int sdio_read_ioreg(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 * dat);
int sdio_read_iomem(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 blockmode,
                    cyg_uint8 opcode, ssize_t cnt, ssize_t blksz, cyg_uint8 * dat);
int sdio_write_iomem(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 blockmode,
                     cyg_uint8 opcode, ssize_t cnt, ssize_t blksz, cyg_uint8 * dat);
void sdio_print_imask(mmc_controller_t);
#if 0
int sdio_request_irq(mmc_card_t card,
                     IRQ_RET_TYPE(*handler) (int, void *, struct pt_regs *),
                     unsigned long irq_flags,
                     const char *devname, void *dev_id);
#endif
int sdio_request_irq(mmc_card_t card,
                 cyg_uint32(*handler) (cyg_vector_t, cyg_addrword_t),
                 unsigned long irq_flags, const char *devname, void *dev_id);
int sdio_free_irq(mmc_card_t card, void *dev_id);
int sdio_read_cisinfo(mmc_card_t card, cyg_uint8 *);
void sdio_enable_SDIO_INT(void);
int sdio_suspend(mmc_card_t card);
int sdio_resume(mmc_card_t card);
int sdio_get_vendor_id(mmc_card_t card);
void print_addresses(sdio_ctrller * ctrller);

#ifdef _MAINSTONE
#endif /* _MAINSTONE */

#ifdef HOTPLUG
extern char hotplug_path[];
extern int call_usermodehelper(char *path, char **argv, char **envp);
#endif

#endif /* __SDIO__H */
