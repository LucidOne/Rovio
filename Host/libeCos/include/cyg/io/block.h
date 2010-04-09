#ifndef __IO_BLOCK_H
#define __IO_BLOCK_H

#include "cyg/io/config_keys.h"
#include "cyg/infra/cyg_type.h"
#define SDRAM_ERR_OK              0x00  // No error - operation complete

typedef struct {
    unsigned int dev_size;
} cyg_io_sdram_getconfig_devsize_t;


typedef struct {
    CYG_ADDRESS offset;
    int block_size;
} cyg_io_sdram_getconfig_blocksize_t;

struct sdram_info {
    void *work_space;
    int   work_space_size;
    int   block_size;   // Assuming fixed size "blocks"
    int   blocks;       // Number of blocks
    int   buffer_size;  // Size of write buffer (only defined for some devices)
    unsigned long block_mask;
    void *start, *end;  // Address range
    int   init;
};
#endif
