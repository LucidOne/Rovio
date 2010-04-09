#include "linux/errno.h"
#include "cyg/infra/cyg_type.h"
#include "cyg/io/devtab.h"
#include "video/w99685.h"

//extern struct sdram_info sdram_info;


static Cyg_ErrNo
w99685iodev_read( cyg_io_handle_t handle, void *buf, cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    w685cf_t *priv = (w685cf_t *)t->priv;
    int err;
    
    err = priv->read(priv, buf, len);
    
    return err;
} // flashiodev_bread()

static Cyg_ErrNo
w99685iodev_write( cyg_io_handle_t handle, const void *buf, cyg_uint32 *len)
{   
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    w685cf_t *priv = (w685cf_t *)t->priv;
    int err;
    
    err = priv->write(priv, buf, len);
    
    return err;
} // flashiodev_bwrite()

static Cyg_ErrNo
w99685iodev_get_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       void* buf,
                       cyg_uint32* len)
{
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    w685cf_t *priv = (w685cf_t *)t->priv;
    int err;
    
    err = priv->ioctl(priv, key, buf);
	return err;
}


static Cyg_ErrNo 
w99685iodev_set_config(cyg_io_handle_t handle, cyg_uint32 key, const void *buf, 
                  cyg_uint32 *len)
{
    cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    w685cf_t *priv = (w685cf_t *)t->priv;
    char *pbuf = (char *)buf;
    int err;

    err = priv->ioctl(priv, key, pbuf);
    return err;
}

static Cyg_ErrNo 
w99685iodev_ioctl(cyg_io_handle_t handle, cyg_uint32 key, const void *buf)
{
	
	cyg_devtab_entry_t *t = (cyg_devtab_entry_t *)handle;
    w685cf_t *priv = (w685cf_t *)t->priv;
    char *pbuf = (char *)buf;
    int err;
    
    err = priv->ioctl(priv, key, pbuf);
    return err;
}

// get_config/set_config should be added later to provide the other flash
// operations possible, like erase etc.

//YC
#if 1
cyg_devio_table_t cyg_io_w99685_ops = { 
    &w99685iodev_write,                                                    
    &w99685iodev_read,   
    cyg_devio_bwrite,                                           
    cyg_devio_bread,      
    0,                                                    
    &w99685iodev_get_config,                                                
    &w99685iodev_set_config,
    &w99685iodev_ioctl
};



#else
DEVIO_TABLE( cyg_io_w99685_ops,
                   &w99685iodev_write,
                   &w99685iodev_read,
                   0, // no select
                   &w99685iodev_get_config,
                   &w99685iodev_set_config,
                   &w99685iodev_ioctl
);

#define CHAR_DEVIO_TABLE(_l,_write,_read,_select,_get_config,_set_config)    \
cyg_devio_table_t _l= {                                        \
    _write,                                                     \
    _read,                                                      \
    cyg_devio_bwrite,                                           \
    cyg_devio_bread,                                            \
    _select,                                                    \
    _get_config,                                                \
    _set_config,                                                \
};

#endif               






