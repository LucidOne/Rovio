/*
 * 	File	: sdio.c 
 *	Low level SDIO Driver
 *
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */

//#include	"include.h"
#include	"kapi.h"
//#include	"pxa-regs.h"
#include	"dma.h"
#include	"sdio.h"
//#include	"sdio_defs.h"
//#include	"sdio_error.h"
//#include	"sdio_spec.h"
//#include	"net/sock.h"
#include "pkgconf/io.h"
#include "cyg/io/io.h"
#include "cyg/io/devtab.h"
#include "sys/malloc.h"
#include "stdio.h"
#include "string.h"
#include "w99702_reg.h"
#include "wblib.h"
#include "wb_fmi.h"
//#include "sdio.h"
#include "if_sdio.h"
#include "w702sdio.h"


//#include "sdio_defs.h"

#define NON_CACHE_FLAG		0x10000000

//#undef 		MMC_PRTBUF
//#define		__REG(x)    (*((cyg_uint32 volatile*)(x)))
//#define		MMC_PRTBUF 	__REG(0x41100024)  /* Partial MMC_TXFIFO FIFO writte

#define outpw(port,value)     (*((cyg_uint32 volatile *) (port))=value)
#define inpw(port)            (*((cyg_uint32 volatile *) (port)))


#define MAX_FIRMWARE_POLL_TRIES		100
#define FIRMWARE_TRANSFER_NBLOCK	2
#define SDIO_HEADER_LEN		4

/* define SD block size for firmware download */
#define SD_BLOCK_SIZE_FW_DL	32
#define 	wmb()	//mb()

cyg_handle_t  gpioIntrHandle;
static cyg_interrupt gpioIntrObject;
cyg_handle_t  sdioIntrHandle;
static cyg_interrupt sdioIntrObject;

/* Global Declarations */

#define SD_BLOCK_SIZE		320     /* To minimize the overhead of ethernet frame
                                           with 1514 bytes, 320 bytes block size is used */

#define ALLOC_BUF_SIZE	(((Maximum(MRVDRV_ETH_RX_PACKET_BUFFER_SIZE, MRVDRV_SIZE_OF_CMD_BUFFER) \
					+ SDIO_HEADER_LEN + SD_BLOCK_SIZE - 1) / SD_BLOCK_SIZE) * SD_BLOCK_SIZE)

#define FIRMWARE_READY			0xfedc

//#define printf	diag_printf

/* global variable */
//static 
sdio_ctrller *sdio_host;
fmi_init_handler_t fmi_do_handle;
//cyg_mutex_t sdiofmi_mutex;


/* Static Variables */
static int irqnum;
static cyg_uint32(*sdioint_handler) (cyg_vector_t, cyg_addrword_t);
static void *net_devid;
static int MMC_I_MASK, MMC_STAT, MMC_STRPCL;
//#define MODULE_GET			try_module_get(THIS_MODULE)
//#define MODULE_PUT			module_put(THIS_MODULE)

/* By default kernel is enabled with HOTPLUG. So HOTPLUG1... */

#ifdef CONFIG_HOTPLUG1
static cyg_uint32 hotplug_extra;
static struct timer_list sdio_detect_card_thr_timer;

static enum
{
    MMC_CARD_REMOVED,
    MMC_CARD_INSERTED
} slot_state = MMC_CARD_REMOVED;
#endif /* HOTPLUG */

/* Static Function Declarations */
static int cleanup_host_controller(sdio_ctrller * ctrller);
static void cleanup_dma_allocated_memory(sdio_ctrller * ctrller);
static int check_ctrller_state(sdio_ctrller * ctrller, int state);
static int sdio_initialize_ireg(sdio_ctrller * ctrller, int mask);
static int sdio_wait_for_interrupt(sdio_ctrller * ctrller, int mask);
static int set_state(sdio_ctrller * ctrller, int state);
static int check_iomem_args(sdio_ctrller * ctrller, iorw_extended_t * io_rw);
static int write_blksz(sdio_ctrller * ctrller, iorw_extended_t * io_rw);
static int cmd53_reador_write(sdio_ctrller * ctrller,
                              iorw_extended_t * io_rw);

void sdio_disable_SDIO_INT(void);

#ifdef _MAINSTONE
#define _MAINSTONE_GPIO_IRQ
#endif

#ifdef _MAINSTONE_GPIO_IRQ

int start_bus_clock(mmc_controller_t ctrlr);

/* GPIO Interrupt Service Routine */
static cyg_uint32
gpio_irq(cyg_vector_t vector, cyg_addrword_t data)
{
    mmc_controller_t ctrlr = (mmc_controller_t) data;

    if (start_bus_clock(ctrlr))
        diag_printf("start_bus_clock() failed in gpio ISR\n");
    else
        diag_printf("SDIO clock started inside the gpio ISR\n");
    
    cyg_interrupt_acknowledge(vector);
	return CYG_ISR_HANDLED;
}

static int
init_gpio_irq(mmc_controller_t ctrlr)
{
#if 0
    SET_PXA_IRQ_TYPE(WAKEUP_GPIO);

    if (request_irq(WAKEUP_GPIO_IRQ, gpio_irq, 0, "WAKEUP_GPIO_IRQ", ctrlr)) {
        diag_printf("failed to request GPIO%d as WAKEUP_GPIO_IRQ(%d)\n",
               WAKEUP_GPIO, WAKEUP_GPIO_IRQ);
        return -1;
    }
	
	cyg_drv_interrupt_create(WAKEUP_GPIO_IRQ,0,(unsigned)ctrlr, gpio_irq, NULL,&gpioIntrHandle,&gpioIntrObject);
	cyg_drv_interrupt_attach(gpioIntrHandle);
#endif

    return 0;
}

static int
remove_gpio_irq(mmc_controller_t ctrlr)
{
    free_irq(WAKEUP_GPIO_IRQ, ctrlr);

    return 0;
}

#endif

/** 
	This function checks the error code that is returned by the
 	MMC_STAT. This is useful to locate the problems in the 
 	card / controller
	
	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
check_for_errors(sdio_ctrller * ctrller)
{
    cyg_uint16 tmp;
    cyg_uint32 ret = MMC_ERROR_GENERIC;

    /* Check for the good condition first! If there are no errors
     * return back, else check the state of the mmc_stat to
     * know the cause of the error
     */

    tmp = (ctrller->mmc_stat & 0x000001ff & MMC_STAT_ERRORS);

    _DBGMSG("tmp success ? 0x%08x\n", tmp);

    if (!tmp)
        return 0;

    return ret;
}

/** 
	This function checks for the error stats for the response R1
	This is the response for CMD7
		
	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

#ifdef DEBUG_SDIO_REG
static int
check_for_err_response_mmc_r1(sdio_ctrller * ctrller)
{
    cyg_uint32 status, ret = SDIO_ERROR_R1_RESP;

    /* SDIO Spec : Response to Command 7 (in R1) contains the 
     * error code from bits 13-31 */
    status = ctrller->mmc_res[4];
    status = status << 24;
    status |= ctrller->mmc_res[3];
    status = status << 16;
    status |= ctrller->mmc_res[2];
    status = status << 8;
    status |= status << 0;

    if (!(status & 0xffffe000))
        return 0;

    return ret;
}
#endif

/**
	This function checks for the error status for the response R5
	This is the response for CMD52 or CMD53
		
	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
check_for_err_response_mmc_r5(sdio_ctrller * ctrller)
{
    cyg_uint32 err, ret = SDIO_ERROR_R5_RESP;

    /* The response R5 contains the error code in 23-31 bits
     */
    err = ctrller->mmc_res[3];

    err = err << 8;

    err |= ctrller->mmc_res[4];

    if (!(err & 0xff))
        return 0;

    return ret;
}

/** 
	This function checks for the error status for the response R6
  	This is the response for CMD3

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
check_for_err_response_mmc_r6(sdio_ctrller * ctrller)
{
    cyg_uint16 err = 0;
    int ret = SDIO_R6_ERROR;

    /*SDIO Spec : The bits 13-15 indicates the error for the 
     * response r6 (CMD3 response)
     */
    err |= ctrller->mmc_res[2] << 8;
    err |= ctrller->mmc_res[1] << 0;

    if (!(err & 0xe0))
        return 0;
    return ret;
}

/** 
	This function interprets the command response as described in the
	SDIO spec

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
interpret_cmd_response(sdio_ctrller * ctrller, mmc_response format)
{
    int nwords, ret;

    switch (format) {
    case MMC_NORESPONSE:
        nwords = -1;
        break;
    case MMC_R1:
        nwords = 3;
        break;
    case MMC_R2:
        nwords = 8;
        break;
    case MMC_R3:
        nwords = 3;
        break;
    case MMC_R4:
        nwords = 3;
        break;
    case MMC_R5:
        nwords = 3;
        break;
    case MMC_R6:
        nwords = 3;
        break;
    default:
        nwords = -1;
        break;
    }

    ret = nwords;

    /* - PXA Manual:
       MMC_RES (MMC_RESPONSE_FIFO) is a 16 bit wide by 8 entries
       SDIO-Spec: The response R1, R4, R5, R6  are 48 bit value. 
       Read the MMC_RES to get them
     */

    if (nwords > 0) {
        register int i;
        int ibase;
        cyg_uint16 resp;
        for (i = nwords - 1; i >= 0; i--) {
           // resp = MMC_RES;
            ibase = i << 1;

            ctrller->mmc_res[ibase] = resp;
            resp = resp >> 8;
            ctrller->mmc_res[ibase + 1] = resp & 0x00ff;
            --ret;
        }
#ifdef DEBUG_SDIO_REG
        if (format == MMC_R1)
            ret = check_for_err_response_mmc_r1(ctrller);

        if (ret < 0) {
            _ERROR("Error in MMC_R1 Response ..\n");
            goto exit;
        }
#endif
    }

    if (format == MMC_R5)
        ret = check_for_err_response_mmc_r5(ctrller);

    else if (format == MMC_R6)
        ret = check_for_err_response_mmc_r6(ctrller);

    if (ret < 0) {
        diag_printf("Error: In the MMC Response\n");
        goto exit;
    }

    /* No errors , Lets set the state to End Cmd */
    set_state(ctrller, SDIO_FSM_END_CMD);
    return ret;

  exit:
    diag_printf("Error in response of MMC_R5 Or MMC_R6\n");
    return ret;
}

/** 
	This function sets the interrupt flag in the appropriate registers
 	and waits for the interrupt to occur

	\param controller pointer to sdio_ctrller structure, response_t format, 
	and the flag
	\returns 0 on success else negative value is returned
*/

static mmc_error_t
send_command(sdio_ctrller * ctrller, mmc_response format, int flag)
{
    cyg_uint32 ret;

    _ENTER();
#if 0
    sdio_initialize_ireg(ctrller, MMC_I_END_CMD_RES);

    //MMC_PRTBUF = MMC_PRTBUF_BUF_FULL;

    start_clock(ctrller);

    sdio_wait_for_interrupt(ctrller, MMC_I_END_CMD_RES);

    /* -Ideally the response should be located on 
     * MMC_RES, check for the ERRORS first and 
     * read the response from the card 
     */
    ret = check_for_errors(ctrller);

    if (ret < 0) {
        diag_printf("Response error %d\n", ret);
        goto exit;
    }

    ret = interpret_cmd_response(ctrller, format);

    if (ret < 0) {
        diag_printf("Error in interpret cmd_response\n");
        goto exit;
    }

    _LEAVE();
    return ret;

  exit:
 #endif
    diag_printf("Error in command response\n");
    return ret;
}

/** 
	This function checks for the card capability, like if the memory 
 	is present , Number of I/O functions etc. This is the response interpret
  	of CMD5 
	
	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
check_for_card_capability(sdio_ctrller * ctrller)
{
    card_capability capability;

    _ENTER();

    /* SDIO Spec: bit 9-12 represents Number Of I/O Functions
     * 12-13 Memory present
     * 16-40 OCR 
     * If the I/O functions < 1 error
     */

    /* OCR */
    capability.ocr = 0x0;
    capability.ocr |= (PXA_MMC_RESPONSE(ctrller, 3) << 16);
    _DBGMSG("%x\n", capability.ocr);
    capability.ocr |= (PXA_MMC_RESPONSE(ctrller, 2) << 8);
    _DBGMSG("%x\n", capability.ocr);
    capability.ocr |= (PXA_MMC_RESPONSE(ctrller, 1) << 0);
    _DBGMSG("%x\n", capability.ocr);

    ctrller->card_capability.ocr = capability.ocr;

    /* NUM OF IO FUNCS */
    capability.num_of_io_funcs = ((PXA_MMC_RESPONSE(ctrller, 4)
                                   & 0x70) >> 4);

    ctrller->card_capability.num_of_io_funcs = capability.num_of_io_funcs;

    diag_printf("IO funcs %x\n", capability.num_of_io_funcs);

    if (capability.num_of_io_funcs < 1) {
        diag_printf("Card Doesnot support the I/O\n");
        return -1;
    }

    /* MEMORY? */
    capability.memory_yes = ((PXA_MMC_RESPONSE(ctrller, 4)
                              & 0x08) ? 1 : 0);
    diag_printf("Memory present  %x\n", capability.memory_yes);
    ctrller->card_capability.memory_yes = capability.memory_yes;

    _LEAVE();
    return 0;
}

/** 
	This function sends the commad 5 to the SDIO card.
  	Proper response to this CMD will further initiates the driver
  	If the response for CMD5 is not correct, this indicates that the 
	inserted card is not a SDIO card

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
send_iosend_op_cmd(sdio_ctrller * ctrller)
{
    int ret = -1;
    cyg_uint16 argl, argh;

    /* After Reset MMC Card must be initialized by sending 
     * 80 Clocks on MMCLK signal.
     */
   // MMC_CLKRT = MMC_CLKRT_FREQ_9_75MHZ; /* Open drain */

   // MMC_RESTO = MMC_RES_TO_MAX;

    ret = stop_clock(ctrller);

    /* -SDIO Spec : CMD5 is used to inquire about the voltage range
     * needed by the I/O card. The normal response to this command is
     * R4 . The host determines the card's configuration based on the
     * data contained in the R4
     *      ------------------------------------------------------------
     *      | S|D|RESERV|C|I/O|M|STB|I/O..................OCR|RESERVE|E|    
     *      ------------------------------------------------------------
     *      Refer to SD Spec.
     */

    argh = argl = 0x0;

    //MMC_CMD = CMD(5);
    //MMC_ARGH = argh;
    //MMC_ARGL = argl;
    wmb();
    //MMC_SPI = MMC_DISABLE_SPI;
    //MMC_CMDAT = MMC_CMDAT_INIT | MMC_CMDAT_R1;

    diag_printf("CMD 5...\n");
//      printk("SDIO init 1  CMD5(0x%04x%04x)\n",  argh, argl);

    ret = send_command(ctrller, MMC_R4, 0);

    if (!ret) {
        /* Fill up the High byte to argh from the reponse 
         *  recieved from the CMD5
         */
        argh = (PXA_MMC_RESPONSE(ctrller, 4) << 8) |
            PXA_MMC_RESPONSE(ctrller, 3);
        argh &= 0x00ff;

        /* Fill up the low byte to argl from the reponse 
         *  recieved from the CMD5
         */
        argl = (PXA_MMC_RESPONSE(ctrller, 2) << 8) |
            PXA_MMC_RESPONSE(ctrller, 1);
    }

    if (!argh && !argl) {
        /* If the value of argh and argl are NULL set the OCR to
         *  max value
         */

        /* Set to full limit */

        argh = 0x00ff;
        argl = 0xff00;
        diag_printf("Setting the card to full voltage limit\n");
    }

    /* SDIO Spec: Send the CMD5 again to set the OCR values */

//      printk("SDIO init 2 stop_bus_clock \n");
    ret = stop_clock(ctrller);

#if 0
    MMC_CMD = CMD(5);
    MMC_ARGH = argh;
    MMC_ARGL = argl;
    MMC_CMDAT = MMC_CMDAT_R1;
#endif
//      printk("SDIO init 3  CMD5(0x%04x%04x)\n",  argh, argl);
    ret = send_command(ctrller, MMC_R4, 0);
    ret = check_for_card_capability(ctrller);

    if (ret < 0) {
        diag_printf("Unable to get the OCR Value\n");
    }

    return ret;
}

/** 
	This function sets/Masks the Required flag for the MMC_I_REG and 
	indicates the initialization is done

	\param controller pointer to sdio_ctrller structure, mask 
	\returns 0 on success else negative value is returned
*/

static int
sdio_initialize_ireg(sdio_ctrller * ctrller, int mask)
{
    cyg_uint32 ret;

    _ENTER();

    ret = xchg(&ctrller->busy, 1);

    _DBGMSG("xchg success value = %d\n", ret);

    if (ret) {
        _ERROR("Error initialize ireg = %d\n", ret);
        goto error;
    }
#if IRQ_DEBUG
    ctrller->irqcnt = 1;
#endif

    //disable_irq(ctrller->irq_line);
//    cyg_interrupt_mask(IRQ_FMI);

    //init_completion(&ctrller->completion);

    /* If the card interrupts are not registered then Mask all the 
     * interrupts otherwise let the sdio_ints fall through
     * If you mask all the interrupts there is a problem of not 
     * getting the sdio_ints ...
     */

    ret = MMC_I_MASK_ALL & (~mask);

#if 0
    MMC_I_MASK = ret;

    wmb();
    if (ret != (MMC_I_MASK & MMC_I_MASK_ALL)) {
#ifdef IRQ_DEBUG
        diag_printf("set MMC_I_MASK fail, mask=%x, return=%x\n", ret, MMC_I_MASK);
#endif
        //enable_irq(ctrller->irq_line);
        //cyg_interrupt_unmask(IRQ_FMI);
        goto error;
    }
#endif
    //enable_irq(ctrller->irq_line);
//    cyg_interrupt_unmask(IRQ_FMI);

    _LEAVE();
    return 0;

  error:
    diag_printf("error in sdio_initialize_ireg\n");
    return -1;
}


/**
	This function waits for the interrupt that has been set previously
	If this function returns success , indicates that the interrupt
 	that was awaited is done

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
sdio_wait_for_interrupt(sdio_ctrller * ctrller, int mask)
{
    int ret;

    _ENTER();

    ret = xchg(&ctrller->busy, 1);

    if (!ret) {
        diag_printf("Errror Somewhere .... sdio_wait_for int %d\n", ret);
        goto done;
    }

    diag_printf("Waiting for the interrupt to occur\n");

#if IRQ_DEBUG
    ctrller->timeo = 0;
    del_timer(&timer);
    timer.function = wait_timeo;
    timer.expires = jiffies + 5UL * HZ;
    timer.data = (unsigned long) ctrller;
    add_timer(&timer);
#endif
    //wait_for_completion(&ctrller->completion);

#if IRQ_DEBUG
    del_timer(&timer);
    if (ctrller->timeo) {
        diag_printf("irq timed out: mask=%x stat=%x MMC_I_MASK =%x\n",
                mask, MMC_STAT, MMC_I_MASK);
        diag_printf("state %d\n", (ctrller->state));

        goto done;
    }
#endif

    if ((mask == MMC_I_MASK_ALL) || (ctrller->mmc_i_reg & mask)) {
        diag_printf("Success\n");
        ret = 0;
        goto done;
    } else {
#ifdef IRQ_DEBUG
        printk("Error in wait for completion!!!mask=%x,ireg=%x,current=%x\n",
               mask, ctrller->mmc_i_reg, MMC_I_REG);
#endif
        ret = -1;
        diag_printf("Error in wait_for_completion\n");
    }

    _LEAVE();

  done:
    xchg(&ctrller->busy, 0);
    return ret;
}

/** 
	This function just sets the state to the desired one.According 
	to pxa docs the controller automatically stops the clock after 
	end of a command 

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

static int
stop_dummy_clock(sdio_ctrller * ctrller)
{
    int ret = -1;

    ret = check_ctrller_state(ctrller, SDIO_FSM_CLK_OFF);

    if (!ret) {
        diag_printf("clock is already off\n");
        return 0;
    }

    ret = check_ctrller_state(ctrller, SDIO_FSM_BUFFER_IN_TRANSIT);

    if (!ret) {
        diag_printf("Buffer in transit\n");
        return -1;
    }

    set_state(ctrller, SDIO_FSM_CLK_OFF);

    return 0;
}

/** 
	This function stops the MMC/SDIO clock

	\param controller pointer to sdio_ctrller structure
	\returns 0 on success else negative value is returned
*/

int
stop_clock(sdio_ctrller * ctrller)
{
    int ret = -1;

    ret = check_ctrller_state(ctrller, SDIO_FSM_CLK_OFF);

    if (!ret) {
        diag_printf(" Ctrller FSM is in CLK_OFF\n");
        goto out;
    }

    ret = check_ctrller_state(ctrller, SDIO_FSM_BUFFER_IN_TRANSIT);

    if (!ret) {
        diag_printf(" Ctrller FSM is not in BUFFER_IN_TRANSIT\n");
        goto out;
    }

        /** - Write 0x01 to MMC_STRCL clk
	 * Mask all the interrupts except CLK_IS_OFF
	 * Wait for MMC_I_REG interrupt
	 */
   // ret = sdio_initialize_ireg(ctrller, MMC_I_CLK_IS_OFF);

   // MMC_STRPCL = MMC_STRPCL_STOP_CLK;

    //wmb();

   // ret = sdio_wait_for_interrupt(ctrller, MMC_I_REG_CLK_IS_OFF);

    diag_printf("Stopping the card manually\n");

    set_state(ctrller, SDIO_FSM_CLK_OFF);

    return ret;

  out:
    diag_printf("Errr *****FSM in a Weired State***** \n");
    return 0;
}

/** IRQ handler for the DMA interrupts
 */

static void
dma_irq(int dma, void *devid)
{
    sdio_ctrller *ctrller = (sdio_ctrller *) devid;
    cyg_uint32 dcsr;
    cyg_uint32 ddadr;
    int chan = ctrller->chan;

    ddadr = DDADR(chan);
    dcsr = DCSR(chan);
   // DCSR(chan) = dcsr & ~DCSR_STOPIRQEN;

    _DBGMSG("<1>MMC DMA interrupt: chan=%d ddadr=0x%08x "
            "dcmd=0x%08x dcsr=0x%08x\n", chan, ddadr, DCMD(chan), dcsr);

    /* bus error */

    //if (dcsr & DCSR_BUSERR) 
    {
        diag_printf("<1>bus error on DMA channel %d\n", chan);
        set_state(ctrller, SDIO_FSM_ERROR);
        goto complete;
    }
    /* data transfer completed */
    //if (dcsr & DCSR_ENDINTR) 
    {
        set_state(ctrller, SDIO_FSM_END_BUFFER);
        goto complete;
    }
    return;

  complete:
//    complete(&ctrller->completion);
    diag_printf("Dma Irq Done\n");
    return;
}

/** This function sets up the GPIO lines as the DAT0-DAT4 and command lines
 
\param none
\returns 0 always
 */

static int
setup_gpio_lines(void)
{
#if 0
    /* MMCLK refer to 24-5 PXA developers Manual */
    SET_PXA_GPIO_MODE(32 | GPIO_ALT_FN_2_OUT);

    /* MMCMD  refer to 24-8 PXA developers Manual */
    SET_PXA_GPIO_MODE(112 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);

    /* MMDAT0 - MMDAT4 PXA developers Manual */
    SET_PXA_GPIO_MODE(92 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
    SET_PXA_GPIO_MODE(109 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
    SET_PXA_GPIO_MODE(110 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
    SET_PXA_GPIO_MODE(111 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
#endif
    return 0;
}

/** 
	This function enables the clock for the controller 
 
	\param none
	\returns 0 always
 */

static int
enable_clock(void)
{
//    CKEN |= CKEN12_MMC;
    return 0;
}

/** 
	This function disables the clock for the controller 

	\param none
	\returns 0 always
 */

static int
disable_clock(void)
{
//    CKEN &= ~CKEN12_MMC;
    return 0;
}

/** 
	Interrupt Service Routine 
*/

void sdio_interrupt()
{
    //sdio_ctrller *ctrller = (sdio_ctrller *)ptr;
    cyg_uint32 save_ireg, save_mask;
    int sdio_scheduled = 0;
#ifdef DEBUG_SDIO_LEVEL1
    static int sdio_interrupt_enabled = 0;
#endif

    //diag_printf("sdio interrupt\n");

    //irqnum++;

	//if((inpw(REG_SDIER) & 0x10) == 0x10)
	{
        /* This interrupt is actually WLAN interrupt. So give
         * the WLAN layer to manage the interrupt as it wants!
         */
       // diag_printf("sdio_interrupt .****.\n");
        /* Now this tells me that the WLAN driver is initialized 
         * and requested for the interrupt 
         */
        if (sdioint_handler != (void *) 0) {
            /* Check if there is any errors out here */

            //diag_printf("address of dev_id = %p  sdioint_handler =%p\n", net_devid, sdioint_handler);
            sdioint_handler(0, (cyg_addrword_t)net_devid);
            sdio_scheduled = 1;
            goto complete;
        } else
            diag_printf("Wlan card interrupt is not yet registered"
                    "sdio ints before firmware download \n");
    }

  complete:
  
	//if((inpw(REG_SDIER) & 0x10) == 0x10)
	{
		{
			//diag_printf("disable SDIO interrupt %x\n", inpw(REG_SDIER));
		}
		sdio_disable_SDIO_INT();
	}
  exit:
	return;
}

/** 
	This function requests the IRQ from the OS

	\param pointer to sdio_controller structure
	\returns 0 on success else negative value given by the OS is returned
*/

static int
request_interrupt(sdio_ctrller * ctrller)
{

#if 0	
    if (request_irq(IRQ_FMI, sdio_interrupt, 0, "SDIO_IRQ", ctrller) < 0) {
        diag_printf("Unable to request Interrupt\n");
        return -1;
    }
#endif

#if 0
	cyg_drv_interrupt_create(IRQ_FMI,0,(unsigned)ctrller, sdio_interrupt, NULL,&sdioIntrHandle,&sdioIntrObject);
	cyg_drv_interrupt_attach(sdioIntrHandle);
#endif
	
	fmiInstallSDIOFunc(sdio_interrupt);
	
    ctrller->irq_line = IRQ_FMI;
    ctrller->tmpl->irq_line = IRQ_FMI;

    return 0;
}

/** 
	This function sends the CMD3 and gets the relative RCA

	\param pointer to sdio_controller structure
	\returns 0 on success else negative value given by the OS is returned
*/

static int
send_relative_addr_cmd(sdio_ctrller * ctrller)
{
    int ret = -1;
    card_capability card_capb;

    _ENTER();

    /* This function sends the CMD 3 and intrprets the
     * response to verify if the response is OK
     */

    ret = stop_clock(ctrller);

//    MMC_CMD = CMD(3);
//    MMC_ARGH = 0x0;
//    MMC_ARGL = 0x0;
    wmb();
//    MMC_CMDAT = MMC_CMDAT_R1;   /* R6 */

    diag_printf("Command 3 ....\n");
//      printk("SDIO init 4  CMD3(0x%04x%04x)\n",  0, 0);

    ret = send_command(ctrller, MMC_R6, 0);

    if (ret)
        diag_printf("Problem in CMD 3\n");

    card_capb.rca = ((PXA_MMC_RESPONSE(ctrller, 4) << 8) |
                     PXA_MMC_RESPONSE(ctrller, 3));
    ctrller->card_capability.rca = card_capb.rca;

    diag_printf("CMD 3 card capability = %x\n", card_capb.rca);

    _LEAVE();
    return 0;
}

/** 
	This function sends the CMD7 to the card
	\param pointer to sdio_controller structure
	\returns 0 on success else negative value is returned
*/

static int
send_select_card_cmd(sdio_ctrller * ctrller)
{
    int ret;

    /* This function sends the CMD 7 and intrprets the
     * response to verify if the response is OK
     */

    /* Take the argh from the CMD3 response and pass it to 
     * CMD7
     */

    ret = stop_clock(ctrller);

//    MMC_CMD = CMD(7);
//    MMC_ARGH = ctrller->card_capability.rca;

//    MMC_ARGL = 0x0;
    wmb();
//    MMC_CMDAT = MMC_CMDAT_R1;   /* R6 */

//    _DBGMSG(" CMD 7 Value of argh = %x\n", MMC_ARGH);
//      printk("SDIO init 5  CMD7(0x%04x%04x)\n", ctrller->card_capability.rca, 0);

    ret = send_command(ctrller, MMC_R1, 0);

    return 0;
}

/** 
	This function is used to check the upper bound for the functions 
	CMD52

	\param pointer to sdio_controller structure , and the contents of ioreg 
	passed by the wlan layer
	\returns 0 on success else negative value is returned
*/

static int
check_for_valid_ioreg(sdio_ctrller * ctrller, ioreg_t * ioreg)
{
    _ENTER();

    if (!ioreg) {
        diag_printf("Bad ioreg\n");
        return -EINVAL;
    }

    if (ioreg->function_num > 7 || ioreg->function_num >
        ctrller->card_capability.num_of_io_funcs) {

        return -EINVAL;
    }

    /* Check register range */
    if ((ioreg->reg_addr & 0xfffe0000) != 0x0) {
        return -EINVAL;
    }

    _LEAVE();
    return 0;
}

/** 
	This function acquires the semaphore and makes sure there is 
	serializes the data

	\param pointer to sdio_controller structure
	\returns 0 on success else negative value given by the OS is returned
*/

static int
acquire_io(sdio_ctrller * ctrller)
{
    int ret = -1;

    if (!ctrller) {
        diag_printf("Bad ctrller\n");
        return ret;
    }

    cyg_mutex_lock(&ctrller->io_sem);

    return 0;
}

/** 
	This function release the semaphore and makes sure there is 
	serializes the data

	\param pointer to sdio_controller structure
	\returns 0 on success 
*/

static int
release_io(sdio_ctrller * ctrller)
{
    int ret = -1;

    if (!ctrller) {
        diag_printf("Bad ctrller\n");
        return ret;
    }

    cyg_mutex_unlock(&ctrller->io_sem);

    return 0;
}

/** 
	This function is used to either read or write a byte of data
	(CMD52) , fills up the CMD52 and sends it out on the cmd line

	\param pointer to sdio_controller structure , and contents of ioreg
	\returns 0 on success else negative value given by the OS is returned
*/

static int
rw_ioreg(sdio_ctrller * ctrller, ioreg_t * ioreg)
{
    int ret;
    cyg_uint16 argh = 0, argl = 0;

    _ENTER();

    /* Nicely fill up argh and argl send the command down
     * Read the response from MMC_R1
     */

    ret = acquire_io(ctrller);
    ctrller->num_ofcmd52++;

    if (ret < 0) {
        diag_printf("acquire io failed\n");
        goto exit;
    }

    ret = stop_dummy_clock(ctrller);

    if (ret < 0) {
        diag_printf("Error Cannot stop the clock \n");
        goto exit;
    }
    /* SDIO Spec: CMD52 is 48 bit long. 
       -----------------------------------------------------------
       S|D|CMDIND|R|FUN|F|S|REGISTER  ADDRESS|S|WRITEBIT|CRC    7|
       -----------------------------------------------------------
       The Command and the Command index will be filled by the SDIO Controller
       (48 - 16)
       So fill up argh (16 bits) argl (16 bits) with 
       R/W flag (1)
       FUNC NUMBER (3)
       RAW FLAG (1)
       STUFF (1)
       REG ADDR (17)
       and the Write data value or a Null if read
     */
    argh =
        (ioreg->read_or_write ? (1 << 15) : 0) |
        (ioreg->function_num << 12) |
        (ioreg->read_or_write == 2 ? (1 << 11) : 0) |
        ((ioreg->reg_addr & 0x0001ff80) >> 7);

    argl =
        ((ioreg->reg_addr & 0x0000007f) << 9) |
        (ioreg->read_or_write ? ioreg->dat : 0);

//    MMC_CMD = CMD(52);
//    MMC_ARGH = argh;
//    MMC_ARGL = argl;
    wmb();

#define HOST_INTSTATUS_REG 0x05
#define CARD_RESET 8
    /* Disabling controller to check for SDIO interrupt from the card
       solves the extra interrupt issue. The next CMD52 write will re-enable it. */
    if ((ioreg->reg_addr == HOST_INTSTATUS_REG) && !ioreg->read_or_write &&
        (ioreg->function_num == FN1))
        ;
//        MMC_CMDAT = MMC_CMDAT_R1 & (~MMC_CMDAT_SDIO_INT_EN);
    else if ((ioreg->reg_addr == IO_ABORT_REG) && ioreg->read_or_write &&
             (ioreg->function_num == FN0) &&
             ((ioreg->dat & CARD_RESET) == CARD_RESET))
             ;
 //       MMC_CMDAT = MMC_CMDAT_SDIO_INT_EN;
    else
    ;
 //       MMC_CMDAT = MMC_CMDAT_R1;       /* R5 */

    ret = send_command(ctrller, MMC_R5, 0);

    if (ret < 0) {
        diag_printf("send_command failed\n");
        goto exit;
    }

    ioreg->dat = PXA_MMC_RESPONSE(ctrller, 1);

    _DBGMSG("ioreg->dat = %x\n", ioreg->dat);

    release_io(ctrller);

    _LEAVE();
    return 0;

  exit:
    release_io(ctrller);
    return -1;
}


/** 
	This function is the hook to the wlan driver to set the bus width
 	either to 1 bit mode or to 4 bit mode

	\param pointer to the card structure and the mode (1bit / 4bit)
	\return returns 0 on success negative value on error
*/

int
sdio_set_buswidth(mmc_card_t card, int mode)
{
#if 0
    switch (mode) {
    case SDIO_BUSWIDTH_1_BIT:
        sdio_write_ioreg(card, FN0, BUS_INTERFACE_CONTROL_REG, 0x00);

        card->ctrlr->bus_width = SDIO_BUSWIDTH_1_BIT;
        diag_printf("\n SDIO: Bus width is " "set to 1 bit mode\n");
        break;

    case SDIO_BUSWIDTH_4_BIT:
        sdio_write_ioreg(card, FN0, BUS_INTERFACE_CONTROL_REG, 0x02);

        card->ctrlr->bus_width = SDIO_BUSWIDTH_4_BIT;

        diag_printf("\n SDIO: Bus width is " "set to 4 bit mode\n");

        break;
    default:
        diag_printf("Not supported Mode, force to 4 bit mode\n");

        sdio_write_ioreg(card, FN0, BUS_INTERFACE_CONTROL_REG, 0x02);

        card->ctrlr->bus_width = SDIO_BUSWIDTH_4_BIT;
        break;
    }
#endif
    return 0;
}

/** 
	This function is the hook to the wlan driver to set the clock frequency
	either to 10 MHz or to 20 MHz 
	
	\param pointer to card structure and the clock to operate at
	\return returns 0 on success 
*/

int
sdio_set_clkrate(mmc_card_t card, int clkrate)
{
#if 0
    switch (clkrate) {

    case SDIO_CLK_RATE_10MHZ:
        MMC_CLKRT = MMC_CLKRT_FREQ_9_75MHZ;
        diag_printf("\nSDIO: Clock speed set to 10MHZ\n");
        break;

    case SDIO_CLK_RATE_20MHZ:
        MMC_CLKRT = MMC_CLKRT_FREQ_19_5MHZ;
        diag_printf("\nSDIO: Clock speed set to 20MHZ\n");
        break;

    default:
        diag_printf("\nSDIO: Un supported clk rate. force to 20MHZ\n");
        MMC_CLKRT = MMC_CLKRT_FREQ_19_5MHZ;
        break;
    }
#endif
    return 0;
}

/** 
	This function is the hook to the wlan driver to read a byte of data 
	from the card

	\param pointer to the card structure, function num and register address
	value to be read 
	\returns 0 on success negative value on error
*/

/* used function */

int sdio_read_ioreg(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 * dat)
{
	SDIO_DATA_T	sdio52;
	cyg_int32 ret;

	sdio52.regAddr = reg;
	sdio52.funNo = func;
	sdio52.WriteData = 0x0;
	sdio52.IsReadAfterWrite = FALSE;
	
	//cyg_mutex_lock(&sdiofmi_mutex);
	//sdio_bit_config(4);		// 4-bit
	ret = fmiSDIO_Read(&sdio52);
	if(ret < 0)
	{
		diag_printf("Read IO Register[%x] error try again\n", reg);
		ret = fmiSDIO_Read(&sdio52);
	}
	//sdio_bit_config(1);		// 1-bit
	//cyg_mutex_unlock(&sdiofmi_mutex);
	
	if (ret < 0)
		return ret;
	else
		*dat = (cyg_uint8)ret;

	return Successful;
}

int sdio_write_ioreg(mmc_card_t card, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 dat)
{
	SDIO_DATA_T	sdio52;
	cyg_int32 ret;

	sdio52.regAddr = reg;
	sdio52.funNo = func;
	sdio52.WriteData = dat;
	sdio52.IsReadAfterWrite = TRUE;
	
	//cyg_mutex_lock(&sdiofmi_mutex);
	//sdio_bit_config(4);		// 4-bit
	ret = fmiSDIO_Write(&sdio52);
	if (ret < 0)
	{
		diag_printf("Write IO Register[%x] error try again\n", reg);
		ret = fmiSDIO_Write(&sdio52);
	}
	//sdio_bit_config(1);		// 1-bit
	//cyg_mutex_unlock(&sdiofmi_mutex);
	
	if (ret < 0)
		return ret;

	return Successful;
}

int sdio_write_iomem(mmc_card_t card, cyg_uint8 func, cyg_uint32 sdio_reg, cyg_uint8 blockmode,
                 cyg_uint8 opcode, ssize_t cnt, ssize_t blksz, cyg_uint8 * dat)
{
	SDIO_MULTIDATA_T sdio53;
    int ret;

#ifdef DEBUG_USEC
    ulong time;
#endif
    sdio_ctrller *ctrller = card->ctrlr;
    iorw_extended_t io_rw;

    _ENTER();

    /* Theory: PXA Manual:
     * The 2 transmit data fifo's are writable , Each transmit data FIFO
     * is a 32 entries of 1 byte data. 
     * To access the FIFO by DMA the software must program the dma to read
     * or write the SDIO FIFO's with a single byte transfer and 32 byte 
     * burst
     * The CMDAT[DMA_ENAB] bit must be set to enable communication
     * with the DMA 
     * Block Data Write: After turning the clock on to start the 
     * command sequence the s/w must program the DMA to fill the 
     * MMC_TXFIFO. The software must continue to fill the FIFO 
     * until all the data has been written to the fifo. 
     * The s/w then should wait for MMC_I_REG[DATA_TRANS_DONE] interrupt
     * and MMC_I_REG[PRG_DONE] interrupt. The s/w can read the MMC_STAT
     * to know the status
     */

    io_rw.rw_flag = IOMEM_WRITE;
    io_rw.func_num = func;
    io_rw.reg_addr = sdio_reg;
    io_rw.blkmode = blockmode;
    io_rw.op_code = opcode;
    io_rw.byte_cnt = cnt;
    io_rw.blk_size = blksz;
    io_rw.tmpbuf = dat;

//    diag_printf("sdio_write_iomem\n");
//    diag_printf("CMD 53 write values rw_flag = %x func_num = %x"
//            "reg_addr = %x, blkmode = %x opcode = %x count = %x"
//            "blk size = %x buf_addr = %p", io_rw.rw_flag,
//            io_rw.func_num, io_rw.reg_addr, io_rw.blkmode,
//            io_rw.op_code, io_rw.byte_cnt, io_rw.blk_size, io_rw.tmpbuf);
#ifdef DEBUG_USEC
    time = get_usecond();
    diag_printf("sdio_write_iomem: time in = %lu\n", time);
#endif

    ret = check_iomem_args(ctrller, &io_rw);

    if (ret < 0) {
        diag_printf("Wrong parameters passed to sdio_write_iomem\n");
        goto exit;
    }

	sdio53.blockSize = blksz;
	sdio53.regAddr = sdio_reg;
	sdio53.bufAddr = (cyg_uint32)dat;
	sdio53.Count = cnt;
	sdio53.funNo = func;
	if (opcode == FIXED_ADDRESS)
		sdio53.OpCode = FMI_SDIO_FIX_ADDRESS; //FMI_SDIO_INC_ADDRESS;
	if (blockmode == BLOCK_MODE)
		sdio53.BlockMode = FMI_SDIO_MULTIPLE; //FMI_SDIO_SINGLE;
	//cyg_mutex_lock(&sdiofmi_mutex);
	//sdio_bit_config(4);		// 4-bit
	ret = fmiSDIO_BlockWrite(&sdio53);
	if (ret < 0)
	{
		diag_printf("rw_iomem error CMD53 write fails [%x] try again\n", ret);
		ret = fmiSDIO_BlockWrite(&sdio53);
	}
	//sdio_bit_config(1);		// 1-bit
	//cyg_mutex_unlock(&sdiofmi_mutex);
    if (ret < 0) {
        diag_printf("rw_iomem error CMD53 write fails [%x]\n", ret);
        goto exit;
    }

  exit:
#ifdef DEBUG_USEC
    time = get_usecond();
    diag_printf("sdio_write_iomem: time out = %lu\n", time);
#endif

    _LEAVE();
    return ret;
}


int sdio_read_iomem(mmc_card_t card, cyg_uint8 func, cyg_uint32 sdio_reg, cyg_uint8 blockmode,
                cyg_uint8 opcode, ssize_t cnt, ssize_t blksz, cyg_uint8 * dat)
{
	SDIO_MULTIDATA_T sdio53;
    int ret;
#ifdef DEBUG_USEC
    ulong time;
#endif
    sdio_ctrller *ctrller = card->ctrlr;
    iorw_extended_t io_rw;

    io_rw.rw_flag = IOMEM_READ;
    io_rw.func_num = func;
    io_rw.reg_addr = sdio_reg;
    io_rw.blkmode = blockmode;
    io_rw.op_code = opcode;
    io_rw.byte_cnt = cnt;
    io_rw.blk_size = blksz;
    io_rw.tmpbuf = dat;

    _ENTER();

//    diag_printf("sdio_read_iomem blksize = %d  count = %d  buffer = %x\n",
//            blksz, cnt, dat);
//    diag_printf("Values are io_rw.rw_flag = %x io_rw.func_num =%x"
//            "io_rw.reg_addr=%x io_rw.blkmode =%x io_rw.op_code =%x"
//            "io_rw.byte_cnt =%xio_rw.blk_size =%x io_rw.buf =%p\n",
//            io_rw.rw_flag, io_rw.func_num, io_rw.reg_addr,
//            io_rw.blkmode, io_rw.op_code, io_rw.byte_cnt,
//            io_rw.blk_size, io_rw.tmpbuf);

#ifdef DEBUG_USEC
    time = get_usecond();
    diag_printf("sdio_read_iomem: time in = %lu\n", time);
#endif
    ret = check_iomem_args(ctrller, &io_rw);

    if (ret < 0) {
        diag_printf("Wrong parameters passed to sdio_write_iomem\n");
        goto exit;
    }

	sdio53.blockSize = blksz;
	sdio53.regAddr = sdio_reg;
	sdio53.bufAddr = (cyg_int32)dat;
	sdio53.Count = cnt;
	sdio53.funNo = func;
	if (opcode == FIXED_ADDRESS)
		sdio53.OpCode = FMI_SDIO_FIX_ADDRESS; //FMI_SDIO_INC_ADDRESS;
	if (blockmode == BLOCK_MODE)
		sdio53.BlockMode = FMI_SDIO_MULTIPLE; //FMI_SDIO_SINGLE;
	//cyg_mutex_lock(&sdiofmi_mutex);
	//sdio_bit_config(4);		// 4-bit
	ret = fmiSDIO_BlockRead(&sdio53);
	if (ret < 0) {
        diag_printf("rw_iomem error CMD53 read fails [%x] try again\n", ret);
        ret = fmiSDIO_BlockRead(&sdio53);
    }
    //sdio_bit_config(1);		// 1-bit
	//cyg_mutex_unlock(&sdiofmi_mutex);
    if (ret < 0) {
        diag_printf("rw_iomem error CMD53 read fails [%x]\n", ret);
        goto exit;
    }

  exit:
#ifdef DEBUG_USEC
    time = get_usecond();
    diag_printf("sdio_read_iomem: time out = %lu\n", time);
#endif
    _LEAVE();
    return ret;
}



/** This function checks the upper bound for the CMD53 read or write
 */

static int
check_iomem_args(sdio_ctrller * ctrller, iorw_extended_t * io_rw)
{
    if (!ctrller)
        diag_printf("Null ctrller\n");

    /* Check function number range */
    if (io_rw->func_num > ctrller->card_capability.num_of_io_funcs) {
        return -ERANGE;
    }

    /* Check register range */
    if ((io_rw->reg_addr & 0xfffe0000) != 0x0) {
        return -ERANGE;
    }

    /* Check cnt range */
    if (io_rw->byte_cnt == 0 || (io_rw->byte_cnt & 0xfffffe00) != 0x0) {
        return -ERANGE;
    }

    /* Check blksz range */
    if (io_rw->blkmode && (io_rw->blk_size == 0 || io_rw->blk_size > 0x0800)) {
        return -ERANGE;
    }

    /* Check null-pointer */
    if (io_rw->tmpbuf == 0x0) {
        return -EINVAL;
    }

    return 0;
}

/** This function is used to read / write block of data from the card
 */

static int
write_blksz(sdio_ctrller * ctrller, iorw_extended_t * io_rw)
{
    int ret;
    ioreg_t ioreg;

    if (io_rw->blkmode && ctrller->card_capability.
        fnblksz[io_rw->func_num] != io_rw->blk_size) {
        /* Write low byte */
        _DBGMSG("looks like a odd size printing the values"
                "blkmode=0x%x blksz=%d fnblksz =%d\n",
                io_rw->blkmode, io_rw->blk_size,
                ctrller->card_capability.fnblksz[io_rw->func_num]);

        ioreg.read_or_write = SDIO_WRITE;
        ioreg.function_num = FN0;
        ioreg.reg_addr = FN_BLOCK_SIZE_0_REG(io_rw->func_num);
        ioreg.dat = (cyg_uint8) (io_rw->blk_size & 0x00ff);

        if (rw_ioreg(ctrller, &ioreg) < 0) {
            diag_printf("rw_ioreg failed rw_iomem\n");
            ret = -ENXIO;
            goto err_down;
        }

        /* Write high byte */
        ioreg.read_or_write = SDIO_WRITE;
        ioreg.function_num = FN0;
        ioreg.reg_addr = FN_BLOCK_SIZE_1_REG(io_rw->func_num);
        ioreg.dat = (cyg_uint8) (io_rw->blk_size >> 8);

        if (rw_ioreg(ctrller, &ioreg) < 0) {
            diag_printf("rw_ioreg failed rw_iomem 1\n");
            ret = -ENXIO;
            goto err_down;
        }

        ctrller->card_capability.fnblksz[io_rw->func_num] = io_rw->blk_size;
    }

    return 0;

  err_down:
    diag_printf("rw_iomem failed\n");
    return -1;
}

/* 
	This function sets up the DMA channel for the DMA and reads the 
	specified amount of data (blocks) from the SDIO card to host

	\param pointer to sdio controller structure and number of bytes to be 
	read
	\return returns byte count to be read or negative on error
*/

static int
trigger_dma_read(sdio_ctrller * ctrller, int byte_cnt)
{

    ssize_t ret = -EIO;
    register int ndesc;
    int chan = ctrller->chan;

    pxa_dma_desc *desc;

    if ((ctrller->state != SDIO_FSM_END_CMD) &&
        (ctrller->state != SDIO_FSM_END_BUFFER)) {
        diag_printf("<1>unexpected state (%d)", (ctrller->state));
        goto error;
    }

    if (byte_cnt > ctrller->bufsz)
        byte_cnt = ctrller->bufsz;

    set_state(ctrller, SDIO_FSM_BUFFER_IN_TRANSIT);

    if (sdio_initialize_ireg(ctrller, ~MMC_I_MASK_ALL)) {
        diag_printf("<1>drv_init_completion failed read_buffer1\n");
        goto error;
    }

#if 0
    if ((desc = ctrller->last_read_desc)) {
        desc->ddadr &= ~DDADR_STOP;
        desc->dcmd &= ~(DCMD_ENDIRQEN | DCMD_LENGTH);
        desc->dcmd |= (1 << 5);
    }

    /* 1) setup descriptors for DMA transfer from the device */
    ndesc = (byte_cnt >> 5) - 1;
    desc = &ctrller->read_desc[ndesc];
    ctrller->last_read_desc = desc;
    desc->ddadr |= DDADR_STOP;
    desc->dcmd |= DCMD_ENDIRQEN;
    DDADR(chan) = ctrller->read_desc_phys_addr;
#endif
    wmb();
//    DCSR(chan) |= DCSR_RUN;

    if ((ret = sdio_wait_for_interrupt(ctrller, MMC_I_MASK_ALL)) < 0) {
        diag_printf("<1>drv_wait_for_completion failed read_buffer2\n");
        goto error;
    }

    if (check_ctrller_state(ctrller, SDIO_FSM_END_BUFFER)) {
        diag_printf("<1>__check_state failed -read_buffer\n");
        goto error;
    }

    if (!(ctrller->mmc_stat & MMC_STAT_ERRORS))
        ret = byte_cnt;
  error:
    diag_printf("dma_read returning ret = %d\n", ret);
    return ret;
}

/** 
	This function reads the data filled in by the DMA channel on to the
	buffer passed by the WLAN driver

	\param pointer to controller structure, pointer to buffer and the count 
	to copied
	\return returns the count copied or a negative value on error
*/

static int
fill_buffer_forread(sdio_ctrller * ctrller, char *buf, int count)
{
    int *ret;

    ret = memcpy(buf, ctrller->iodata, count);

    if (!ret) {
        diag_printf("memcpy failed in fill_buffer_forread\n");
        return -1;
    }

    return count;
}

/** 
	This function writes the data to DMA buffer from the buffer passed by 
	the WLAN driver

	\param pointer to controller structure, pointer to buffer and the count 
	to copied
	\return returns the count copied or a negative value on error
*/

int
fill_buffer_forwrite(sdio_ctrller * ctrller, char *buf, int count)
{
    int *ret;

    _ENTER();

    if (!ctrller)
        diag_printf("Null pointer... fill_buffer_for_write\n");

    if (!buf)
        diag_printf("Buffer empty fill_buffer_for_write\n");

    diag_printf("Just before memcpy count = %d\n", count);

    ret = memcpy(ctrller->iodata, buf, count);

    if (!ret) {
        diag_printf("memcpy failed in fill_buffer_forwrite\n");
        return -1;
    }

    _LEAVE();
    return count;
}

/** 
	This function sets up the DMA channel for the DMA and writes the 
	specified amount of data (blocks) from the hodst to SDIO card 
	
	\param pointer to sdio controller structure and number of bytes to be 
	read
	\return returns byte count to be read or negative on error
*/

int
trigger_dma_write(sdio_ctrller * ctrller, int cnt)
{
    ssize_t ret = -EIO;
    register int ndesc;

    int chan = ctrller->chan;
    pxa_dma_desc *desc;

    _ENTER();

    ctrller->state = SDIO_FSM_END_CMD;

    if ((ctrller->state != SDIO_FSM_END_CMD) &&
        (ctrller->state != SDIO_FSM_END_BUFFER)) {
        diag_printf("<1>Unexpected state (%d)\n", (ctrller->state));
        goto error;
    }

    if (!cnt) {
        diag_printf("Count value is zero erring out trigger_dma_write\n");
        goto error;
    }

    if (cnt > ctrller->bufsz)
        cnt = ctrller->bufsz;

    set_state(ctrller, SDIO_FSM_BUFFER_IN_TRANSIT);

    if (sdio_initialize_ireg(ctrller, ~MMC_I_MASK_ALL)) {
        diag_printf("drv_init_completion failed write_buffer\n");
        goto error;
    }

#if 0
    if ((desc = ctrller->last_write_desc)) {
        desc->ddadr &= ~DDADR_STOP;
        desc->dcmd &= ~(DCMD_ENDIRQEN | DCMD_LENGTH);
        desc->dcmd |= (1 << 5);
    }
    /* setup descriptors for DMA transfer to the device */
    ndesc = (cnt >> 5) - 1;
    desc = &ctrller->write_desc[ndesc];

    ctrller->last_write_desc = desc;
    desc->ddadr |= DDADR_STOP;
    desc->dcmd |= DCMD_ENDIRQEN;
    /* start DMA channel */
    DDADR(chan) = ctrller->write_desc_phys_addr;
#endif
//    DCSR(chan) |= DCSR_RUN;

    if (sdio_wait_for_interrupt(ctrller, MMC_I_MASK_ALL)) {
        diag_printf("<1>drv_wait_for_completion dma failed" "write_buffer\n");
        goto error;
    }

    if (check_ctrller_state(ctrller, SDIO_FSM_END_BUFFER)) {
        diag_printf("<1>__check_state error write_buffer\n");
        goto error;
    }
    if (!(ctrller->mmc_stat & MMC_STAT_ERRORS))
        ret = cnt;

    _LEAVE();
  error:
    return ret;
}

int
complete_io(sdio_ctrller * ctrller, int rw_flag)
{
    int ret = MMC_ERROR_GENERIC;

    if (check_ctrller_state(ctrller, SDIO_FSM_END_IO)) {
        diag_printf("<1>check_state failed complete_io\n");
        goto error;
    }

    /*  wait for DATA_TRAN_DONE intr */

    if ((ret = sdio_initialize_ireg(ctrller, MMC_I_MASK_DATA_TRAN_DONE))) {
        goto error;
    }
    diag_printf("waiting for MMC_I_REG_DATA_TRAN_DONE\n");
    if ((ret = sdio_wait_for_interrupt(ctrller, MMC_I_REG_DATA_TRAN_DONE))) {
        diag_printf("sdio_wait for interrupt failed ... comple_io\n");
        //udelay(500);
        goto error;
    }

    if (rw_flag) {
                /**  wait for PRG_DONE intr */
        if ((ret = sdio_initialize_ireg(ctrller, MMC_I_MASK_PRG_DONE))) {
            diag_printf("<1>drv_init_completion failed" "-complete_io 2\n");
            goto error;
        }
        diag_printf("complete_io waiting for MMC_I_REG_PRG_DONE\n");
        if ((ret = sdio_wait_for_interrupt(ctrller, MMC_I_REG_PRG_DONE))) {
            diag_printf("<1>drv_wait_for_completion complete_io 2\n");
            goto error;
        }

        diag_printf("Got MMC_IREG_PRG_DONE ->4\n");
    }

    set_state(ctrller, SDIO_FSM_CLK_OFF);

    if (set_state(ctrller, SDIO_FSM_IDLE)) {
        diag_printf("<1>__set_state failed -complete_io\n");
        goto error;
    }
    ret = 0;
  error:
    return ret;
}

/** 
	This function fills up the CMD53
	\param pointer to the controller structure and parameters for the CMD53
	\return returns 0 on success or a negative value on error
*/

int
cmd53_reador_write(sdio_ctrller * ctrller, iorw_extended_t * io_rw)
{
    cyg_uint32 cmdat_temp;
    int ret = -ENODEV, ret1 = 0;
    cyg_uint16 argh = 0UL, argl = 0UL;
    ssize_t bytecnt;
    /* CMD53 */

    /* SDIO Spec: 
       R/W flag (1) 
       Function Number (3)
       Block Mode (1)
       OP Code (1) (Multi byte read / write from fixed location or 
       from the fixed location      
       Register Address (17) 
       Byte Count (9)
       Command and the CRC will be taken care by the controller, so 
       fill up (48-16) bits
     */

    argh =
        (io_rw->rw_flag << 15) |
        (io_rw->func_num << 12) |
        (io_rw->blkmode << 11) |
        (io_rw->op_code << 10) | ((io_rw->reg_addr & 0x0001ff80) >> 7);

    argl = ((io_rw->reg_addr & 0x0000007f) << 9) | (io_rw->byte_cnt & 0x1ff);

    if ((ret = stop_clock(ctrller)))
        goto error;

//    MMC_CMD = CMD(53);
//    MMC_ARGH = argh;
//    MMC_ARGL = argl;

    cmdat_temp =
        MMC_CMDAT_R1 | MMC_CMDAT_BLOCK | MMC_CMDAT_DATA_EN |
        (io_rw->rw_flag ? MMC_CMDAT_WR_RD : MMC_CMDAT_READ);

#if 0
    if (ctrller->bus_width == SDIO_BUSWIDTH_4_BIT)
        cmdat_temp |= MMC_CMDAT_SD_4DAT;

    else if (ctrller->bus_width == SDIO_BUSWIDTH_1_BIT)
        cmdat_temp |= MMC_CMDAT_SD_1DAT;

    else
        cmdat_temp |= MMC_CMDAT_SD_4DAT;
#endif
    if (io_rw->blkmode) {
        //MMC_BLKLEN = io_rw->blk_size;
        //MMC_NOB = io_rw->byte_cnt;
    } else {
        //MMC_BLKLEN = io_rw->byte_cnt;
        //MMC_NOB = 1;
    }

    cmdat_temp |= MMC_CMDAT_DMA_EN;
    wmb();

//    MMC_CMDAT = cmdat_temp;

    ctrller->num_ofcmd53++;

    diag_printf("CMD53(0x%04x%04x)\n", argh, argl);

    if ((ret = send_command(ctrller, MMC_R5, 0)))
        goto error;

    if (io_rw->blkmode)
        bytecnt = io_rw->blk_size * io_rw->byte_cnt;
    else
        bytecnt = io_rw->byte_cnt * 1;

    /* Start transferring data on DAT line */
    diag_printf("Byte count = %d\n", bytecnt);
    while (bytecnt > 0) {
        if (io_rw->rw_flag == 0) {
            /* READ */
            diag_printf("read_buffer of %d\n", bytecnt);

            if ((ret = trigger_dma_read(ctrller, bytecnt)) <= 0) {
                diag_printf("<1>HWAC Read buffer error\n");
                ret1 = ret;
                goto error;
            }

            if ((ret = fill_buffer_forread(ctrller, (char*)io_rw->tmpbuf, ret)) < 0)
                goto error;
        } else {
            /* WRITE */
            if ((ret = fill_buffer_forwrite(ctrller,
                                            (char*)io_rw->tmpbuf, bytecnt)) < 0)
                goto error;

            if ((ret = trigger_dma_write(ctrller, ret)) <= 0) {
                diag_printf("HWAC Write buffer error\n");
                goto error;
            }
        }

        io_rw->tmpbuf += ret;
        bytecnt -= ret;
    }

    if (set_state(ctrller, SDIO_FSM_END_IO)) {
        diag_printf("<1>set_state failed rw_iomem\n");
        goto error;
    }

    if ((ret = complete_io(ctrller, io_rw->rw_flag))) {
        diag_printf("<1>complete_io failed rw_iomem\n");
        goto error;
    }

    return 0;
  error:
    if (set_state(ctrller, SDIO_FSM_END_IO)) {
        diag_printf("<1>set_state failed rw_iomem\n");
    }

    if ((ret = complete_io(ctrller, io_rw->rw_flag))) {
        diag_printf("complete_io failed in cmd53_reador_write\n");
    }

    return ret1;
}


/** 
	This function is a hook to the WLAN driver to request the interrupts
	(for Card interrupt)on SDIO card
*/

int
sdio_request_irq(mmc_card_t card,
                 cyg_uint32(*handler) (cyg_vector_t, cyg_addrword_t),
                 unsigned long irq_flags, const char *devname, void *dev_id)
{
    int ret, func;

    cyg_uint8 io_enable_reg, int_enable_reg;

    sdio_ctrller *ctrller = card->ctrlr;

    _ENTER();

    if (!card) {
        diag_printf("Passed a Null card exiting out\n");
        ret = -ENODEV;
        goto done;
    }

    net_devid = dev_id;

    sdioint_handler = handler;

    diag_printf("address of dev_id = %p  sdioint_handler =%p\n",
            net_devid, sdioint_handler);
    if (!ctrller->card_int_ready)
        ctrller->card_int_ready = YES;

    io_enable_reg = 0x0;

    int_enable_reg = IENM;

    for (func = 1; func <= 7 && func <=
         ctrller->card_capability.num_of_io_funcs; func++) {

        io_enable_reg |= IOE(func);
        int_enable_reg |= IEN(func);
    }

        /** Enable function IOs on card */
    sdio_write_ioreg(card, FN0, IO_ENABLE_REG, io_enable_reg);
        /** Enable function interrupts on card */
    sdio_write_ioreg(card, FN0, INT_ENABLE_REG, int_enable_reg);

    //MMC_I_MASK &= ~MMC_I_MASK_SDIO_INT;
    sdio_enable_SDIO_INT();
    wmb();
    diag_printf("sdio_request_irq: MMC_I_MASK = 0x%x\n", MMC_I_MASK);

#ifdef DEBUG_SDIO_LEVEL1
    print_addresses(ctrller);
#endif

    ret = 0;
    _LEAVE();

  done:
    return ret;
}



/** 
	This function is a hook to the WLAN driver to release the interrupts
	requested for
*/

int
sdio_free_irq(mmc_card_t card, void *dev_id)
{
    int ret;
    //sdio_ctrller  *ctrller = card->ctrlr;

    _ENTER();

    dev_id = net_devid;

    if (!card) {
        ret = -ENODEV;
        goto done;
    }

        /** Disable function IOs on the card */
    sdio_write_ioreg(card, FN0, IO_ENABLE_REG, 0x0);

        /** Disable function interrupts on the function */
    sdio_write_ioreg(card, FN0, INT_ENABLE_REG, 0x0);

        /** Release IRQ from OS */

    MMC_I_MASK = MMC_I_MASK_ALL;
    sdioint_handler = NULL;

    _LEAVE();
    return 0;

  done:
    return 0;
}

/** 
	This function checks for the i/o block size on the SDIO card

	\param pointer to controller structure
	\return returns 0 on success, negative value on error
*/

static int
get_ioblk_size(sdio_ctrller * ctrller)
{
    int ret = -1;
    int fn;
    cyg_uint8 dat;

    _ENTER();

    if (!ctrller) {
        diag_printf("Null controller get_ioblk_size\n");
        goto exit;
    }

    if (!ctrller->card)
        diag_printf("ctrller->card is null\n");

    for (fn = 0; fn <= ctrller->card_capability.num_of_io_funcs; fn++) {
        /* Low byte */
        ret = sdio_read_ioreg(ctrller->card, FN0,
                              FN_BLOCK_SIZE_0_REG(fn), &dat);
        /* High byte */
        ret = sdio_read_ioreg(ctrller->card, FN0,
                              FN_BLOCK_SIZE_1_REG(fn), &dat);
    }

    return ret;

  exit:
    diag_printf("Recived a Null controller returning back\n");
    return -1;
}

/**
	This function gets the CIS adress present on the SDIO card

	\param pointer to controller structure
	\return returns 0 on success, negative value on error
*/

static int
get_cisptr_address(sdio_ctrller * ctrller)
{
	int ret, fn;
	cyg_uint8 dat;

	for (fn = 0; fn <= ctrller->card_capability.num_of_io_funcs; fn++)
	{

		ret = sdio_read_ioreg(ctrller->card, FN0, FN_CIS_POINTER_0_REG(fn), &dat);

        ctrller->card_capability.cisptr[fn] = (((cyg_uint32) dat) << 0);

		//printf("dat = %x\n", dat);

		ret = sdio_read_ioreg(ctrller->card, FN0, FN_CIS_POINTER_1_REG(fn), &dat);

        ctrller->card_capability.cisptr[fn] |= (((cyg_uint32) dat) << 8);

		//printf("dat = %x\n", dat);

		ret = sdio_read_ioreg(ctrller->card, FN0, FN_CIS_POINTER_2_REG(fn), &dat);

        ctrller->card_capability.cisptr[fn] |= (((cyg_uint32) dat) << 16);

		//printf("dat = %x\n", dat);

		diag_printf("Card CIS Addr = %x fn = %d\n", ctrller->card_capability.cisptr[fn], fn);

	}

	return 0;
}


/** 
	This function reads the Manufacturing ID and the Vendor ID

	\param pointer to controller structure and function numer
*/

#define	CISTPL_MANFID		0x20
#define	CISTPL_END			0xff

static int
read_manfid(sdio_ctrller * ctrller, int func)
{
	int offset = 0, ret=0;
	cyg_uint32 manfid, card_id;
	cyg_uint8 tuple, link, datah, datal;

	do
	{
		ret = sdio_read_ioreg(ctrller->card, func, ctrller->card_capability.cisptr[func] + offset, &tuple);
		if (ret < 0)
			return ret;

		if (tuple == CISTPL_MANFID)
		{
			offset += 2;

			ret = sdio_read_ioreg(ctrller->card, func, ctrller->card_capability.cisptr[func] + offset, &datal);
			if (ret < 0)
				return ret;

			offset++;

			ret = sdio_read_ioreg(ctrller->card, func, ctrller->card_capability.cisptr[func] + offset, &datah);
			if (ret < 0)
				return ret;

			manfid = datal | datah << 8;

			offset++;

			ret = sdio_read_ioreg(ctrller->card, func,ctrller->card_capability.cisptr[func] + offset, &datal);
			if (ret < 0)
				return ret;

			offset++;

			ret = sdio_read_ioreg(ctrller->card, func, ctrller->card_capability.cisptr[func] + offset, &datah);
			if (ret < 0)
				return ret;

			card_id = datal | datah << 8;

			diag_printf("Card id = 0%2x manfid = %x\n", card_id, manfid);
			return manfid;
		}

		ret = sdio_read_ioreg(ctrller->card, func, ctrller->card_capability.cisptr[func] + offset + 1, &link);
		if (ret < 0)
			return ret;

		offset += link + 2;

	} while (tuple != CISTPL_END);

	return Fail;
}


int
stop_bus_clock_2(sdio_ctrller * ctrller)
{
    stop_clock(ctrller);
    return 0;
}

int
start_bus_clock(sdio_ctrller * ctrller)
{
    start_clock(ctrller);
    return 0;
}

int
sdio_get_vendor_id(mmc_card_t card)
{
    return (read_manfid(card->ctrlr, FN0));

}

/** 
	Read CIS data; This function is used to read the CIS (255 bytes) of
	information and pass it to the user space for further processing
		
	\param pointer to card structure and pointer to the buffer
*/

int
sdio_read_cisinfo(mmc_card_t card, cyg_uint8 * buf)
{

    int i, ret;
    sdio_ctrller *ctrller = card->ctrlr;

    _ENTER();

    /* Read the Tuple Data */
    for (i = 0; i < SIZE_OF_TUPLE; i++) {
        ret = sdio_read_ioreg(card, FN0,
                              ctrller->card_capability.cisptr[FN0] + i,
                              buf + i);
        if (ret < 0)
            return ret;
    }

    _LEAVE();
    return 0;
}

/** 
	This function initiailizes the semaphore 

	\param pointer to controller structure
*/

static void
init_semaphores(sdio_ctrller * ctrller)
{
    //init_MUTEX(&ctrller->io_sem);
    cyg_mutex_init(&ctrller->io_sem);
    return;
}

/** 
	This function initializes the dma controller 

	\param pointer to card structure 
	\return returns 0 on success, negative value on error
*/

static int
init_dma(mmc_card_t card)
{
    int ret = -1, i;
    sdio_ctrller *ctrller = card->ctrlr;
    pxa_dma_desc *desc;         /*/asm/dma.h */

    _ENTER();

    if (!card) {
        diag_printf("Null pointer recieved ....\n");
        return -1;
    }
#if 0
    ctrller->read_desc = DMA_BUF_ALLOC(GFP_KERNEL,
                                       &ctrller->read_desc_phys_addr,
                                       (PXA_MMC_IODATA_SIZE >> 5)
                                       * sizeof(pxa_dma_desc));

    if (!ctrller->read_desc) {
        ret = -ENOMEM;
        goto exit;
    }
    ctrller->write_desc = DMA_BUF_ALLOC(GFP_KERNEL,
                                        &ctrller->write_desc_phys_addr,
                                        (PXA_MMC_IODATA_SIZE >> 5)
                                        * sizeof(pxa_dma_desc));

    if (!ctrller->write_desc) {
        ret = -ENOMEM;
        goto exit;
    }
    //ctrller->phys_addr = SRAM_MEM_PHYS;
    //ctrller->iodata = (void *)io_p2v(ctrller->phys_addr);
    //ctrller->iodata = (void *)0xfe000000;

    ctrller->iodata = DMA_BUF_ALLOC(GFP_ATOMIC,
                                    &ctrller->phys_addr, PXA_MMC_IODATA_SIZE);



    if (!ctrller->iodata) {
        ret = -ENOMEM;
        diag_printf("phys_addr=%#x iodata=%p\n", ctrller->phys_addr,
               ctrller->iodata);
        goto exit;
    }
#endif
    ctrller->blksz = PXA_MMC_BLKSZ_MAX;
    ctrller->bufsz = PXA_MMC_IODATA_SIZE;
    ctrller->nob = PXA_MMC_BLOCKS_PER_BUFFER;

    /* request DMA channel */
    if ((ctrller->chan = pxa_request_dma("MMC", DMA_PRIO_LOW,
                                         dma_irq, ctrller)) < 0) {
        diag_printf("failed to request DMA channel\n");
        goto exit;
    }
    diag_printf("Requested DMA Channel = %d\n", ctrller->chan);

//    DRCMRRXMMC = ctrller->chan | DRCMR_MAPVLD;
//    DRCMRTXMMC = ctrller->chan | DRCMR_MAPVLD;

#if 0
    for (i = 0; i < ((PXA_MMC_IODATA_SIZE >> 5) - 1); i++) {
        desc = &ctrller->read_desc[i];
        desc->ddadr = ctrller->read_desc_phys_addr
            + ((i + 1) * sizeof(pxa_dma_desc));
        desc->dsadr = MMC_RXFIFO_PHYS_ADDR;
        desc->dtadr = ctrller->phys_addr + (i << 5);
        desc->dcmd = DCMD_FLOWSRC | DCMD_INCTRGADDR
            | DCMD_WIDTH1 | DCMD_BURST32 | (1 << 5);

        desc = &ctrller->write_desc[i];
        desc->ddadr = ctrller->write_desc_phys_addr
            + ((i + 1) * sizeof(pxa_dma_desc));
        desc->dsadr = ctrller->phys_addr + (i << 5);
        desc->dtadr = MMC_TXFIFO_PHYS_ADDR;
        desc->dcmd = DCMD_FLOWTRG | DCMD_INCSRCADDR
            | DCMD_WIDTH1 | DCMD_BURST32 | (1 << 5);
    }
    desc = &ctrller->read_desc[i];
    desc->ddadr = (ctrller->read_desc_phys_addr +
                   (i + 1) * sizeof(pxa_dma_desc)) | DDADR_STOP;
    desc->dsadr = MMC_RXFIFO_PHYS_ADDR;
    desc->dtadr = ctrller->phys_addr + (i << 5);
    desc->dcmd = DCMD_FLOWSRC | DCMD_INCTRGADDR | DCMD_WIDTH1
        | DCMD_BURST32 | (1 << 5);

    desc = &ctrller->write_desc[i];
    desc->ddadr = (ctrller->write_desc_phys_addr +
                   (i + 1) * sizeof(pxa_dma_desc)) | DDADR_STOP;
    desc->dsadr = ctrller->phys_addr + (i << 5);
    desc->dtadr = MMC_TXFIFO_PHYS_ADDR;
    desc->dcmd = DCMD_FLOWTRG | DCMD_INCSRCADDR | DCMD_WIDTH1
        | DCMD_BURST32 | (1 << 5);
  #endif
    diag_printf("\n SDIO: DMA Initialization done....\n");

    diag_printf("\n SDIO: DMA Channel Number is %d\n", ctrller->chan);
    _LEAVE();
    return 0;

  exit:
    diag_printf("Error in init_dma\n");
    cleanup_dma_allocated_memory(ctrller);
    return -1;
}

#ifdef CONFIG_HOTPLUG1
/** 
	This function cleans up the allocated memory and other resources 
	allocated when the card is plugged out
*/

static void
remove_hotplug(sdio_ctrller * ctrller)
{
    mmc_card_t card = ctrller->card;

    if (ctrller->dma_init == YES) {
        diag_printf("Dma allocated is true\n");
        cleanup_dma_allocated_memory(ctrller);
        ctrller->dma_init = NO;
    }

    sdio_write_ioreg(ctrller->card, FN0, 0x06, 0x08);

    memset(ctrller, 0, sizeof(sdio_ctrller) + hotplug_extra);
    ctrller->card = card;
}

/** 
	This function calls /sbin/hotplug and a user level script when the card
	is inserted
*/

static void
run_sbin_hotplug(sdio_ctrller * ctrller)
{

    int i, ret;
    char *argv[3], *envp[8];
    char *buf = NULL, *scratch;

    if (!hotplug_path[0])
        return;

    diag_printf(": hotplug_path=%s insert=%d\n", hotplug_path, ctrller->slot);

    i = 0;
    argv[i++] = hotplug_path;
    argv[i++] = "sdio";
    argv[i] = 0;

    /* minimal command environment */
    i = 0;
    envp[i++] = "HOME=/";
    envp[i++] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";

    MALLOC(buf, (char *), 256, 0, M_NOWAIT);
    if(!buf)
    {
        diag_printf("Unable to allocate the memory to buf\n");
        return;
    }

    /* other stuff we want to pass to /sbin/hotplug */

    /*
     * 1. Initialize the card 
     * 2. Read the Vendor ID
     * 3. Read the supported functionaleties in the card
     * 4. Pass these parameters to the user space
     */

    if (ctrller->slot) {
        ret = send_iosend_op_cmd(ctrller);

        if (ret < 0) {
            diag_printf("Unable to get the command 5 response\n");
            goto cleanup_hotplug;
        }

        /*TODO SDIO Spec: Continuosly send CMD3 and CMD7 until card 
         * is put in to standby mode 
         */

        /* CMD 3 */

        ret = send_relative_addr_cmd(ctrller);

        if (ret < 0) {
            diag_printf("Unable to get the Response for CMD3\n");
            goto cleanup_hotplug;
        }

        /* CMD 7 */
        ret = send_select_card_cmd(ctrller);

        if (ret < 0) {
            diag_printf("Unable to get the Response for CMD 7\n");
            goto cleanup_hotplug;
        }

        ret = send_select_card_cmd(ctrller);

        if (ret < 0) {
            diag_printf("Unable to get the Response for CMD 7\n");
            goto cleanup_hotplug;
        }

        init_semaphores(ctrller);

        /* Now read the regesters CMD52 */

        /* Get the function block size
         * Get the CIS pointer base
         * Parse the tuple
         * Get the Vendor ID compare with Marvell ID
         * End of Ctrller initialization
         */

        /* Now the card is initialized. Set the clk to 10/20 Mhz */
        ret = get_ioblk_size(ctrller);

        if (ret < 0) {
            diag_printf("get_ioblk_size failed\n");
            goto cleanup_hotplug;
        }

        sdio_set_clkrate(ctrller->card, clkrate);

        sdio_set_buswidth(ctrller->card, mode);

        ret = get_cisptr_address(ctrller);

        if (ret < 0) {
            diag_printf("get_cisptr_address failed\n");
            goto cleanup_hotplug;
        }

        ret = read_manfid(ctrller, FN0);

        if (ret < 0) {
            diag_printf("Unable to read the manf or vendor id\n");
            goto cleanup_hotplug;
        }

        diag_printf("Manf id	=		 %x\n", ctrller->manf_id);
        diag_printf("Device id = 		 %x\n", ctrller->dev_id);

        if (ctrller->manf_id == MRVL_MANFID) {
            diag_printf("Found the MARVELL SDIO (0x%x) card\n", ret);
            diag_printf("SDIO Driver insertion " "Successful\n");
        }

        /* Stop the clock and put it to IDLE state */
        ret = stop_clock(ctrller);

        if (ret < 0)
            diag_printf("Unable to stop the timer\n");

        set_state(ctrller, SDIO_FSM_IDLE);

        scratch = buf;

        envp[i++] = "ACTION=add";

        envp[i++] = scratch;

        scratch += sprintf(scratch, "VENDOR_ID=%06x", ctrller->manf_id) + 1;

        envp[i++] = scratch;
        scratch += sprintf(scratch, "DEVICE_ID=%06x", ctrller->dev_id) + 1;
        envp[i] = 0;
    } else {
        envp[i++] = "ACTION=remove";
        envp[i] = 0;
        remove_hotplug(ctrller);
    }

/* 
	This function actually pushes the envoirnment variables for
	the bash shell
*/
    ret = call_usermodehelper(argv[0], argv, envp);

    FREE(buf, 0);

    diag_printf("After calling usermodehelper fun %d;path =%s\n", ret, argv[0]);

    return;

  cleanup_hotplug:
    if (buf)
        FREE(buf, 0);
    return;
}

/** 
	Handler for hotplug
*/

static void
sdio_hotplug_handler(void *ctrller)
{
    _DBGMSG("sdio_hotplug_handler ptr = %p\n", ctrller);
    run_sbin_hotplug(ctrller);
    return;
}

        /* Calling the user mode program should be done in the process
         * Context , hence a task que...
         */

static struct tq_struct sdio_hotplug = {
  routine:sdio_hotplug_handler
};

static void
sdio_hotplug_helper(void *ctrller)
{
    _DBGMSG("sdio_hotplug_helper ctrller = %p\n", ctrller);
    sdio_hotplug.data = ctrller;
    schedule_task(&sdio_hotplug);
    return;
}

/** 
	This function checks the status of the card Inserted / removed
*/

static int
mmc_slot_is_empty(int slot)
{
    /* Check the bits; which tells the status of the 
     * driver insertion / removal
     * value -> 0 Card inserted ; >0 Card removal
     */
    return (MST_MSCRD & MST_MSCRD_nMMC_CD);
}

/** 
	This function is called from the task context and checks for the status
	of the card and takes the necessary action depending on the status
*/

static void
sdio_detect_card_timer_fun(unsigned long val)
{
    int empty;

    sdio_ctrller *ctrller = (sdio_ctrller *) val;

    _DBGMSG("sdio_detect_card: ptr = %p\n", ctrller);

    empty = mmc_slot_is_empty(0);

    if (empty == 128) {
        _ERROR("No card in slot\n");

                /**Clean up the ctrller structure and be ready for 
		 * insertion
		 */
        slot_state = MMC_CARD_REMOVED;
        ctrller->slot = MMC_CARD_REMOVED;
        sdio_hotplug_helper(ctrller);
    } else if (empty == 0) {
        _PRINTK("Found the card in slot\n");
        slot_state = MMC_CARD_INSERTED;
        ctrller->slot = MMC_CARD_INSERTED;
        sdio_hotplug_helper(ctrller);
    }
}

/** 
	Interrupt service routine for hotplug
*/

static void
sdio_card_isr(int irq, void *dev, struct pt_regs *regs)
{
    /* Directly running the module of the interrupt context 
     * is not setting the bits in MMC_REG .. So a delay :)
     */
    _DBGMSG("sdio_card_isr ptr value = %p\n", dev);
    sdio_detect_card_thr_timer.data = (cyg_uint32) dev;
    mod_timer(&sdio_detect_card_thr_timer, HZ);
}

/** 
	Initialization for the hotplug
*/

static int
init_hotplug(sdio_ctrller * ctrller)
{
/** sdio_detect:
 * Used to detect the automatic insertion and removal of the card.
 * This module also helps the modules to be hotpluggable.
 * Theory: When the card is inserted, there is a card detect resistor
 * on the Bulverede, which sends an interrupt.
 * Based on the register values , the module detects the insertion and removal
 * and informs the kevnetd with the required parameters. 
 * The keventd creates a child thread and calls the hotplug routines.
 */
    int ret;

    init_timer(&sdio_detect_card_thr_timer);

    sdio_detect_card_thr_timer.function = sdio_detect_card_timer_fun;

    ret = request_irq(MAINSTONE_MMC_IRQ, sdio_card_isr,
                      SA_INTERRUPT, "SDIO Card Detect", ctrller);

    /* let the initial state be unplugged */
    slot_state = MMC_CARD_REMOVED;

    if (ret < 0)
        diag_printf("Unable to request the IRQ\n");

    return ret;
}
#endif

/**
	This Function will send out CMD5;CMD7 to check if there are
	Any cards present on the host
*/

static int
init_host_controller(sdio_ctrller * ctrller)
{
    int ret = -1;
#if 0
    ret = setup_gpio_lines();

    if (ret < -1) {
        diag_printf("Unable to get the GPIO Lines requested\n");
        goto exit;
    }
#endif
    //enable_clock();

    ret = request_interrupt(ctrller);

    if (ret < 0) {
        diag_printf("Unable to request the interrupt from the system\n");
        goto exit;
    }

        /** Send Command 5, 3, 7 Check for the Vendor ID, if not 
	 * Marvell Silently get out from here 
	 */
#ifdef CONFIG_HOTPLUG1

    /* This flag is used to indicate that no one has requested
     * DMA. Ideally the DMA is needed for CMD53 , so wait till somebody
     * makes a call to CMD53 read or write
     */
    ctrller->dma_init = NO;

    ret = init_hotplug(ctrller);
#else

#if 0
    ret = send_iosend_op_cmd(ctrller);

    /* Ret = 0 Means CMD5 Successful
     */

    if (ret < 0) {
        diag_printf("Unable to get the Response for CMD5 Bailing out\n");
        goto exit;
    }

    /* CMD 3 */
    ret = send_relative_addr_cmd(ctrller);

    if (ret < 0) {
        diag_printf("Unable to get the Response for CMD3\n");
        goto exit;
    }

    /* CMD 7 */
    ret = send_select_card_cmd(ctrller);

    if (ret < 0) {
        diag_printf("Unable to get the Response for CMD 7\n");
        goto exit;
    }

    /* Send the correct rca and set the proper state */

    ret = send_select_card_cmd(ctrller);

    if (ret < 0) {
        diag_printf("Unable to get the Response for CMD 7\n");
        goto exit;
    }
#endif
    init_semaphores(ctrller);

    /* Now read the regesters CMD52 */

    /* Get the function block size
     * Get the CIS pointer base
     * Parse the tuple
     * Get the Vendor ID compare with Marvell ID
     * End of Ctrller initialization
     */

    /* Now the card is initialized. Lets set the clk to 10/20 Mhz */
#if 0
    ret = get_ioblk_size(ctrller);

    if (ret < 0) {
        diag_printf("get_ioblk_size failed\n");
        goto exit;
    }
#endif
    ret = get_cisptr_address(ctrller);

    if (ret < 0) {
        diag_printf("get_cisptr_address failed\n");
        goto exit;
    }
#if 0
    ret = read_manfid(ctrller, FN0);

    diag_printf(" Read Manf id = %x\n", ret);

    if (ret == MRVL_MANFID)
        diag_printf("\n Found Marvell SDIO Card (0x%04x) "
                "Initializing the driver\n", ret);
    /* Starting the DMA Channel */
#endif
    ctrller->dma_init = YES;

    //init_dma(ctrller->card);

    /* Now Wait for the Wlan driver to do any operations
     */

    /* Ideallly set by the Wlan driver default lets set to
     * 10 MHZ and 1 bit mode */

//    sdio_set_clkrate(ctrller->card, clkrate);

//    sdio_set_buswidth(ctrller->card, mode);
#if 0
    stop_clock(ctrller);
#endif
    set_state(ctrller, SDIO_FSM_IDLE);

#endif
    return 0;

    /* Clean up */

  exit:
    diag_printf("init_host_ctrller fails\n");

    //free_irq(IRQ_FMI, ctrller);
    //cyg_interrupt_detach(sdioIntrHandle);
    //cyg_interrupt_delete(sdioIntrHandle);

    if (ctrller->card)
        FREE(ctrller->card, 0);

    if (ctrller->tmpl)
        FREE(ctrller->tmpl, 0);

    if (ctrller)
        FREE(ctrller, 0);
    /* Gracefully exit from here */

    return -1;
}

/** 
	This function cleans up the cllocated DMA resources from the OS
	
	\param pointer to sdio controller structure
	\return returns none
*/

static void
cleanup_dma_allocated_memory(sdio_ctrller * ctrller)
{
    if (ctrller->chan >= 0) {
//        DRCMRRXMMC = 0;
//        DRCMRTXMMC = 0;
        pxa_free_dma(ctrller->chan);
    }

    if (ctrller->iodata) {
#if 0
        DMA_BUF_FREE(ctrller->iodata,
                     ctrller->phys_addr, PXA_MMC_IODATA_SIZE);
#else
		FREE(ctrller->iodata, 0);
#endif
        ctrller->iodata = 0;
        //ctrller->phys_addr = 0;
    }
#if 0
    if (ctrller->read_desc)
        DMA_BUF_FREE(ctrller->read_desc,
                     ctrller->read_desc_phys_addr,
                     (PXA_MMC_IODATA_SIZE >> 5) * sizeof(pxa_dma_desc));

    if (ctrller->write_desc)
        DMA_BUF_FREE(ctrller->write_desc,
                     ctrller->write_desc_phys_addr,
                     (PXA_MMC_IODATA_SIZE >> 5) * sizeof(pxa_dma_desc));
#endif
}

/*
	One Global variable for initialization and
	Removal of the card. Use with care :)
 */

//static sdio_ctrller *sdio_host;

/** 
	This function registers the Card as the host , allocates the memory
	for the structure 
*/

#ifdef DEBUG_SDIO_LEVEL1
void
print_addresses(sdio_ctrller * ctrller)
{
    diag_printf("ctrller = 0x%p\n", ctrller);
    diag_printf("ctrller.card = 0x%p\n", ctrller->card);
    diag_printf("ctrller.tmpl = 0x%p\n", ctrller->tmpl);
    diag_printf("ctrller.tmpl.irq_line = %d\n", ctrller->tmpl->irq_line);
}
#endif

static void *
register_sdiohost(void *ops, int extra)
{
    sdio_ctrller *ctrller;
    mmc_card_t card;

    _ENTER();

    /* - Initialize the SDIO Controller 
     * Allocate memory to the card structure
     * Request for a DMA channel
     * Request for the IRQ
     */

    MALLOC(ctrller, sdio_ctrller *, sizeof(sdio_ctrller) + extra, 0, M_NOWAIT);

    if (!ctrller) {
        diag_printf("Kmalloc failed bailing out ...\n");
        goto out;
    }
	ctrller = (sdio_ctrller *)((unsigned int)ctrller | NON_CACHE_FLAG);
    memset(ctrller, 0, sizeof(sdio_ctrller) + extra);

#ifdef CONFIG_HOTPLUG1
    hotplug_extra = extra;
#endif

    MALLOC(card, mmc_card_t, sizeof(mmc_card_rec), 0, M_NOWAIT);
    if(!card)
    {
        diag_printf("Cannot allocate memory for card!\n");
        goto out;
    }
	card = (mmc_card_t)((unsigned int)card | NON_CACHE_FLAG);
    card->ctrlr = ctrller;      /* Back Reference */

    ctrller->card = card;

    MALLOC(ctrller->tmpl, struct _dummy_tmpl *, sizeof(dummy_tmpl), 0, M_NOWAIT);

    if (!ctrller->tmpl) {
        diag_printf("Kmalloc failed bailing out ...\n");
        goto out;
    }
    ctrller->tmpl = (struct _dummy_tmpl *)((unsigned int)ctrller->tmpl | NON_CACHE_FLAG);
#ifdef DEBUG_SDIO_LEVEL1
    print_addresses(ctrller);
#endif
	
    if ((init_host_controller(ctrller)) < 0) {
        diag_printf("init_host_controller failed \n");
        goto out;
    }

    return ctrller;

  out:
    if (ctrller->card)
        FREE(ctrller->card, 0);

    if (ctrller->tmpl)
        FREE(ctrller->tmpl, 0);

    if (ctrller)
        FREE(ctrller, 0);

    return NULL;
}

/**
	Registers the WLAN driver and makes a call back function to the WLAN 
	driver's entry point

	\param pointer to the function passed by the wlan layer, pointer to the 
	controller structure
*/

static void *
register_sdiouser(mmc_notifier_rec_t * ptr, sdio_ctrller * ctrller)
{
    /* Increment the count */
 //   MODULE_GET;

    if (!ptr) {
        diag_printf("null func pter\n");
    }

    if (!ptr->add || !ptr->remove) {
        diag_printf("No function pointers\n");
    }

    diag_printf("Value of the sdio_ctrller = %p\n", ctrller);

    if (!ctrller) {
        diag_printf("Null ctrller\n");
        goto exit;

    }
    diag_printf("Value of card = %p\n", ctrller->card);

    if (ptr->add(ctrller->card)) {
        diag_printf("add_card failed\n");
        goto exit;
    }

    /* Initialize the DMA channel .. By now we know that it is the 
     * Marvell SD Card */
    diag_printf("returning the func pter value =%p\n", ptr);

    return ptr;
  exit:
    diag_printf("Null controller or null funcptr\n");
//    MODULE_PUT;
    return 0;
}

/** 
	This function clean up the host controller and release the resources
	allocated by the OS

	\param pointer to controller structure
	\return returns 0 on success , negative value on error
*/

static int
cleanup_host_controller(sdio_ctrller * ctrller)
{
    _ENTER();

    if (!ctrller) {
        diag_printf("Bad ctrller\n");
        return -1;
    }

    /* For Cold restart */
    sdio_write_ioreg(ctrller->card, FN0, 0x06, 0x08);

#if 0
    /* free buffer(s) */
    if (ctrller->dma_init == YES) {

        if (ctrller->iodata) {
            DMA_BUF_FREE(ctrller->iodata,
                         ctrller->phys_addr, PXA_MMC_IODATA_SIZE);
            ctrller->iodata = 0;
            ctrller->phys_addr = 0;
        }

        if (ctrller->read_desc)
            DMA_BUF_FREE(ctrller->read_desc,
                         ctrller->read_desc_phys_addr,
                         (PXA_MMC_IODATA_SIZE >> 5) * sizeof(pxa_dma_desc));

        if (ctrller->write_desc)
            DMA_BUF_FREE(ctrller->write_desc,
                         ctrller->write_desc_phys_addr,
                         (PXA_MMC_IODATA_SIZE >> 5) * sizeof(pxa_dma_desc));

        /* release DMA channel */

        diag_printf("crash after consistent_free \n");

        if (ctrller->chan >= 0) {
            DRCMRRXMMC = 0;
            DRCMRTXMMC = 0;
            pxa_free_dma(ctrller->chan);
        }
    }
#endif
    disable_clock();

#ifdef CONFIG_HOTPLUG1
    free_irq(MAINSTONE_MMC_IRQ, sdio_host);
#endif
//    free_irq(IRQ_FMI, ctrller);

    if (ctrller->card)
        FREE(ctrller->card, 0);

    if (ctrller->tmpl)
        FREE(ctrller->tmpl, 0);

    if (ctrller)
        FREE(ctrller, 0);

    _LEAVE();
    return 0;
}

/** 
	Function holder
*/

sdio_operations sops = {
  "SDIO DRIVER",
    /* RFU */
};

/** 
	This function registers the card as HOST/USER(WLAN)

	\param type of the card, function pointer
	\return returns pointer to the type requested for
*/

void *
sdio_register(int type, void *ops, int memory)
{
    void *ret = NULL;

    _ENTER();

    switch (type) {

    case MMC_TYPE_HOST:
        ret = register_sdiohost(ops, memory);

        if (!ret) {
            diag_printf("sdio_register failed\n");
            return NULL;
        }

        diag_printf("register_sdio_host val of ret = %p\n", ret);
        break;

    case MMC_REG_TYPE_USER:

        /* There can be multiple regestrations
         * Take care of serializing this
         */
        ret = register_sdiouser(ops, sdio_host);
        break;

    default:
        break;
    }

    _LEAVE();
    return ret;
}

/** 
	This function unregisters the WLAN driver

	\param Function pointer passed by the WLAN driver
*/

static int
unregister_sdiouser(mmc_notifier_rec_t * ptr)
{
    _ENTER();

        /** Decrement the count so that the Wlan interface can be 
	 * detached
	 */
    //MODULE_PUT;

    if (!ptr) {
        diag_printf("Null pointer passed...\n");
        goto exit;
    }

    ptr->remove(sdio_host->card);

    _LEAVE();
    return 0;
  exit:
    diag_printf("unregister_sdiouser recieved null function pointer\n");
    return -1;
}

/** 
	This function is a hook to the WLAN driver to unregister
	
	\param type to be registered, function pointer
*/

int
sdio_unregister(int type, void *ops)
{
    switch (type) {
    case MMC_TYPE_HOST:
        break;
    case MMC_REG_TYPE_USER:
        unregister_sdiouser(ops);
        break;

    default:
        break;
    }

    return 0;
}

/** 
	This function is used to register the SDIO card as host
	
	\param none
*/

static int
init_controller(cyg_uint8 io_func)
{

    _ENTER();

#ifdef _MAINSTONE
    _DBGMSG("MST_MSCWR1=%lx", MST_MSCWR1);
    /* Turn on power for MMC controller */
    MST_MSCWR1 |= MST_MSCWR1_MMC_ON;
    /* Signals are routed to MMC controller */
    MST_MSCWR1 &= ~MST_MSCWR1_MS_SEL;
#endif

    sdio_host = sdio_register(MMC_TYPE_HOST, &sops, 0);

    if (!sdio_host) {
        diag_printf("SDIO Registration failed\n");
        return -1;
    }
    
    sdio_host->card_capability.num_of_io_funcs = io_func;
    
#ifdef _MAINSTONE_GPIO_IRQ
    //init_gpio_irq(sdio_host);
#endif

    _LEAVE();
    return 0;
}

/** 
	This funciton is used to clean up the controller structure
	
	\param none
*/

static void
cleanup_controller(void)
{
    _ENTER();

#ifdef _MAINSTONE
    /* Turn off power for MMC controller */
    MST_MSCWR1 &= ~MST_MSCWR1_MMC_ON;
#endif
    cleanup_host_controller(sdio_host);

    _LEAVE();
}

#if 0
/** 
	Dummy function holders RFU
*/

int
sdio_open(struct inode *in, struct file *file)
{
  //  MODULE_GET;

    return 0;
}

int
sdio_release(struct inode *in, struct file *file)
{
    _ENTER();

   // MODULE_PUT;

    unregister_chrdev(MAJOR_NUMBER, "sdio");

    _LEAVE();
    return 0;
}

ssize_t
sdio_write(struct file * file, const char *buf, size_t size, loff_t * ptr)
{
    return 0;
}

ssize_t
sdio_read(struct file * file, char *buf, size_t size, loff_t * ptr)
{
    return 0;
}

int
sdio_ioctl(struct inode *in, struct file *file, uint val, ulong val1)
{
    return 0;
}

struct file_operations sdio_fops = {
  owner:THIS_MODULE,
  open:sdio_open,
  release:sdio_release,
  read:sdio_read,
  write:sdio_write,
  ioctl:sdio_ioctl,
  llseek:NULL,
};
#endif

/** 
	This function register the SDIO driver as a charecter driver with the OS
	
	\param none
*/

static int
register_sdio_driver(void)
{
#if 0
    if (register_chrdev(MAJOR_NUMBER, "sdio", &sdio_fops) < 0) {
        diag_printf("Unable to register the sdio driver as a char " "dev\n");
        return -1;
    }
#endif
    return 0;
}

/** 
	This function checks the state  returns 0 on success negative value on 
	error
	
	\param pointer to controller structure and the state to be chcked
	\return returns 0 on success, negative value on error
*/

static int
check_ctrller_state(sdio_ctrller * ctrller, int state)
{
    if ((ctrller->state != state))
        return -1;
    else
        return 0;
}

/** 
	This function sets the state of the contrller to the desired state
	
	\param pointer to controller structure, and the seired state
*/

static int
set_state(sdio_ctrller * ctrller, int state)
{
    if (state == SDIO_FSM_IDLE) {
//        MMC_I_MASK = ~MMC_I_MASK_SDIO_INT;
		sdio_enable_SDIO_INT();

        wmb();
    }

    ctrller->state = state;

    return 0;
}

/** 
	This function starts the SDIO clock

	\param pointer to sdio controller structure
	\return returns 0 
*/

int
start_clock(sdio_ctrller * ctrller)
{
    /* - PXA Doc: Write 0x02 to MMC_STRPCL to start the clock
     */
    _ENTER();

#if 0
    if ((MMC_STAT & MMC_STAT_CLK_EN)) {
        diag_printf("Clock is already on, returning\n");
        return 0;
    }
    diag_printf("*Starting the clock manually\n");

    MMC_STRPCL = MMC_STRPCL_START_CLK;
    wmb();
#endif
    _LEAVE();
    return 0;
}

int
sdio_suspend(mmc_card_t card)
{
    sdio_ctrller *ctrller = card->ctrlr;

    _ENTER();

    /* Save the regesters and the mode GPIO mode */
#if 0
    ctrller->saved_mmc_clkrt = MMC_CLKRT;
    ctrller->saved_mmc_resto = MMC_RESTO;
    ctrller->saved_mmc_i_mask = MMC_I_MASK;
    ctrller->saved_mmc_spi = MMC_SPI;
    ctrller->saved_drcmrrxmmc = DRCMRRXMMC;
    ctrller->saved_drcmrtxmmc = DRCMRTXMMC;
    ctrller->suspended = TRUE;

    CKEN &= ~CKEN12_MMC;        /* disable MMC unit clock */
    wmb();
#endif
   // SET_PXA_GPIO_MODE(32);

    _LEAVE();
    return 0;
}

int
sdio_resume(mmc_card_t card)
{
    sdio_ctrller *ctrller = card->ctrlr;

    _ENTER();

    diag_printf("Kernel Call Back Calling SD-Resume\n");

    if (ctrller->suspended == TRUE) {
        /* restore registers */
//        MMC_I_MASK = MMC_I_MASK_ALL;
//        wmb();
//        SET_PXA_GPIO_MODE(32 | GPIO_ALT_FN_2_OUT);
        /* Configure MMCMD command/response bidirectional signal
           ([2], 15.3, 24.4.2) */
//        SET_PXA_GPIO_MODE(112 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);

        /* Configure MMDAT[0123] data bidirectional signals
           ([2], 15.3, 24.4.2) */
//        SET_PXA_GPIO_MODE(92 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
//        SET_PXA_GPIO_MODE(109 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
//        SET_PXA_GPIO_MODE(110 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
//        SET_PXA_GPIO_MODE(111 | GPIO_ALT_FN_1_IN | GPIO_ALT_FN_1_OUT);
//        wmb();

//        CKEN |= CKEN12_MMC;     /* enable MMC unit clock */
//        wmb();
 //       MMC_I_MASK = MMC_I_MASK_ALL;
//        wmb();

 //       MMC_CLKRT = ctrller->saved_mmc_clkrt;
 //       wmb();
 //       MMC_RESTO = ctrller->saved_mmc_resto;
 //       wmb();
//        MMC_SPI = ctrller->saved_mmc_spi;
 //       wmb();
//        DRCMRRXMMC = ctrller->saved_drcmrrxmmc;
//        DRCMRTXMMC = ctrller->saved_drcmrtxmmc;
//        wmb();

 //       MMC_I_MASK = ctrller->saved_mmc_i_mask & (~MMC_I_MASK_SDIO_INT);
 //       MMC_I_MASK |= MMC_I_MASK_RES_ERR | MMC_I_MASK_END_CMD_RES;
 //			sdio_enable_SDIO_INT();
//        wmb();

//        MMC_CMDAT = MMC_CMDAT_SDIO_INT_EN;
//        wmb();

 //       MMC_CMD = CMD(0);
//        wmb();

//        mdelay(1);

 //       MMC_I_MASK = ctrller->saved_mmc_i_mask & (~MMC_I_MASK_SDIO_INT);
 			sdio_enable_SDIO_INT();
        wmb();
        ctrller->suspended = FALSE;
    }

    diag_printf("Got out of SD-Resume\n");

    _LEAVE();
    return 0;
}

/* Dummy holders for driver integration */

int
sdio_clear_imask(sdio_ctrller * ctrller)
{
    return 0;
}

void
sdio_enable_SDIO_INT(void)
{
	outpw(REG_SDIER, inpw(REG_SDIER) | 0x10);
    return;
}

void
sdio_disable_SDIO_INT(void)
{
	outpw(REG_SDIER, inpw(REG_SDIER) & ~0x10);
    return;
}

cyg_uint8 sdio_int_enable_reg;
int disable_sdio_int(void)
{
    mmc_card_t card;
#if 1
    sdio_read_ioreg(card, FN0, INT_ENABLE_REG, &sdio_int_enable_reg);

    /** Disable function interrupts on the function */
    sdio_write_ioreg(card, FN0, INT_ENABLE_REG, 0x0);
#else
	
	int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 host_int_mask;
	
	//ret = sdio_read_ioreg(card, FN1, HOST_INTSTATUS_REG, &host_int_mask);	
	//diag_printf("*********01intstatus = %x\n",ret);
	
    /* Read back the host_int_mask register */
    ret = sdio_read_ioreg(card, FN1, HOST_INT_MASK_REG,
                          &host_int_mask);
    if (ret < 0) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
	
	sdio_int_enable_reg = host_int_mask;
    /* Update with the mask and write back to the register */
    host_int_mask &= ~HIM_DISABLE;
    ret = sdio_write_ioreg(card, FN1, HOST_INT_MASK_REG,
                           host_int_mask);
    if (ret < 0) {
        diag_printf("Unable to diable the host interrupt!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }
    
    //ret = sdio_read_ioreg(card, FN1, HOST_INTSTATUS_REG, &host_int_mask);
    //diag_printf("*********02intstatus = %x\n",ret);

  done:
    return ret;
	
#endif
}

int enable_sdio_int(void)
{
    mmc_card_t card;

#if 1
    /** Enable function interrupts on card */
    sdio_write_ioreg(card, FN0, INT_ENABLE_REG, sdio_int_enable_reg);
#else
	
	int ret = WLAN_STATUS_SUCCESS;

    /* Simply write the mask to the register */
    ret = sdio_write_ioreg(card, FN1, HOST_INT_MASK_REG, sdio_int_enable_reg);//HIM_ENABLE

    if (ret < 0) {
        diag_printf("ret = %d\n", ret);
        ret = WLAN_STATUS_FAILURE;
    }
	
	return ret;
    //priv->adapter->HisRegCpy = 1;

	
#endif
}



int
sdio_check_idle_state(mmc_controller_t ctrlr)
{
    return 0;
}


void
sdio_print_imask(sdio_ctrller * ctrller)
{
    return;
}

/*#ifdef DEBUG_SDIO_LEVEL1
static void HexDump(char *prompt, cyg_uint8* buf, int len)
{
	int i = 0;

  	_PRINTK("%s: ", prompt);
	for (i = 1; i <= len; i ++)
		_PRINTK("%02x ", *buf++);

	_PRINTK("\n");
}
#endif*/

/** 
	Module entry , Kernel entry point
*/

int coremodule_init(cyg_uint8  io_func)
{
    int ret = -1;

    ret = init_controller(io_func);

    if (ret < 0) {
        diag_printf("Init controller failed\n");
        goto exit;
    }

    ret = register_sdio_driver();

    if (ret < 0) {
        diag_printf("Unable to Install SDIO Driver\n");
        goto exit;
    }

  exit:
    diag_printf("Unable to register with the O.S\n");
    return ret;

}

/** 
	Module Exit 
*/

static void 
coremodule_exit(void)
{
    _ENTER();

    diag_printf(" Passing the ctrller address = %p\n", sdio_host);

#ifdef _MAINSTONE_GPIO_IRQ
    if (sdio_host)
        remove_gpio_irq(sdio_host);
#endif

    cleanup_controller();

    unregister_chrdev(MAJOR_NUMBER, "sdio");

    _LEAVE();
}
//#endif

typedef struct {
	unsigned int clkcon;
	unsigned int apllcon;
	unsigned int upllcon;
	unsigned int clksel;
	unsigned int clkdiv0;
	unsigned int clkdiv1;
	unsigned int sdcon;
	unsigned int sdtime0;
} w99702_clk_t;


#define DELAY_LOOPS	0x400
#define DELAY(x)	{volatile int delay;delay = (x); while(delay--);}

static w99702_clk_t clk_profile[] = {

	/* Clock setting profile of menu operation mode */
#if CONFIG_MEMORY_SIZE == 16
	{0x0F003094, 0x0000E220, 0x00004212, 0x0000030E, 0x00400303, 0x0010A9AA, 0x130080FF, 0xC0005748},//108M
	{0x0F003094, 0x0000E220, 0x0000442d, 0x0000030E, 0x00400303, 0x0010A9AA, 0x130080FF, 0xC0005748},//135M
	{0x0F003094, 0x0000E220, 0x00006212, 0x0000030E, 0x00400300, 0x0000A9AA, 0x130080FF, 0xC0005748},//54M
#elif CONFIG_MEMORY_SIZE == 8
	{0x0F003094, 0x0000E220, 0x00004212, 0x0000030E, 0x00400303, 0x0010A9AA, 0x130080FF, 0xF8005748},//108M
	{0x0F003094, 0x0000E220, 0x0000442d, 0x0000030E, 0x00400303, 0x0010A9AA, 0x130080FF, 0xF8005748},//135M
	{0x0F003094, 0x0000E220, 0x00006212, 0x0000030E, 0x00400300, 0x0000A9AA, 0x130080FF, 0xF8005748},//54M
#else
#	error "No memory size defined"
#endif

};

static void pwr_set_clk(w99702_clk_t *cfg)
{
	unsigned int reg,bak,bak_audio, bak_mp4dec;
	unsigned int clkcon;
	
	/* if UPLL is enabled, switch the PLL to APLL first */
	bak = 0; bak_audio = 0; bak_mp4dec = 0;
	reg = inpw(REG_CLKSEL);
	
	/* Reserve the audio clock */	
	outpw(REG_APLLCON, 0x6210);

	if( 	inpw(REG_UPLLCON) == cfg->upllcon && 
			(inpw(REG_CLKDIV0)&0xF0000000UL) == (cfg->clkdiv0&0xF0000000UL) &&
			(inpw(REG_CLKDIV1)&0xF0000000UL) == (cfg->clkdiv1&0xF0000000UL) )
	{
		/* Just change dividor when UPLL & HCLK is not changed */
			
		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}

		clkcon = inpw(REG_CLKCON);	
		if(clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		}

		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}

		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}

		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}

		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
			outpw(REG_SDTIME0, cfg->sdtime0);
			DELAY(DELAY_LOOPS);
		}
	}
	else if( inpw(REG_UPLLCON) == cfg->upllcon )
	{
		/* change dividor and SDRAM timing when only UPLL fixed */
		/* Force the SDRAM timing to slowest */
#if CONFIG_MEMORY_SIZE == 16
		outpw(REG_SDTIME0, 0xC0006948);
#elif CONFIG_MEMORY_SIZE == 8
		outpw(REG_SDTIME0, 0xF8006948);
#else
#	error "No memory size defined"
#endif
		DELAY(DELAY_LOOPS);

		/* select the clock source */
		if(inpw(REG_CLKSEL) != cfg->clksel)
		{
			outpw(REG_CLKSEL, cfg->clksel);
			DELAY(DELAY_LOOPS);
		}
		
		clkcon = inpw(REG_CLKCON);
		if( clkcon != cfg->clkcon)
		{
			/* Enable IP's step by step */		
			clkcon |= (cfg->clkcon&0xF0000000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00F00000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x000F0000);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
			clkcon |= (cfg->clkcon&0x00000F00);
			outpw(REG_CLKCON, clkcon);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting the dividor */		
		if(inpw(REG_CLKDIV0) != cfg->clkdiv0)
		{
			outpw(REG_CLKDIV0, cfg->clkdiv0);
			DELAY(DELAY_LOOPS);
		}
		
		if(inpw(REG_CLKDIV1) != cfg->clkdiv1)
		{
			outpw(REG_CLKDIV1, cfg->clkdiv1);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting SDRAM control */
		if(inpw(REG_SDICON) != cfg->sdcon)
		{
			outpw(REG_SDICON, cfg->sdcon);
			DELAY(DELAY_LOOPS);
		}
		
		/* setting SDRAM timing */
		if(inpw(REG_SDTIME0) != cfg->sdtime0)
		{
#if CONFIG_MEMORY_SIZE == 16
			outpw(REG_SDTIME0, (cfg->sdtime0&0x00FFFFFF)|0xC0000000);
#elif CONFIG_MEMORY_SIZE == 8
			outpw(REG_SDTIME0, cfg->sdtime0);
#else
#	error "No memory size defined"
#endif
			DELAY(DELAY_LOOPS);
		}
	}
	else
	{
		/* Force the SDRAM timing to slowest */
#if CONFIG_MEMORY_SIZE == 16	
		outpw(REG_SDTIME0, 0xC0006948);
#elif CONFIG_MEMORY_SIZE == 8
		outpw(REG_SDTIME0, 0xF8006948);
#else
#	error "No memory size defined"
#endif
		DELAY(DELAY_LOOPS);

		/* close some IP to make system stable */
		clkcon = cfg->clkcon & 0x1F00F0F0;
		outpw(REG_CLKCON , clkcon);
		
		/* force the clock source to crystal for futur clock change */
		outpw(REG_CLKSEL , 0x00000114);
		/* change the UPLL first, and wait for stable */
		outpw(REG_UPLLCON, cfg->upllcon);
		DELAY(DELAY_LOOPS);
		
		/* select the clock source to UPLL */
		outpw(REG_CLKSEL, cfg->clksel);
		DELAY(DELAY_LOOPS);
		
		/* Enable IP's step by step */		
		clkcon |= (cfg->clkcon&0xE0000000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x00F00000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x000F0000);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x00000F00);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		clkcon |= (cfg->clkcon&0x0000000F);
		outpw(REG_CLKCON, clkcon);
		DELAY(DELAY_LOOPS);
		
		/* setting the dividor */		
		outpw(REG_CLKDIV0, cfg->clkdiv0);
		outpw(REG_CLKDIV1, cfg->clkdiv1);
		DELAY(DELAY_LOOPS);
		
		/* setting SDRAM control */
		outpw(REG_SDICON, cfg->sdcon);
		DELAY(DELAY_LOOPS);
		
		/* setting SDRAM timing */
#if CONFIG_MEMORY_SIZE == 16	
		outpw(REG_SDTIME0, (cfg->sdtime0&0x00FFFFFF)|0xC0000000);
#elif CONFIG_MEMORY_SIZE == 8
		outpw(REG_SDTIME0, cfg->sdtime0);
#else
#	error "No memory size defined"
#endif
		DELAY(DELAY_LOOPS);
	}	
}



cyg_int32 wlan_fmi_init()
{
	int volatile i;
	int io_func=0;
	FMI_CARD_DETECT_T card;
	int TotalSectors_SM;
	
	//cyg_mutex_init(&sdiofmi_mutex);
	/* initial system */
	sysDisableCache();
	sysInvalidCache();
    sysEnableCache(CACHE_WRITE_THROUGH);

	pwr_set_clk(&clk_profile[2]);

#if 0 //cplu--
	/* Reset WiFi card */
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFDFFFF);		/* Set GPIO17 output */
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	cyg_thread_delay(10);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFDFFFF);	/* Set GPIO17 low */
	cyg_thread_delay(10);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
	cyg_thread_delay(10);
#endif

	/* demo board FMI settings */
#if defined IPCAM_CONFIG_IP_CAM_VER_0 \
	|| defined IPCAM_CONFIG_IP_CAM_VER_1 \
	|| defined IPCAM_CONFIG_IP_CAM_VER_2 \
	|| defined IPCAM_CONFIG_IP_CAM_VER_3
	card.uCard = FMI_SD_CARD;		// card type
	card.uGPIO = FMI_NO_CARD_DETECT;				// card detect GPIO pin
	card.uWriteProtect = FMI_NO_WRITE_PROTECT;		// write protect detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = FMI_NO_POWER_PIN;				// card power pin, -1 means no power pin
	card.bIsTFlashCard = FALSE;
#elif defined IPCAM_CONFIG_MP4_EVB_VER_0 || defined IPCAM_CONFIG_MP4_EVB_VER_1
	card.uCard = FMI_SD_CARD;		// card type
	card.uGPIO = 4;					// card detect GPIO pin
	card.uWriteProtect = 16;		// write protect detect GPIO pin
	card.uInsert = 0;				// 0/1 which one is insert
	card.nPowerPin = 12;			// card power pin, -1 means no power pin
	card.bIsTFlashCard = FALSE;
#else
#	error "No hardware platform defined!"
#endif

	fmiSetCardDetection(&card);
	fmiSetSDOutputClockbykHz(24000);
	fmiSetFMIReferenceClock(72000);//108000//135000//54000
	fmiSetGPIODebounceTick(10);
	fsFixDriveNumber('C', 'D', 'Z');//SD card = C: Nand flash = D:
	fsInitFileSystem();
	fmiInitDevice();

	for (i=0; i<100000; i++);
	//sysSetLocalInterrupt(ENABLE_IRQ);
	//printf("fmiInitSMDevice - status [%x]\n", fmiInitSMDevice(0));

	/* download firmware */
	io_func = fmiInitSDDevice();
	diag_printf("fmiInitSDDevice - status [%x]\n", io_func);
//cplu++
	while(io_func == 0xFFFF0102)
	{
		outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & 0xFFFDFFFF);		/* Set GPIO17 output */
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
		cyg_thread_delay(10);
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) & 0xFFFDFFFF);	/* Set GPIO17 low */
		cyg_thread_delay(10);
		outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | 0x00020000);	/* Set GPIO17 high */
		cyg_thread_delay(10);
		io_func = fmiInitSDDevice();
		diag_printf("fmiInitSDDevice - status [%x]\n", io_func);
		}
//cplu end
	TotalSectors_SM = fmiInitSMDevice(0);
	diag_printf("gTotalSectors_SM = %d\n",TotalSectors_SM);
	
	outpw(REG_SDIER, 0x03);
	
	return io_func;
	//sdio_init_module(io_func);

}

