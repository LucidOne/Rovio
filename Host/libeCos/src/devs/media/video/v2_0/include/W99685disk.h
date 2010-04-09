#ifndef __W99685DISK_H__
#define __W99685DISK_H__

#include "linux/stat.h"
#include "linux/fs.h"
#include "linux/errno.h"
//#include "linux/unistd.h"
#include "asm/semaphore.h"


#define DISKSIZE 1024*1024


//long sys_mkdir(const char *name, int mode);
//long sys_newstat(char * filename, struct stat * statbuf);
//long sys_unlink(const char * pathname);

int InitDisk();

typedef struct BigBufferTransfer {
	int TotalLen;
	int Current;
	char *BufferNeeded;
	struct semaphore sem;
}BigBufferTransfer_t;

#endif
