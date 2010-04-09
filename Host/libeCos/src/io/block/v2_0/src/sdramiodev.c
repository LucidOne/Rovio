#include "cyg/io/block.h"
#include "cyg/error/codes.h"
#include "pkgconf/io_sdram.h"
#include "cyg/infra/cyg_type.h"
#include "cyg/io/devtab.h"
#include "string.h"

static int dummy_printf( const char *fmt, ... ) {return 0;}
extern struct sdram_info sdram_info;
extern int sdram_init(void);

static Cyg_ErrNo
sdramiodev_bread( cyg_io_handle_t handle, void *buf, cyg_uint32 *len,
                  cyg_uint32 pos)
{
    char *startpos = (char *)sdram_info.start + pos + 
        CYGNUM_IO_SDRAM_BLOCK_OFFSET_1;
//diag_printf("read startpos=0x%x,len=%d\n",startpos,*len);
    memcpy( buf, startpos, (*len) );
    
    return ENOERR;
} // flashiodev_bread()

static Cyg_ErrNo
sdramiodev_bwrite( cyg_io_handle_t handle, const void *buf, cyg_uint32 *len,
                   cyg_uint32 pos )
{   
    Cyg_ErrNo err = ENOERR;
    char *startpos = (char *)sdram_info.start + pos + CYGNUM_IO_SDRAM_BLOCK_OFFSET_1;

//diag_printf("write startpos=0x%x,len=%d\n",startpos,*len);

//*(volatile unsigned int *)0xfff80000='N';

    memcpy(startpos,buf, (*len) );

    if ( err )
        err = -EIO; // just something sane
    return err;
} // flashiodev_bwrite()

static Cyg_ErrNo
sdramiodev_get_config( cyg_io_handle_t handle,
                       cyg_uint32 key,
                       void* buf,
                       cyg_uint32* len)
{
    switch (key) {
    
    case CYG_IO_GET_CONFIG_SDRAM_DEVSIZE:
    {
        if ( *len != sizeof( cyg_io_sdram_getconfig_devsize_t ) )
             return -EINVAL;
        {
            cyg_io_sdram_getconfig_devsize_t *d =
                (cyg_io_sdram_getconfig_devsize_t *)buf;

	    //d->dev_size = flash_info.blocks * flash_info.block_size;
	    d->dev_size = CYGNUM_IO_SDRAM_BLOCK_LENGTH_1;
        }
        return ENOERR;
    }
    case CYG_IO_GET_CONFIG_SDRAM_BLOCKSIZE:
    {
        if ( *len != sizeof( cyg_io_sdram_getconfig_blocksize_t ) )
             return -EINVAL;
        {
            cyg_io_sdram_getconfig_blocksize_t *d =
                (cyg_io_sdram_getconfig_blocksize_t *)buf;

	    //d->dev_size = flash_info.blocks * flash_info.block_size;
	    d->block_size = sdram_info.block_size;
        }
        return ENOERR;
    }

    default:
        return -EINVAL;
    }
} // flashiodev_get_config()


static bool sdramiodev_init(struct cyg_devtab_entry *tab)
{
    int stat = sdram_init();
//diag_printf("sdramiodev_init\n");
    if ( stat == 0 )
        return true;
    else
        return false;
}

// get_config/set_config should be added later to provide the other flash
// operations possible, like erase etc.

#if 1
cyg_devio_table_t cyg_io_sdram_ops = {                                        
    cyg_devio_cwrite,                                           
    cyg_devio_cread,                                            
    &sdramiodev_bwrite,                                                    
    &sdramiodev_bread,                                                     
    0,                                                    
    &sdramiodev_get_config,                                                
    0                                                
};
#pragma arm section rwdata = "devtab"
cyg_devtab_entry_t cyg_io_sdram= {
   CYGDAT_IO_SDRAM_BLOCK_DEVICE_NAME_1,
   0,
   &cyg_io_sdram_ops,
   &sdramiodev_init,
   0,
   NULL,
   CYG_DEVTAB_STATUS_BLOCK
};
#pragma arm section rwdata
#else
BLOCK_DEVIO_TABLE( cyg_io_sdram_ops,
                   &sdramiodev_bwrite,
                   &sdramiodev_bread,
                   0,
                   &sdramiodev_get_config,
                   0 );
                   

BLOCK_DEVTAB_ENTRY( cyg_io_sdram,
                    CYGDAT_IO_SDRAM_BLOCK_DEVICE_NAME_1,
                    0,
                    &cyg_io_sdram_ops,
                    &sdramiodev_init,
                    0, // No lookup required
                    NULL );
#endif