/** @file wlan_debug.c
  * @brief This file contains functions for debug proc file.
  * 
  *  Copyright © Marvell International Ltd. and/or its affiliates, 2003-2006
  */
/********************************************************
Change log:
	10/04/05: Add Doxygen format comments
	01/05/06: Add kernel 2.6.x support	
	
********************************************************/

#include  "include.h"
#include  "sys/malloc.h"

/********************************************************
		Local Variables
********************************************************/

#define item_size(n) (sizeof ((wlan_adapter *)0)->n)
#define item_addr(n) ((cyg_uint32) &((wlan_adapter *)0)->n)

#define item1_size(n) (sizeof ((wlan_dev_t *)0)->n)
#define item1_addr(n) ((cyg_uint32) &((wlan_dev_t *)0)->n)
struct debug_data
{
    char name[32];
    cyg_uint32 size;
    cyg_uint32 addr;
    cyg_uint32 offset;
};

/* To debug any member of wlan_adapter or wlan_dev_t, simply add one line here.
 */
#define ITEMS_FROM_WLAN_DEV		1

static struct debug_data items[] = {
    {"IntCounter", item_size(IntCounter), 0, item_addr(IntCounter)},
    {"PSMode", item_size(PSMode), 0, item_addr(PSMode)},
    {"PSState", item_size(PSState), 0, item_addr(PSState)},
    {"dnld_sent", item1_size(dnld_sent), 0, item1_addr(dnld_sent)},
};

static int num_of_items = sizeof(items) / sizeof(items[0]);

/********************************************************
		Global Variables
********************************************************/

/********************************************************
		Local Functions
********************************************************/

/** 
 *  @brief convert string to number
 *
 *  @param s   	   pointer to numbered string
 *  @return 	   converted number from string s
 */
int
string_to_number(char *s)
{
    int r = 0;
    int base = 0;

    if ((strncmp(s, "0x", 2) == 0) || (strncmp(s, "0X", 2) == 0))
        base = 16;
    else
        base = 10;
    if (base == 16)
        s += 2;
    for (s = s; *s != 0; s++) {
        if ((*s >= 48) && (*s <= 57))
            r = (r * base) + (*s - 48);
        else if ((*s >= 65) && (*s <= 70))
            r = (r * base) + (*s - 55);
        else if ((*s >= 97) && (*s <= 102))
            r = (r * base) + (*s - 87);
        else
            break;
    }

    return r;
}

/** 
 *  @brief proc read function
 *
 *  @param page	   pointer to buffer
 *  @param s       read data starting position
 *  @param off     offset
 *  @param cnt     counter 
 *  @param eof     end of file flag
 *  @param data    data to output
 *  @return 	   number of output data
 */
static int
wlan_debug_read(char *page, char **s, off_t off, int cnt, int *eof,
                void *data)
{
    int val = 0;
    char *p = page;
    int i;

    struct debug_data *d = (struct debug_data *) data;

    MODULE_GET;

    for (i = 0; i < num_of_items; i++) {
        if (d[i].size == 1)
            val = *((cyg_uint8 *) d[i].addr);
        else if (d[i].size == 2)
            val = *((cyg_uint16 *) d[i].addr);
        else if (d[i].size == 4)
            val = *((cyg_uint32 *) d[i].addr);
        p += sprintf(p, "%s=%d\n", d[i].name, val);
    }
    MODULE_PUT;
    return p - page;
}

/** 
 *  @brief proc write function
 *
 *  @param f	   file pointer
 *  @param buf     pointer to data buffer
 *  @param cnt     data number to write
 *  @param data    data to write
 *  @return 	   number of data
 */
static int
wlan_debug_write(struct file *f, const char *buf, unsigned long cnt,
                 void *data)
{
    int r, i;
    char *pdata;
    char *p;
    char *p0;
    char *p1;
    char *p2;
    struct debug_data *d = (struct debug_data *) data;

    MODULE_GET;

    MALLOC(pdata, (char *), cnt, 0, M_NOWAIT);
    if (pdata == NULL) {
        MODULE_PUT;
        return 0;
    }

    if (copy_from_user(pdata, buf, cnt)) {
        diag_printf("Copy from user failed\n");
        FREE(pdata, 0);
        MODULE_PUT;
        return 0;
    }

    p0 = pdata;
    for (i = 0; i < num_of_items; i++) {
        do {
            p = strstr(p0, d[i].name);
            if (p == NULL)
                break;
            p1 = strchr(p, '\n');
            if (p1 == NULL)
                break;
            p0 = p1++;
            p2 = strchr(p, '=');
            if (!p2)
                break;
            p2++;
            r = string_to_number(p2);
            if (d[i].size == 1)
                *((cyg_uint8 *) d[i].addr) = (cyg_uint8) r;
            else if (d[i].size == 2)
                *((cyg_uint16 *) d[i].addr) = (cyg_uint16) r;
            else if (d[i].size == 4)
                *((cyg_uint32 *) d[i].addr) = (cyg_uint32) r;
            break;
        } while (TRUE);
    }
    FREE(pdata, 0);
    MODULE_PUT;
    return cnt;
}

/********************************************************
		Global Functions
********************************************************/
/** 
 *  @brief create debug proc file
 *
 *  @param priv	   pointer wlan_private
 *  @param dev     pointer net_device
 *  @return 	   N/A
 */
void
wlan_debug_entry(wlan_private * priv, struct net_device *dev)
{
    int i;
    struct proc_dir_entry *r;

    if (priv->proc_entry == NULL)
        return;

    for (i = 0; i < (num_of_items - ITEMS_FROM_WLAN_DEV); i++) {
        items[i].addr = items[i].offset + (cyg_uint32) priv->adapter;
    }
    for (i = num_of_items - ITEMS_FROM_WLAN_DEV; i < num_of_items; i++) {
        items[i].addr = items[i].offset + (cyg_uint32) & priv->wlan_dev;
    }
    r = create_proc_entry("debug", 0644, priv->proc_entry);
    if (r == NULL)
        return;

    r->data = &items[0];
    r->read_proc = wlan_debug_read;
    r->write_proc = wlan_debug_write;
    r->owner = THIS_MODULE;

}

/** 
 *  @brief remove proc file
 *
 *  @param priv	   pointer wlan_private
 *  @return 	   N/A
 */
void
wlan_debug_remove(wlan_private * priv)
{
    remove_proc_entry("debug", priv->proc_entry);
}
