/** @file if_sdio.c
 *  @brief This file contains SDIO IF (interface) module
 *  related functions.
 *
 *  Copyright ?Marvell International Ltd. and/or its affiliates, 2003-2006
 */
/****************************************************
Change log:
	10/14/05: add Doxygen format comments 
	10/20/05: add SD8686 support 
	11/10/05: add iMX21 support
	01/05/06: add kernel 2.6.x support
	01/23/06: add fw downlaod
	06/06/06: add macro SD_BLOCK_SIZE_FW_DL for firmware download
		  add macro ALLOC_BUF_SIZE for cmd resp/Rx data skb buffer allocation
****************************************************/

#include	"if_sdio.h"
#include "cyg/io/eth/netdev.h"
#include "cyg/io/eth/eth_drv.h"
#include "cyg/io/eth/eth_drv_stats.h"
#include "net/wireless.h"
#include "net/iw_handler.h"
#include    "wlan_defs.h"
#include    "wlan_thread.h"
#include    "wlan_types.h"
//#include    "wlan_wmm.h"
#include    "wlan_11d.h"

//#include    "os_timers.h"

#include    "host.h"
#include    "hostcmd.h"

#include    "wlan_scan.h"
#include    "wlan_join.h"

#include    "wlan_dev.h"

#include	"sbi.h"
#include "std.h"
#include "w702sdio.h"
#include "helper.h"
#include "sd8686.h"

/* define SD block size for firmware download */
#define SD_BLOCK_SIZE_FW_DL	32

/* define SD block size for data Tx/Rx */
#define SD_BLOCK_SIZE		320     /* To minimize the overhead of ethernet frame
                                           with 1514 bytes, 320 bytes block size is used */

#define ALLOC_BUF_SIZE		(((MAX(MRVDRV_ETH_RX_PACKET_BUFFER_SIZE, \
					MRVDRV_SIZE_OF_CMD_BUFFER) + SDIO_HEADER_LEN \
					+ SD_BLOCK_SIZE - 1) / SD_BLOCK_SIZE) * SD_BLOCK_SIZE)

#ifdef _MAINSTONE
#define GPIO_PORT_NUM		24
#define GPIO_PORT_INIT()	(SET_PXA_GPIO_MODE((GPIO_PORT_NUM) | GPIO_OUT))
#define GPIO_PORT_TO_HIGH()	(GPSR0 = 1 << GPIO_PORT_NUM)
#define GPIO_PORT_TO_LOW()	(GPCR0 = 1 << GPIO_PORT_NUM)
#else /* !_MAINSTONE */
#define GPIO_PORT_NUM
#define GPIO_PORT_INIT()
#define GPIO_PORT_TO_HIGH()
#define GPIO_PORT_TO_LOW()
#endif /* _MAINSTONE */

extern int ethernetRunning;

/********************************************************
		Local Variables
********************************************************/

static wlan_private *pwlanpriv;
static wlan_private *(*wlan_add_callback) (void *dev_id);
static int (*wlan_remove_callback) (void *dev_id);

/********************************************************
		Global Variables
********************************************************/

mmc_notifier_rec_t if_sdio_notifier;
isr_notifier_fn_t isr_function;

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief This function adds the card
 *  
 *  @param card    A pointer to the card
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
sbi_add_card(void *card)
{
    if (!wlan_add_callback)
        return WLAN_STATUS_FAILURE;

    pwlanpriv = wlan_add_callback(card);

    if (pwlanpriv)
        return WLAN_STATUS_SUCCESS;
    else
        return WLAN_STATUS_FAILURE;
}

/** 
 *  @brief This function removes the card
 *  
 *  @param card    A pointer to the card
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
sbi_remove_card(void *card)
{
    if (!wlan_remove_callback)
        return WLAN_STATUS_FAILURE;

    pwlanpriv = NULL;
    return wlan_remove_callback(card);
}

/** 
 *  @brief This function reads scratch registers
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param dat	   A pointer to keep returned data
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
mv_sdio_read_scratch(wlan_private * priv, cyg_uint16 * dat)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 scr0;
    cyg_uint8 scr1;
	cyg_uint8 scr2;
    ret = sdio_read_ioreg(priv->wlan_dev.card, FN1, CARD_OCR_0_REG, &scr0);
    if (ret < 0)
        return WLAN_STATUS_FAILURE;

//    diag_printf("SCRATCH_0_REG = %x\n", scr0);
    ret = sdio_read_ioreg(priv->wlan_dev.card, FN1, CARD_OCR_1_REG, &scr1);
    if (ret < 0)
        return WLAN_STATUS_FAILURE;

//    diag_printf("SCRATCH_1_REG = %x\n", scr1);

	/* the SCRATCH_REG @ 0x8fc tells the cause for the Mac Event */
	ret = sdio_read_ioreg(priv->wlan_dev.card, FN1, CARD_OCR_3_REG, &scr2);
	if (ret < 0)
		return WLAN_STATUS_FAILURE;

	priv->adapter->EventCause = scr2;

    *dat = (((cyg_uint16) scr1) << 8) | scr0;

    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function polls the card status register.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param bits    	the bit mask
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
mv_sdio_poll_card_status(wlan_private * priv, cyg_uint8 mv_bits)
{
    int tries;
    int rval;
    cyg_uint8 cs;

    for (tries = 0; tries < MAX_POLL_TRIES; tries++) {
        rval = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                               CARD_STATUS_REG, &cs);
        if (rval == 0 && (cs & mv_bits) == mv_bits) {
            return WLAN_STATUS_SUCCESS;
        }

        udelay(10);
    }

    diag_printf("mv_sdio_poll_card_status: FAILED!\n");
    return WLAN_STATUS_FAILURE;
}


/** 
 *  @brief This function programs the firmware image.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param firmware 	A pointer to the buffer of firmware image
 *  @param firmwarelen 	the length of firmware image
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
sbi_prog_firmware_image(wlan_private * priv,
                        const cyg_uint8 * firmware, int firmwarelen)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint16 firmwarestat;
    cyg_uint8 *fwbuf = priv->adapter->TmpTxBuf;
    int fwblknow;
    cyg_uint32 tx_len;
#ifdef FW_DOWNLOAD_SPEED
    cyg_uint32 tv1, tv2;
#endif

	int start, end;

    ENTER();

    /* Check if the firmware is already downloaded */
    if ((ret = mv_sdio_read_scratch(priv, &firmwarestat)) < 0) {
        diag_printf("read scratch returned <0\n");
        goto done;
    }

    if (firmwarestat == FIRMWARE_READY) {
        diag_printf("Firmware already downloaded!\n");
        ret = WLAN_STATUS_SUCCESS;
        goto done;
    }

    diag_printf("Downloading helper image (%d bytes), block size %d bytes \n",
           firmwarelen, SD_BLOCK_SIZE_FW_DL);

#ifdef FW_DOWNLOAD_SPEED
    tv1 = get_utimeofday();
#endif
    /* Perform firmware data transfer */
    tx_len =
        (FIRMWARE_TRANSFER_NBLOCK * SD_BLOCK_SIZE_FW_DL) - SDIO_HEADER_LEN;
    start = cyg_current_time();
    for (fwblknow = 0; fwblknow < firmwarelen; fwblknow += tx_len) {

        /* The host polls for the DN_LD_CARD_RDY and IO_READY bits */
        ret = mv_sdio_poll_card_status(priv, IO_READY | DN_LD_CARD_RDY);
        if (ret < 0) {
            diag_printf("Firmware download died @ %d\n", fwblknow);
            goto done;
        }

        /* Set blocksize to transfer - checking for last block */
        if (firmwarelen - fwblknow < tx_len)
            tx_len = firmwarelen - fwblknow;

        fwbuf[0] = ((tx_len & 0x000000ff) >> 0);        /* Little-endian */
        fwbuf[1] = ((tx_len & 0x0000ff00) >> 8);
        fwbuf[2] = ((tx_len & 0x00ff0000) >> 16);
        fwbuf[3] = ((tx_len & 0xff000000) >> 24);

        /* Copy payload to buffer */
        memcpy(&fwbuf[SDIO_HEADER_LEN], &firmware[fwblknow], tx_len);

        diag_printf(".");

        /* Send data */
        ret = sdio_write_iomem(priv->wlan_dev.card, FN1,
                               priv->wlan_dev.ioport, BLOCK_MODE,
                               FIXED_ADDRESS, FIRMWARE_TRANSFER_NBLOCK,
                               SD_BLOCK_SIZE_FW_DL, fwbuf);

        if (ret < 0) {
            diag_printf("IO error: transferring block @ %d\n", fwblknow);
            goto done;
        }
    }
	end = cyg_current_time();
    diag_printf("\ndone (%d/%d bytes) %d %dBps\n", fwblknow, firmwarelen, end - start);
#ifdef FW_DOWNLOAD_SPEED
    tv2 = get_utimeofday();
    diag_printf("helper: %ld.%03ld.%03ld ", tv1 / 1000000,
           (tv1 % 1000000) / 1000, tv1 % 1000);
    diag_printf(" -> %ld.%03ld.%03ld ", tv2 / 1000000,
           (tv2 % 1000000) / 1000, tv2 % 1000);
    tv2 -= tv1;
    diag_printf(" == %ld.%03ld.%03ld\n", tv2 / 1000000,
           (tv2 % 1000000) / 1000, tv2 % 1000);
#endif

    /* Write last EOF data */
    diag_printf("Transferring EOF block\n");
    memset(fwbuf, 0x0, SD_BLOCK_SIZE_FW_DL);
    ret = sdio_write_iomem(priv->wlan_dev.card, FN1,
                           priv->wlan_dev.ioport, BLOCK_MODE,
                           FIXED_ADDRESS, 1, SD_BLOCK_SIZE_FW_DL, fwbuf);

    if (ret < 0) {
        diag_printf("IO error in writing EOF firmware block\n");
        goto done;
    }
    
    //cyg_thread_delay(100);//clyu
    udelay(1000*200);

    ret = WLAN_STATUS_SUCCESS;

  done:
    return ret;
}

/** 
 *  @brief This function downloads firmware image to the card.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param firmware	A pointer to firmware image buffer
 *  @param firmwarelen	the length of firmware image
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
sbi_download_wlan_fw_image(wlan_private * priv,
                           const cyg_uint8 * firmware, int firmwarelen)
{
    cyg_uint8 base0;
    cyg_uint8 base1;
    int ret = WLAN_STATUS_SUCCESS;
    int offset;
    cyg_uint8 *fwbuf = priv->adapter->TmpTxBuf;
    int timeout = 5000;
    cyg_uint16 len;
    int txlen = 0;
	int	fwblknow = 0;
	int	tx_len = 0;
#ifdef FW_DOWNLOAD_SPEED
    cyg_uint32 tv1, tv2;
#endif
	
    ENTER();

    diag_printf("WLAN_FW: Downloading firmware of size %d bytes\n",
           firmwarelen);

#ifdef FW_DOWNLOAD_SPEED
    tv1 = get_utimeofday();
#endif

    /* Wait initially for the first non-zero value */
    do {
        if ((ret = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                                   HOST_F1_RD_BASE_0, &base0)) < 0) {
            diag_printf("Dev BASE0 register read failed:"
                   " base0=0x%04X(%d)\n", base0, base0);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        if ((ret = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                                   HOST_F1_RD_BASE_1, &base1)) < 0) {
            diag_printf("Dev BASE1 register read failed:"
                   " base1=0x%04X(%d)\n", base1, base1);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        len = (((cyg_uint16) base1) << 8) | base0;
        mdelay(1);
    } while (!len && --timeout);

    if (!timeout) {
        diag_printf("Helper downloading finished.\n");
        diag_printf("Timeout for firmware downloading!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    /* The host polls for the DN_LD_CARD_RDY and IO_READY bits */
    ret = mv_sdio_poll_card_status(priv, IO_READY | DN_LD_CARD_RDY);
    if (ret < 0) {
        diag_printf("Firmware download died @ the end\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    diag_printf("WLAN_FW: Len got from firmware = 0x%04X(%d)\n", len, len);
    len &= ~B_BIT_0;
	
    /* Perform firmware data transfer */
    for (offset = 0; offset < firmwarelen; offset += txlen)
    {
        txlen = len;

        /* Set blocksize to transfer - checking for last block */
        if (firmwarelen - offset < txlen)
        {
            txlen = firmwarelen - offset;
        }
        //diag_printf("WLAN_FW: offset=%d, txlen = 0x%04X(%d)\n", offset, txlen, txlen);

 	    tx_len = (FIRMWARE_TRANSFER_NBLOCK * SD_BLOCK_SIZE * 8);

    	for (fwblknow = 0; fwblknow < txlen; fwblknow += tx_len)
    	{
	        /* The host polls for the DN_LD_CARD_RDY and IO_READY bits */
	        ret = mv_sdio_poll_card_status(priv, IO_READY | DN_LD_CARD_RDY);
	        if (ret < 0) {
	            diag_printf("Firmware download died @ %d\n", offset);
	            goto done;
    	    }
			if (txlen - fwblknow < tx_len)
				tx_len = txlen - fwblknow;

	        /* Copy payload to buffer */
			memcpy(fwbuf, &firmware[offset+fwblknow], tx_len);

        	/* Send data */
        	ret = sdio_write_iomem(priv->wlan_dev.card, FN1,
                               priv->wlan_dev.ioport, BLOCK_MODE,
                               FIXED_ADDRESS, FIRMWARE_TRANSFER_NBLOCK*8, SD_BLOCK_SIZE_FW_DL,
                               fwbuf);

        	if (ret < 0) {
            	diag_printf("IO error:transferring @ %d\n", offset);
            	goto done;
        	}
        }

        do {
            udelay(10);
            if ((ret = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                                       HOST_F1_CARD_RDY, &base0)) < 0) {
                diag_printf("Dev CARD_RDY register read failed:"
                       " base0=0x%04X(%d)\n", base0, base0);
                ret = WLAN_STATUS_FAILURE;
                goto done;
            }
//            diag_printf("offset=0x%08X len=0x%04X: FN1,"
//                   "HOST_F1_CARD_RDY: 0x%04X\n", offset, txlen, base0);
        } while (!(base0 & 0x08) || !(base0 & 0x01));

        if ((ret = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                                   HOST_F1_RD_BASE_0, &base0)) < 0) {
            diag_printf("Dev BASE0 register read failed:"
                   " base0=0x%04X(%d)\n", base0, base0);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        if ((ret = sdio_read_ioreg(priv->wlan_dev.card, FN1,
                                   HOST_F1_RD_BASE_1, &base1)) < 0) {
            diag_printf("Dev BASE1 register read failed:"
                   " base1=0x%04X(%d)\n", base1, base1);
            ret = WLAN_STATUS_FAILURE;
            goto done;
        }
        len = (((cyg_uint16) base1) << 8) | base0;

        if (!len) {
            diag_printf("WLAN Firmware Download Over\n");
            break;
        }

        if (len & B_BIT_0) {
            diag_printf("CRC32 Error indicated by the helper:"
                   " len=0x%04X(%d)\n", len, len);
            len &= ~B_BIT_0;
            /* Setting this to 0 to resend from same offset */
            txlen = 0;
        } else {
            //diag_printf("%d,%d bytes block of firmware downloaded\n",
            //       offset, txlen);
        }
    }

    diag_printf("Firmware Image of Size %d bytes downloaded\n", firmwarelen);

    ret = WLAN_STATUS_SUCCESS;
  done:
#ifdef FW_DOWNLOAD_SPEED
    tv2 = get_utimeofday();
    diag_printf("firmware: %ld.%03ld.%03ld ", tv1 / 1000000,
           (tv1 % 1000000) / 1000, tv1 % 1000);
    diag_printf(" -> %ld.%03ld.%03ld ", tv2 / 1000000,
           (tv2 % 1000000) / 1000, tv2 % 1000);
    tv2 -= tv1;
    diag_printf(" == %ld.%03ld.%03ld\n", tv2 / 1000000,
           (tv2 % 1000000) / 1000, tv2 % 1000);
#endif
    LEAVE();
    return ret;
}

/** 
 *  @brief This function reads data from the card.
 *  
 *  @param priv    	A pointer to wlan_private structure
 *  @param type	   	A pointer to keep type as data or command
 *  @param nb		A pointer to keep the data/cmd length retured in buffer
 *  @param payload 	A pointer to the data/cmd buffer
 *  @param nb	   	the length of data/cmd buffer
 *  @param npayload	the length of data/cmd buffer
 *  @return 	   	WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
mv_sdio_card_to_host(wlan_private * priv,
                     cyg_uint32 * type, int *nb, cyg_uint8 * payload, int npayload)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint16 buf_len = 0;
    int buf_block_len;
    int blksz;
    cyg_uint32 *pevent;

    if (!payload) {
        diag_printf("payload NULL pointer received!\n");
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

    /* Read the length of data to be transferred */
    ret = mv_sdio_read_scratch(priv, &buf_len);
    if (ret < 0) {
        diag_printf("Failed to read the scratch reg\n");
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

   // diag_printf("Receiving %d bytes from card at scratch reg value\n", buf_len);

    if (((buf_len - SDIO_HEADER_LEN) <= 0) || (buf_len > npayload)) {
        diag_printf("Invalid packet size from firmware, size = %d\n",
               buf_len);
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

    /* Allocate buffer */
    blksz = SD_BLOCK_SIZE;
    buf_block_len = (buf_len + blksz - 1) / blksz;

    /* The host polls for the IO_READY bit */
    ret = mv_sdio_poll_card_status(priv, IO_READY);
    if (ret < 0) {
        diag_printf("Poll failed in card_to_host : %d\n", ret);
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

    ret = sdio_read_iomem(priv->wlan_dev.card, FN1, priv->wlan_dev.ioport,
                          BLOCK_MODE, FIXED_ADDRESS, buf_block_len,
                          blksz, payload);
#if 0
	{
		int i;
		//diag_printf("\n*****Read Data*****\n");
		diag_printf("\n");
		
		for (i=0; i<buf_len; i++) {
			diag_printf("%02x ",payload[i]);
			if (!((i+1)%8))
				diag_printf("\n");
		}
		diag_printf("\n");
	}
#endif

	//priv->adapter->CurCmd->BufVirtualAddr = &payload[4];	// HPChen

    if (ret < 0) {
        diag_printf("sdio_read_iomem failed - mv_sdio_card_to_host\n");
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }
    *nb = buf_len;

    if (*nb <= 0) {
        diag_printf("Null packet recieved \n");
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

    *type = (payload[2] | (payload[3] << 8));

    if (*type == MVSD_EVENT) {
        pevent = (cyg_uint32 *) & payload[4];
        priv->adapter->EventCause = MVSD_EVENT | (((cyg_uint16) (*pevent)) << 3);
    }

  exit:
    return ret;
}

/** 
 *  @brief This function enables the host interrupts mask
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param mask	   the interrupt mask
 *  @return 	   WLAN_STATUS_SUCCESS
 */
static int
enable_host_int_mask(wlan_private * priv, cyg_uint8 mask)
{
    int ret = WLAN_STATUS_SUCCESS;
    mmc_card_t card = (mmc_card_t) priv->wlan_dev.card;

    /* Simply write the mask to the register */
    ret = sdio_write_ioreg(card, FN1, HOST_INT_MASK_REG, mask);

    if (ret < 0) {
        diag_printf("ret = %d\n", ret);
        ret = WLAN_STATUS_FAILURE;
    }

    priv->adapter->HisRegCpy = 1;

    return ret;
}

/**  @brief This function disables the host interrupts mask.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param mask	   the interrupt mask
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
static int
disable_host_int_mask(wlan_private * priv, cyg_uint8 mask)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 host_int_mask;

    /* Read back the host_int_mask register */
    ret = sdio_read_ioreg(priv->wlan_dev.card, FN1, HOST_INT_MASK_REG,
                          &host_int_mask);
    if (ret < 0) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    /* Update with the mask and write back to the register */
    host_int_mask &= ~mask;
    ret = sdio_write_ioreg(priv->wlan_dev.card, FN1, HOST_INT_MASK_REG,
                           host_int_mask);
    if (ret < 0) {
        diag_printf("Unable to diable the host interrupt!\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

  done:
    return ret;
}

/********************************************************
		Global Functions
********************************************************/

int start_bus_clock(mmc_controller_t);
int stop_bus_clock_2(mmc_controller_t);

/** 
 *  @brief This function handles the interrupt.
 *  
 *  @param irq 	   The irq of device.
 *  @param dev_id  A pointer to net_device structure
 *  @param fp	   A pointer to pt_regs structure
 *  @return 	   n/a
 */
static cyg_uint32
sbi_interrupt(cyg_vector_t vector, cyg_addrword_t data)
{
    //struct net_device *dev = dev_id;
    struct cyg_netdevtab_entry *tab = (struct cyg_netdevtab_entry *)data;
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    wlan_private *priv = (wlan_private *)sc->driver_private;
	wlan_adapter *Adapter = priv->adapter;
	cyg_uint8 ireg = 0;

    //priv = dev->priv;

    //diag_printf("sbi_interrupt called IntCounter= %d\n",priv->adapter->IntCounter);
	
	
//	sbi_get_int_status(sc, &ireg);
//	Adapter->HisRegCpy |= ireg;
    wlan_interrupt(sc);

    //cyg_interrupt_acknowledge(vector);
	return 0;//CYG_ISR_HANDLED;
}

/** 
 *  @brief This function registers the IF module in bus driver.
 *  
 *  @param add	   wlan driver's call back funtion for add card.
 *  @param remove  wlan driver's call back funtion for remove card.
 *  @param arg     not been used
 *  @return	   An int pointer that keeps returned value
 */
int *
sbi_register(wlan_notifier_fn_add add, wlan_notifier_fn_remove remove,
             void *arg)
{
    int *sdio_ret;

    wlan_add_callback = add;
    wlan_remove_callback = remove;

    if_sdio_notifier.add = (mmc_notifier_fn_t) sbi_add_card;
    if_sdio_notifier.remove = (mmc_notifier_fn_t) sbi_remove_card;
    isr_function = sbi_interrupt;
    sdio_ret = sdio_register(MMC_REG_TYPE_USER, &if_sdio_notifier, 0);

    /* init GPIO PORT for wakeup purpose */
    //GPIO_PORT_INIT();

    /* set default value */
    //GPIO_PORT_TO_HIGH();

    return sdio_ret;
}

/** 
 *  @brief This function de-registers the IF module in bus driver.
 *  
 *  @return 	   n/a
 */
void
sbi_unregister(void)
{
    sdio_unregister(MMC_REG_TYPE_USER, &if_sdio_notifier);
}

/** 
 *  @brief This function reads the IO register.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param func	   funcion number
 *  @param reg	   register to be read
 *  @param dat	   A pointer to variable that keeps returned value
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_read_ioreg(wlan_private * priv, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 * dat)
{
    return sdio_read_ioreg(priv->wlan_dev.card, func, reg, dat);
}

/** 
 *  @brief This function writes the IO register.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param func	   funcion number
 *  @param reg	   register to be written
 *  @param dat	   the value to be written
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_write_ioreg(wlan_private * priv, cyg_uint8 func, cyg_uint32 reg, cyg_uint8 dat)
{
    return sdio_write_ioreg(priv->wlan_dev.card, func, reg, dat);
}

/** 
 *  @brief This function checks the interrupt status and handle it accordingly.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param ireg    A pointer to variable that keeps returned value
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_get_int_status(struct eth_drv_sc *sc, cyg_uint8 * ireg)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 *cmdBuf;
    wlan_private *wlan_priv = (wlan_private *)sc->driver_private;
    wlan_dev_t *wlan_dev = &wlan_priv->wlan_dev;
    w99702_priv_t * priv = wlan_priv->priv;
    TXBD *txbd;
    RXBD *rxbd;
   // struct sk_buff *skb;
    mmc_card_t card = (mmc_card_t) wlan_dev->card;

	txbd=( TXBD *)priv->tx_head;
	rxbd=(RXBD *)priv->rx_tail ;

#if 0
	{
		int volatile i;
		for (i=0; i<4000; i++);
		diag_printf("delay ***\n");
	}
#endif

   	if ((ret = sdio_read_ioreg(card, FN1, HOST_INTSTATUS_REG, ireg)) < 0) {
   	    diag_printf("sdio_read_ioreg: reading interrupt status register"
   	           " failed\n");
   	    ret = WLAN_STATUS_FAILURE;
   	    goto done;
   	}
    	
   	if (*ireg != 0)
   	{
   	    /*
   	     * DN_LD_HOST_INT_STATUS and/or UP_LD_HOST_INT_STATUS
   	     * Clear the interrupt status register
   	     */
   	    if ((ret = sdio_write_ioreg(card, FN1, HOST_INTSTATUS_REG,
   	                                ~(*ireg) & (DN_LD_HOST_INT_STATUS |
   	                                            UP_LD_HOST_INT_STATUS))) < 0)
		{
   	        diag_printf(
   	               "sdio_write_ioreg: clear interrupt status"
   	               " register failed\n");
   	        ret = WLAN_STATUS_FAILURE;
   	        goto done;
   	    }
   	    // clear FMI SDIO interrupt
		outpw(REG_SDISR, inpw(REG_SDISR) | 0x400);

   	}
    //else
    //	diag_printf("ireg = 0x%x, ~ireg = 0x%x\n", *ireg, ~(*ireg));

    if (*ireg & DN_LD_HOST_INT_STATUS) {        /* tx_done INT */
        *ireg |= HIS_TxDnLdRdy;
        if (!wlan_priv->wlan_dev.dnld_sent) {        /* tx_done already received */
            diag_printf("warning: tx_done already received:"
                   " dnld_sent=0x%x ireg=0x%x\n",
                   wlan_priv->wlan_dev.dnld_sent, *ireg);
        } else {
    	       // if (wlan_priv->wlan_dev.dnld_sent == DNLD_DATA_SENT)
    	        //    os_start_queue(priv);
    	        wlan_priv->wlan_dev.dnld_sent = DNLD_RES_RECEIVED;
		}
    }

    if (*ireg & UP_LD_HOST_INT_STATUS)
    {
        /* 
         * DMA read data is by block alignment,so we need alloc extra block
         * to avoid wrong memory access.
         */
        /*if (!(skb = dev_alloc_skb(ALLOC_BUF_SIZE))) {
            diag_printf("No free skb\n");
            priv->stats.rx_dropped++;
            return WLAN_STATUS_FAILURE;
        }*/
        if((rxbd->mode & RXfOwnership_DMA) != RXfOwnership_DMA)
        {
        	rxbd = &priv->tmprx_desc;
        	diag_printf("receive discriptor full %x, %x\n", rxbd->mode, rxbd->buffer);
        	//ret = WLAN_STATUS_RECEIVE_FULL;
        	//return ret;
		}
        /* 
         * Transfer data from card
         * skb->tail is passed as we are calling skb_put after we
         * are reading the data
         */
         //diag_printf("03REG_SDISR = %x\n",inpw(REG_SDISR));
        if (mv_sdio_card_to_host(wlan_priv, &wlan_dev->upld_typ,
                                 (int *) &wlan_dev->upld_len, (cyg_uint8 *)rxbd->buffer,
                                 WLAN_UPLD_SIZE) < 0) {
            cyg_uint8 cr = 0;

            diag_printf("Card to host failed: ireg=0x%x\n", *ireg);
            if (sdio_read_ioreg(wlan_dev->card, FN1,
                                CONFIGURATION_REG, &cr) < 0)
                diag_printf("sdio_read_ioreg failed\n");

            diag_printf("Config Reg val = %d\n", cr);
            if (sdio_write_ioreg(wlan_dev->card, FN1,
                                 CONFIGURATION_REG, (cr | 0x04)) < 0)
                diag_printf("write ioreg failed\n");

            diag_printf("write success\n");
            if (sdio_read_ioreg(wlan_dev->card, FN1,
                                CONFIGURATION_REG, &cr) < 0)
                diag_printf("sdio_read_ioreg failed\n");

            diag_printf("Config reg val =%x\n", cr);
            ret = WLAN_STATUS_FAILURE;
            //kfree_skb(skb);
            goto done;
        }

        if (wlan_dev->upld_typ == MVSD_DAT) {

			if(ethernetRunning)
			{
	            *ireg |= HIS_RxUpLdRdy;
    	        rxbd->SL = wlan_priv->wlan_dev.upld_len;
	            if(rxbd->next)
	            {
	            	rxbd->mode = 0;
	            	priv->rx_tail = (RXBD *)rxbd->next;
	            	//eth_drv_dsr(0, 0, (cyg_addrword_t)sc);
	            }
	        }
#if 0
            {
            	char *ptr = (char*)rxbd->buffer;
            	ptr += 20;
            	//ptr += 14;//ether net header
            	//ptr += 2;
            	#if 0
            	if((*ptr == 0x00) && (*(ptr + 1) == 0x04) && (*(ptr + 2) == 0xE2) && (*(ptr + 3) == 0x44)
            			&& (*(ptr + 4) == 0x1C)&& (*(ptr + 5) == 0x78))
       			{
       			
       				ptr += 14;//ether net header
       				ptr += 8;
       				//arp packet
       				ptr += 2;
	            	if((*ptr == 0x08) && (*(ptr + 1) == 0x00))
    	        	{
        	    		ptr += 4;
            			if((*ptr == 0x00) && (*(ptr + 1) == 0x02))
            			{
            				diag_printf("get APR response\n");
	            		}
    	        	}
    	        }
       			#else
       			if(rxbd->SL >=300)
       			{
       				ptr += 6;
       				if((*ptr == 0x00) && (*(ptr + 1) == 0x02) && (*(ptr + 2) == 0xa5) && (*(ptr + 3) == 0x75)
            				&& (*(ptr + 4) == 0x0C)&& (*(ptr + 5) == 0xD3))
       				{
	            		ptr -= 6;
	            		ptr += 14;//ether net header
	            		ptr += 8;//sdio header
            			ptr +=28;//udp
       					ptr +=28;//bootp
       					
	            		if((*ptr == 0x00) && (*(ptr + 1) == 0x04) && (*(ptr + 2) == 0xE2) && (*(ptr + 3) == 0x44)
            				&& (*(ptr + 4) == 0x1C)&& (*(ptr + 5) == 0x78))
    	        		{
           					diag_printf("get DHCP offer\n");
    	        		}
    	        	}
    	    	}
       			#endif
            }
#endif
            /*skb_put(skb, priv->wlan_dev.upld_len);
            skb_pull(skb, SDIO_HEADER_LEN);
            list_add_tail((struct list_head *) skb,
                          (struct list_head *) &priv->adapter->RxSkbQ);*/
        } else if (wlan_dev->upld_typ == MVSD_CMD) {
            //diag_printf("Up load type is CMD\n");
            *ireg &= ~(HIS_RxUpLdRdy);
            *ireg |= HIS_CmdUpLdRdy;

            /* take care of CurCmd = NULL case by reading the 
             * data to clear the interrupt */
            if (!wlan_priv->adapter->CurCmd) {
                cmdBuf = wlan_priv->wlan_dev.upld_buf;
                wlan_priv->adapter->HisRegCpy &= ~HIS_CmdUpLdRdy;
            } else {
                cmdBuf = wlan_priv->adapter->CurCmd->BufVirtualAddr;
            }

            wlan_priv->wlan_dev.upld_len -= SDIO_HEADER_LEN;
            memcpy(cmdBuf, (void*)(rxbd->buffer + SDIO_HEADER_LEN),
                   MIN(MRVDRV_SIZE_OF_CMD_BUFFER, wlan_priv->wlan_dev.upld_len));
           // kfree_skb(skb);
        } else if (wlan_dev->upld_typ == MVSD_EVENT) {
            *ireg |= HIS_CardEvent;
            //kfree_skb(skb);
        }
        
        *ireg |= HIS_CmdDnLdRdy;
    }

    ret = WLAN_STATUS_SUCCESS;
done:
	//diag_printf("enable sdio interrupt\n");
	sbi_reenable_host_interrupt(wlan_priv, 0x00);//clyu

    return ret;
}

/** 
 *  @brief This function is a dummy function.
 *  
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_card_to_host(wlan_private * priv, cyg_uint32 type,
                 cyg_uint32 * nb, cyg_uint8 * payload, cyg_uint16 npayload)
{
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function is a dummy function.
 *  
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_read_event_cause(wlan_private * priv)
{
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function reenables the host interrupts.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param bits    the bit mask
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_reenable_host_interrupt(wlan_private * priv, cyg_uint8 bits)
{
    sdio_enable_SDIO_INT();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function disables the host interrupts.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_disable_host_int(wlan_private * priv)
{
    return disable_host_int_mask(priv, HIM_DISABLE);
}

/** 
 *  @brief This function enables the host interrupts.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_enable_host_int(wlan_private * priv)
{

    return enable_host_int_mask(priv, HIM_ENABLE);
}

/** 
 *  @brief This function de-registers the device.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_unregister_dev(wlan_private * priv)
{
    ENTER();

    if (priv->wlan_dev.card != NULL) {
        /* Release the SDIO IRQ */
        sdio_free_irq(priv->wlan_dev.card, priv->wlan_dev.netdev);
        diag_printf("Making the sdio dev card as NULL\n");
    }
    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function registers the device.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_register_dev(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 reg;
    mmc_card_t card = (mmc_card_t) priv->wlan_dev.card;
    struct eth_drv_sc *sc = (struct eth_drv_sc *)priv->wlan_dev.netdev->device_instance;

    ENTER();

    /* Initialize the private structure */
    strncpy(priv->wlan_dev.name, "sdio0", sizeof(priv->wlan_dev.name));
    priv->wlan_dev.ioport = 0;
    priv->wlan_dev.upld_rcv = 0;
    priv->wlan_dev.upld_typ = 0;
    priv->wlan_dev.upld_len = 0;

    /* Read the IO port */
    ret = sdio_read_ioreg(card, FN1, IO_PORT_0_REG, &reg);
    if (ret < 0)
        goto failed;
    else
        priv->wlan_dev.ioport |= reg;

    ret = sdio_read_ioreg(card, FN1, IO_PORT_1_REG, &reg);
    if (ret < 0)
        goto failed;
    else
        priv->wlan_dev.ioport |= (reg << 8);

    ret = sdio_read_ioreg(card, FN1, IO_PORT_2_REG, &reg);
    if (ret < 0)
        goto failed;
    else
        priv->wlan_dev.ioport |= (reg << 16);

    diag_printf("SDIO FUNC1 IO port: 0x%x\n", priv->wlan_dev.ioport);

    /* Disable host interrupt first. */
    if ((ret = disable_host_int_mask(priv, 0xff)) < 0) {
        diag_printf("Warning: unable to disable host interrupt!\n");
    }

    /* Request the SDIO IRQ */
    diag_printf("Before request_irq Address is if==>%p\n", isr_function);
    ret = sdio_request_irq(priv->wlan_dev.card,
                           (handler_fn_t) isr_function, 0,
                           "sdio_irq", priv->wlan_dev.netdev);

    diag_printf("IrqLine: %d\n", card->ctrlr->tmpl->irq_line);

    if (ret < 0) {
        diag_printf("Failed to request IRQ on SDIO bus (%d)\n", ret);
        goto failed;
    }

//    priv->wlan_dev.netdev->irq = card->ctrlr->tmpl->irq_line;
//    priv->adapter->irq = priv->wlan_dev.netdev->irq;
	 priv->adapter->irq = sc->funs->int_vector(sc);
    priv->adapter->chip_rev = card->chiprev;

    return WLAN_STATUS_SUCCESS;

  failed:
    priv->wlan_dev.card = NULL;

    return WLAN_STATUS_FAILURE;
}

/** 
 *  @brief This function sends data to the card.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param type	   data or command
 *  @param payload A pointer to the data/cmd buffer
 *  @param nb	   the length of data/cmd
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_host_to_card(wlan_private * priv, cyg_uint8 type, cyg_uint8 * payload, cyg_uint16 nb)
{
    int ret = WLAN_STATUS_SUCCESS;
    int buf_block_len;
    int blksz;

    ENTER();

    priv->adapter->HisRegCpy = 0;

    /* Allocate buffer and copy payload */
    blksz = SD_BLOCK_SIZE;
    buf_block_len = (nb + SDIO_HEADER_LEN + blksz - 1) / blksz;

    payload[0] = (nb + SDIO_HEADER_LEN) & 0xff;
    payload[1] = ((nb + SDIO_HEADER_LEN) >> 8) & 0xff;
    payload[2] = type;
    payload[3] = 0x0;

//	priv->adapter->TmpTxBuf[0] = (nb + SDIO_HEADER_LEN) & 0xff;
//	priv->adapter->TmpTxBuf[1] = ((nb + SDIO_HEADER_LEN) >> 8) & 0xff;
//	priv->adapter->TmpTxBuf[2] = (type == MVSD_DAT) ? 0 : 1;
//	priv->adapter->TmpTxBuf[3] = 0x0;


    if (payload != NULL && nb > 0) {
        //if (type == MVMS_CMD)
           // memcpy(&priv->adapter->TmpTxBuf[SDIO_HEADER_LEN], payload, nb);
    } else {
        diag_printf("sbi_host_to_card(): Error: payload=%p, nb=%d\n",
               payload, nb);
    }

    /* The host polls for the IO_READY bit */
    ret = mv_sdio_poll_card_status(priv, IO_READY);
    if (ret < 0) {
        diag_printf("Poll failed in host_to_card : %d\n", ret);
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    }

#if 0
	{
		int i = 0;
		cyg_uint8 *ptr;
		ptr = (cyg_uint8 *)priv->adapter->TmpTxBuf;
		for (i=0; i<nb+4; i++)
		{
			diag_printf("%02x ", (cyg_uint8) *ptr++);
			if (((i+1) % 8) == 0)
				diag_printf("\n");
		}
		diag_printf("\n");
	}
#endif

    /* Transfer data to card */
    ret = sdio_write_iomem(priv->wlan_dev.card, FN1, priv->wlan_dev.ioport,
                           BLOCK_MODE, FIXED_ADDRESS, buf_block_len,
                           blksz,payload/* priv->adapter->TmpTxBuf*/);
    if (ret < 0) {
        diag_printf("sdio_write_iomem failed: ret=%d\n", ret);
        ret = WLAN_STATUS_FAILURE;
        goto exit;
    } else {
        //diag_printf("sdio write -dnld val =>%d, type[%x]\n", ret, type);
    }

    if (type == MVSD_DAT)
        priv->wlan_dev.dnld_sent = DNLD_DATA_SENT;
    else
        priv->wlan_dev.dnld_sent = DNLD_CMD_SENT;

  exit:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function reads CIS informaion.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_get_cis_info(wlan_private * priv)
{
    mmc_card_t card = (mmc_card_t) priv->wlan_dev.card;
    wlan_adapter *Adapter = priv->adapter;
    cyg_uint8 tupledata[255];
    char cisbuf[512];
    int ofs = 0, i;
    cyg_uint32 ret = WLAN_STATUS_SUCCESS;

    ENTER();

    /* Read the Tuple Data */
    for (i = 0; i < sizeof(tupledata); i++) {
        ret = sdio_read_ioreg(card, FN0, card->info.cisptr[FN0] + i,
                              &tupledata[i]);
        if (ret < 0)
            return WLAN_STATUS_FAILURE;
    }

    memset(cisbuf, 0x0, sizeof(cisbuf));
    memcpy(cisbuf + ofs, tupledata, sizeof(cisbuf));

    /* Copy the CIS Table to Adapter */
    memset(Adapter->CisInfoBuf, 0x0, sizeof(cisbuf));
    memcpy(Adapter->CisInfoBuf, &cisbuf, sizeof(cisbuf));
    Adapter->CisInfoLen = sizeof(cisbuf);

    LEAVE();
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function probes the card.
 *  
 *  @param card_p  A pointer to the card
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_probe_card(void *card_p)
{
    mmc_card_t card = (mmc_card_t) card_p;
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint8 bic;

    ENTER();

    if (!card) {
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    /* Check for MANFID */
    ret = sdio_get_vendor_id(card);

#define MARVELL_VENDOR_ID 0x02df
    if (ret == MARVELL_VENDOR_ID) {
        diag_printf("Marvell SDIO card detected!\n");
    } else {
        diag_printf("Ignoring a non-Marvell SDIO card...\n");
        ret = WLAN_STATUS_FAILURE;
        goto done;
    }

    /* read Revision Register to get the hw revision number */
    if (sdio_read_ioreg(card, FN1, CARD_REVISION_REG, &card->chiprev) < 0) {
        diag_printf("cannot read CARD_REVISION_REG\n");
    } else {
        diag_printf("revsion=0x%x\n", card->chiprev);
        switch (card->chiprev) {
        default:
            card->block_size_512 = TRUE;
            card->async_int_mode = TRUE;

            /* enable async interrupt mode */
            sdio_read_ioreg(card, FN0, BUS_INTERFACE_CONTROL_REG, &bic);
            bic |= ASYNC_INT_MODE;
            sdio_write_ioreg(card, FN0, BUS_INTERFACE_CONTROL_REG, bic);
            break;
        }
    }

    ret = WLAN_STATUS_SUCCESS;
  done:
    LEAVE();
    return ret;
}

/** 
 *  @brief This function calls sbi_download_wlan_fw_image to download
 *  firmware image to the card.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_prog_firmware_w_helper(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->fmimage != NULL) {
        return sbi_download_wlan_fw_image(priv,
                                          Adapter->fmimage,
                                          Adapter->fmimage_len);
    }
	else
	{
		diag_printf("download FW image\n");
		return sbi_download_wlan_fw_image(priv,
				fmimage, sizeof(fmimage) );
	}
}

/** 
 *  @brief This function calls sbi_prog_firmware_image to program
 *  firmware image to the card.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_prog_firmware(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->fmimage != NULL) {
        return sbi_prog_firmware_image(priv,
                                       Adapter->fmimage,
                                       Adapter->fmimage_len);
    } else {
        diag_printf("No external firmware image\n");
        return WLAN_STATUS_FAILURE;
    }
}

/** 
 *  @brief This function programs helper image.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_prog_helper(wlan_private * priv)
{
    wlan_adapter *Adapter = priv->adapter;
    if (Adapter->helper != NULL) {
        return sbi_prog_firmware_image(priv,
                                       Adapter->helper, Adapter->helper_len);
    }else
    {
		diag_printf("download helper\n");
		return sbi_prog_firmware_image(priv, 
				helperimage, sizeof( helperimage ) );
	}
}

/** 
 *  @brief This function checks if the firmware is ready to accept
 *  command or not.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_verify_fw_download(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    cyg_uint16 firmwarestat;
    int tries;
    cyg_uint8 rsr;

    /* Wait for firmware initialization event */
    for (tries = 0; tries < MAX_FIRMWARE_POLL_TRIES; tries++) {
        if ((ret = mv_sdio_read_scratch(priv, &firmwarestat)) < 0)
            continue;

        if (firmwarestat == FIRMWARE_READY) {
            ret = WLAN_STATUS_SUCCESS;
            diag_printf("Firmware successfully downloaded\n");
            break;
        } else {
            mdelay(10);
            ret = WLAN_STATUS_FAILURE;
        }
    }

    if (ret < 0) {
        diag_printf("Timeout waiting for firmware to become active\n");
        goto done;
    }

    ret = sdio_read_ioreg(priv->wlan_dev.card, FN1, HOST_INT_RSR_REG, &rsr);
    if (ret < 0) {
        diag_printf("sdio_read_ioreg: reading INT RSR register failed\n");
        return WLAN_STATUS_FAILURE;
    } else
        diag_printf("sdio_read_ioreg: RSR register 0x%x\n", rsr);

    ret = WLAN_STATUS_SUCCESS;
  done:
    return ret;
}

/** 
 *  @brief This function set bus clock on/off
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @param option    TRUE--on , FALSE--off
 *  @return 	   WLAN_STATUS_SUCCESS
 */
int
sbi_set_bus_clock(wlan_private * priv, cyg_uint8 option)
{
    if (option == TRUE)
        start_bus_clock(((mmc_card_t) ((priv->wlan_dev).card))->ctrlr);
    else
        stop_bus_clock_2(((mmc_card_t) ((priv->wlan_dev).card))->ctrlr);
    return WLAN_STATUS_SUCCESS;
}

/** 
 *  @brief This function makes firmware exiting from deep sleep.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_exit_deep_sleep(wlan_private * priv)
{
    int ret = WLAN_STATUS_SUCCESS;
    diag_printf(
           "Trying to wakeup device... Conn=%d IntC=%d PS_Mode=%d PS_State=%d\n",
           priv->adapter->MediaConnectStatus, priv->adapter->IntCounter,
           priv->adapter->PSMode, priv->adapter->PSState);
    sbi_set_bus_clock(priv, TRUE);

    if (priv->adapter->fwWakeupMethod == WAKEUP_FW_THRU_GPIO) {
        GPIO_PORT_TO_LOW();
    } else
        ret = sdio_write_ioreg(priv->wlan_dev.card, FN1, CONFIGURATION_REG,
                               HOST_POWER_UP);

    return ret;
}

/** 
 *  @brief This function resets the setting of deep sleep.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_reset_deepsleep_wakeup(wlan_private * priv)
{

    int ret = WLAN_STATUS_SUCCESS;

    ENTER();

    if (priv->adapter->fwWakeupMethod == WAKEUP_FW_THRU_GPIO) {
        GPIO_PORT_TO_HIGH();
    } else
        ret =
            sdio_write_ioreg(priv->wlan_dev.card, FN1, CONFIGURATION_REG, 0);

    LEAVE();

    return ret;
}

#ifdef ENABLE_PM
/** 
 *  @brief This function suspends the device.
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
 */
int
sbi_suspend(wlan_private * priv)
{
    return sdio_suspend(priv->wlan_dev.card);
}

/** 
 *  @brief This function resumes the device from sleep
 *  
 *  @param priv    A pointer to wlan_private structure
 *  @return 	   WLAN_STATUS_SUCCESS or WLAN_STATUS_FAILURE
*/
int
sbi_resume(wlan_private * priv)
{
    return sdio_resume(priv->wlan_dev.card);
}
#endif
