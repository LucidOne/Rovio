/* 
   BlueZ - Bluetooth protocol stack for Linux
   Copyright (C) 2000-2001 Qualcomm Incorporated

   Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation;

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
   IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) AND AUTHOR(S) BE LIABLE FOR ANY
   CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES 
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ALL LIABILITY, INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PATENTS, 
   COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS, RELATING TO USE OF THIS 
   SOFTWARE IS DISCLAIMED.
*/

/*
 * BlueZ HCI UART driver.
 *
 * $Id: hci_ldisc.c,v 1.5 2002/10/02 18:37:20 maxk Exp $    
 */
#ifdef CYGPKG_NET_BLUEZ_STACK

#define VERSION "2.1"


#include "pkgconf/io_bluetooth_hci.h"
#include "pkgconf/io.h"

#include "cyg/io/io.h"
#include "cyg/io/devtab.h"
#include "cyg/io/serial.h"

#include "linux/kernel.h"
#include "cyg/kernel/kapi.h"
#include "linux/spinlock.h"
#include "asm/atomic.h"
#include "asm/bitops.h"
#include "linux/list.h"
#include "linux/sched.h"
#include "linux/types.h"
#include "asm/poll.h"

#include "linux/slab.h"
#include "linux/errno.h"
#include "sys/ioctl.h"

#include "net/bluetooth/bluetooth.h"
#include "net/bluetooth/hci_core.h"
#include "hci_uart.h"

//#define HCI_UART_DEBUG

#ifndef HCI_UART_DEBUG
#undef  BT_DBG
#define BT_DBG( A... )
#undef  BT_DMP
#define BT_DMP( A... )
#endif

static struct hci_uart_proto *hup[HCI_UART_MAX_PROTO];

int hci_uart_register_proto(struct hci_uart_proto *p)
{
	if (p->id >= HCI_UART_MAX_PROTO)
		return -EINVAL;

	if (hup[p->id])
		return -EEXIST;

	hup[p->id] = p;
	return 0;
}

int hci_uart_unregister_proto(struct hci_uart_proto *p)
{
	if (p->id >= HCI_UART_MAX_PROTO)
		return -EINVAL;

	if (!hup[p->id])
		return -EINVAL;

	hup[p->id] = NULL;
	return 0;
}

static struct hci_uart_proto *hci_uart_get_proto(unsigned int id)
{
	if (id >= HCI_UART_MAX_PROTO)
		return NULL;
	return hup[id];
}

static inline void hci_uart_tx_complete(struct hci_uart *hu, int pkt_type)
{
	struct hci_dev *hdev = &hu->hdev;
	
	BT_DBG("pkt_type %x\n",pkt_type);
	
	/* Update HCI stat counters */
	switch (pkt_type) {
	case HCI_COMMAND_PKT://0x01
		hdev->stat.cmd_tx++;
		break;

	case HCI_ACLDATA_PKT://0x02
		hdev->stat.acl_tx++;
		break;

	case HCI_SCODATA_PKT://0x03
		hdev->stat.cmd_tx++;
		break;
	}
}

static inline struct sk_buff *hci_uart_dequeue(struct hci_uart *hu)
{
	struct sk_buff *skb = hu->tx_skb;
	if (!skb)
		skb = hu->proto->dequeue(hu);
	else
		hu->tx_skb = NULL;
	return skb;
}

int hci_uart_tx_wakeup(struct hci_uart *hu)
{
	cyg_io_handle_t serial = hu->tty;
	struct hci_dev *hdev = &hu->hdev;
	struct sk_buff *skb;
	
	if (test_and_set_bit(HCI_UART_SENDING, &hu->tx_state)) {
		set_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
		return 0;
	}

//	BT_DBG("");

restart:
	clear_bit(HCI_UART_TX_WAKEUP, &hu->tx_state);
	while ((skb = hci_uart_dequeue(hu))) {
		int len = skb->len;
		int ret;
	
		//set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
		//len = tty->driver.write(tty, 0, skb->data, skb->len);//UART write
		//diag_printf("write skb = %x\n",skb);
		//diag_printf("data[0] %x %x %x %x\n",skb->data[0],skb->data[1],skb->data[2],skb->data[3]);
		ret = (int)cyg_io_write(serial, skb->data, (cyg_uint32*)&len);
		if(ret != ENOERR)
			break;

		hdev->stat.byte_tx += len;

		skb_pull(skb, len);
		if (skb->len) {
			hu->tx_skb = skb;
			break;
		}
	
		hci_uart_tx_complete(hu, skb->pkt_type);
		kfree_skb(skb);
	} 
	
	if (test_bit(HCI_UART_TX_WAKEUP, &hu->tx_state))
		goto restart;

	clear_bit(HCI_UART_SENDING, &hu->tx_state);
	return 0;
}

/* ------- Interface to HCI layer ------ */
/* Initialize device */
static int hci_uart_open(struct hci_dev *hdev)
{
	BT_DBG("%s %p", hdev->name, hdev);

	/* Nothing to do for UART driver */

	set_bit(HCI_RUNNING, &hdev->flags);
	return 0;
}

/* Reset device */
static int hci_uart_flush(struct hci_dev *hdev)
{
	struct hci_uart *hu  = (struct hci_uart *) hdev->driver_data;
	cyg_io_handle_t serial = hu->tty;

	BT_DBG("hdev %p tty %p", hdev, tty);

	if (hu->tx_skb) {
		kfree_skb(hu->tx_skb); hu->tx_skb = NULL;
	}

#if 0
	/* Flush any pending characters in the driver and discipline. */
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);

	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
#else
	//cyg_tty_info_t	tty_info;
	//cyg_uint32 len = sizeof(cyg_tty_info_t);
	//cyg_io_write(tty, const void *buf, cyg_uint32 *len)
	cyg_io_get_config(serial, CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH, NULL, NULL);
#endif

	if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
		hu->proto->flush(hu);

	return 0;
}

/* Close device */
static int hci_uart_close(struct hci_dev *hdev)
{
	BT_DBG("hdev %p", hdev);

	if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
		return 0;

	hci_uart_flush(hdev);
	return 0;
}

/* Send frames from HCI layer */
static int hci_uart_send_frame(struct sk_buff *skb)
{
	struct hci_dev* hdev = (struct hci_dev *) skb->dev;
	cyg_io_handle_t serial;
	struct hci_uart *hu;

	if (!hdev) {
		//BT_ERR
		printf("Frame for uknown device (hdev=NULL)\n");
		return -ENODEV;
	}

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return -EBUSY;

	hu = (struct hci_uart *) hdev->driver_data;
	serial = hu->tty;

	//BT_DBG
	printf("%s: type %d len %d\n", hdev->name, skb->pkt_type, skb->len);

	hu->proto->enqueue(hu, skb);

	hci_uart_tx_wakeup(hu);
	return 0;
}

static void hci_uart_destruct(struct hci_dev *hdev)
{
	struct hci_uart *hu;

	if (!hdev) return;

	BT_DBG("%s", hdev->name);

	hu = (struct hci_uart *) hdev->driver_data;
	free(hu);

	//MOD_DEC_USE_COUNT;
}

/* ------ LDISC part ------ */
/* hci_uart_tty_open
 * 
 *     Called when line discipline changed to HCI_UART.
 *
 * Arguments:
 *     tty    pointer to tty info structure
 * Return Value:    
 *     0 if success, otherwise error code
 */
static int hci_uart_tty_open(cyg_io_handle_t tty)
{
	struct hci_uart *hu = (struct hci_uart *)serial_getdisc(tty);//(void *) tty->disc_data;

	//BT_DBG
	//printf("open tty %p, hu= %x,return=%x\n", tty, hu, __builtin_return_address(0));

	if (hu)
		return -EEXIST;

	if (!(hu = malloc(sizeof(struct hci_uart)))) {
		//BT_ERR
		printf("Can't allocate controll structure\n");
		return -ENFILE;
	}
	memset(hu, 0, sizeof(struct hci_uart));

	serial_setdisc(tty, (void*)hu);
	hu->tty = tty;

//	spin_lock_init(&hu->rx_lock);
	cyg_spinlock_init(&hu->rx_lock, 0);
	

#if 0
	/* Flush any pending characters in the driver and line discipline */
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);

	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
	
	MOD_INC_USE_COUNT;
#else
	cyg_io_get_config(tty, CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH, NULL, NULL);
#endif
	return 0;
}

/* hci_uart_tty_close()
 *
 *    Called when the line discipline is changed to something
 *    else, the tty is closed, or the tty detects a hangup.
 */
static void hci_uart_tty_close(cyg_io_handle_t tty)
{
	struct hci_uart *hu = (struct hci_uart *)serial_getdisc(tty);//(void *)tty->disc_data;

	BT_DBG("tty %p", tty);

	/* Detach from the tty */
	//tty->disc_data = NULL;
	serial_setdisc(tty, NULL);

	if (hu) {
		struct hci_dev *hdev = &hu->hdev;
		hci_uart_close(hdev);

		if (test_and_clear_bit(HCI_UART_PROTO_SET, &hu->flags)) {
			hu->proto->close(hu);
			hci_unregister_dev(hdev);
		}

		//MOD_DEC_USE_COUNT;
	}
}

/* hci_uart_tty_wakeup()
 *
 *    Callback for transmit wakeup. Called when low level
 *    device driver can accept more send data.
 *
 * Arguments:        tty    pointer to associated tty instance data
 * Return Value:    None
 */
static void hci_uart_tty_wakeup(cyg_io_handle_t tty)
{
	struct hci_uart *hu = (struct hci_uart *)serial_getdisc(tty);//(void *)tty->disc_data;

//	BT_DBG("");

	if (!hu)
		return;

//	clear_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	if (tty != hu->tty)
		return;

	if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
		hci_uart_tx_wakeup(hu);
}

/* hci_uart_tty_room()
 * 
 *    Callback function from tty driver. Return the amount of 
 *    space left in the receiver's buffer to decide if remote
 *    transmitter is to be throttled.
 *
 * Arguments:        tty    pointer to associated tty instance data
 * Return Value:    number of bytes left in receive buffer
 */
static int hci_uart_tty_room (cyg_io_handle_t tty)
{
	return 65536;
}

/* hci_uart_tty_receive()
 * 
 *     Called by tty low level driver when receive data is
 *     available.
 *     
 * Arguments:  tty          pointer to tty isntance data
 *             data         pointer to received data
 *             flags        pointer to flags for data
 *             count        count of received data in bytes
 *     
 * Return Value:    None
 */
static void hci_uart_tty_receive(cyg_io_handle_t tty, const cyg_uint8 *data, char *flags, int count)
{
	struct hci_uart *hu = (struct hci_uart *)serial_getdisc(tty);//(void *)tty->disc_data;
	cyg_uint8 *p;
		
	if (!hu || tty != hu->tty)
		return;

	if (!test_bit(HCI_UART_PROTO_SET, &hu->flags))
		return;

	//spin_lock(&hu->rx_lock);
	cyg_spinlock_spin(&hu->rx_lock);

p = (cyg_uint8*)data;
printf("ldisc: %x,%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x,count=%d,rx_count=%d",p,
			p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11],
			p[12],p[13],p[14],p[15],count);

	hu->proto->recv(hu, (void *) data, count);
	hu->hdev.stat.byte_rx += count;
	//spin_unlock(&hu->rx_lock);
	cyg_spinlock_clear(&hu->rx_lock);

#if 0
	if (test_and_clear_bit(TTY_THROTTLED,&tty->flags) && tty->driver.unthrottle)
		tty->driver.unthrottle(tty);
#endif
}

static int hci_uart_register_dev(struct hci_uart *hu)
{
	struct hci_dev *hdev;

	//BT_DBG("");

	/* Initialize and register HCI device */
	hdev = &hu->hdev;

	hdev->type = HCI_UART;
	hdev->driver_data = hu;

	hdev->open  = hci_uart_open;
	hdev->close = hci_uart_close;
	hdev->flush = hci_uart_flush;
	hdev->send  = hci_uart_send_frame;
	hdev->destruct = hci_uart_destruct;

	if (hci_register_dev(hdev) < 0) {
		//BT_ERR
		printf("Can't register HCI device %s\n", hdev->name);
		return -ENODEV;
	}
	//MOD_INC_USE_COUNT;
	return 0;
}

static int hci_uart_set_proto(struct hci_uart *hu, int id)
{
	struct hci_uart_proto *p;
	int err;	

	p = hci_uart_get_proto(id);
	if (!p)
		return -EPROTONOSUPPORT;
//diag_printf("hci_uart_set_proto02,p=%x\n",p);
	err = p->open(hu);
	if (err)
		return err;

	hu->proto = p;
//diag_printf("open h4 success\n");
	err = hci_uart_register_dev(hu);
	if (err) {
		p->close(hu);
		return err;
	}

	return 0;
}

/* hci_uart_tty_ioctl()
 *
 *    Process IOCTL system call for the tty device.
 *
 * Arguments:
 *
 *    tty        pointer to tty instance data
 *    file       pointer to open file object for device
 *    cmd        IOCTL command code
 *    arg        argument for IOCTL call (cmd dependent)
 *
 * Return Value:    Command dependent
 */
static int hci_uart_tty_ioctl(cyg_io_handle_t tty, struct CYG_FILE_TAG * file,
                            unsigned int cmd, unsigned long arg)
{
	struct hci_uart *hu = (struct hci_uart *)serial_getdisc(tty);//(void *)tty->disc_data;
	int err = 0;

//	BT_DBG("");
	/* Verify the status of the device */
	if (!hu)
		return -EBADF;

//diag_printf("hci_uart_tty_ioctl tty=%x,hu=%x\n",tty,hu);

	switch (cmd) {
	case HCIUARTSETPROTO:
		if (!test_and_set_bit(HCI_UART_PROTO_SET, &hu->flags)) {
			err = hci_uart_set_proto(hu, arg);
			if (err) {
				clear_bit(HCI_UART_PROTO_SET, &hu->flags);
				return err;
			}
			//tty->low_latency = 1;
		} else	
			return -EBUSY;
//diag_printf("HCIUARTGETPROTO\n");
	case HCIUARTGETPROTO:
		if (test_bit(HCI_UART_PROTO_SET, &hu->flags))
			return hu->proto->id;

		return -EUNATCH;
		
	default:
		err = cyg_io_ioctl(tty, cmd, (char*)arg);//? arg?
		//err = n_tty_ioctl(tty, file, cmd, arg);
		break;
	};

	return err;
}

/*
 * We don't provide read/write/poll interface for user space.
 */
static cyg_uint32 hci_uart_tty_read(cyg_io_handle_t tty, struct CYG_FILE_TAG *file, unsigned char *buf, size_t nr)
{
	return 0;
}
static cyg_uint32 hci_uart_tty_write(cyg_io_handle_t tty, struct CYG_FILE_TAG *file, const unsigned char *data, size_t count)
{
	return 0;
}
#if 0
static unsigned int hci_uart_tty_poll(cyg_io_handle_t tty, struct CYG_FILE_TAG *filp, poll_table *wait)
{
	return 0;
}
#endif

#ifdef CONFIG_BLUEZ_HCIUART_H4
int h4_init(void);
int h4_deinit(void);
#endif
#ifdef CONFIG_BLUEZ_HCIUART_BCSP
int bcsp_init(void);
int bcsp_deinit(void);
#endif


int hci_uart_init(void)
{
	static struct tty_ldisc hci_uart_ldisc;
	int err;

	//BT_INFO
	diag_printf("BlueZ HCI UART driver ver %s Copyright (C) 2000,2001 Qualcomm Inc\n", 
		VERSION);
	diag_printf("Written 2000,2001 by Maxim Krasnyansky <maxk@qualcomm.com>\n");

	/* Register the tty discipline */

	memset(&hci_uart_ldisc, 0, sizeof (hci_uart_ldisc));
	hci_uart_ldisc.magic       = 0;//TTY_LDISC_MAGIC;
	hci_uart_ldisc.name        = "n_hci";
	hci_uart_ldisc.open        = hci_uart_tty_open;
	hci_uart_ldisc.close       = hci_uart_tty_close;
	hci_uart_ldisc.read        = hci_uart_tty_read;
	hci_uart_ldisc.write       = hci_uart_tty_write;
	hci_uart_ldisc.ioctl       = hci_uart_tty_ioctl;
//	hci_uart_ldisc.poll        = hci_uart_tty_poll;
	hci_uart_ldisc.receive_room= hci_uart_tty_room;
	hci_uart_ldisc.receive_buf = hci_uart_tty_receive;
	hci_uart_ldisc.write_wakeup= hci_uart_tty_wakeup;

	if ((err = serial_register_ldisc(&hci_uart_ldisc))) {
		//BT_ERR
		printf("Can't register HCI line discipline (%d)\n", err);
		return err;
	}

#ifdef CONFIG_BLUEZ_HCIUART_H4
	h4_init();
#endif
#ifdef CONFIG_BLUEZ_HCIUART_BCSP
	bcsp_init();
#endif
	
	return 0;
}

void hci_uart_cleanup(void)
{
	int err;

#ifdef CONFIG_BLUEZ_HCIUART_H4
	h4_deinit();
#endif
#ifdef CONFIG_BLUEZ_HCIUART_BCSP
	bcsp_deinit();
#endif

	/* Release tty registration of line discipline */
	if ((err = serial_unregister_ldisc(NULL)))
		//BT_ERR
		printf("Can't unregister HCI line discipline (%d)\n", err);
}
#endif
