/**************************************************************************
 * FreeDOS 32 FAT Driver                                                  *
 * by Salvo Isaja                                                         *
 *                                                                        *
 * Copyright (C) 2001-2003, Salvatore Isaja                               *
 *                                                                        *
 * This is "fatinit.c" - Driver initialization for the FreeDOS 32 kernel  *
 *                                                                        *
 *                                                                        *
 * This file is part of the FreeDOS 32 FAT Driver.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is free software; you can redistribute it    *
 * and/or modify it under the terms of the GNU General Public License     *
 * as published by the Free Software Foundation; either version 2 of the  *
 * License, or (at your option) any later version.                        *
 *                                                                        *
 * The FreeDOS 32 FAT Driver is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with the FreeDOS 32 FAT Driver; see the file COPYING;            *
 * if not, write to the Free Software Foundation, Inc.,                   *
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                *
 **************************************************************************/

#include "pkgconf/hal.h"
#include "pkgconf/kernel.h"
#include "pkgconf/io_fileio.h"

#include "cyg/kernel/ktypes.h"         // base kernel types
#include "fat.h"
#include "cyg/io/devtab.h"              // device subsystem
#include <stdlib.h>
#include "dirent.h"

/* These are from fd32/include/kernel.h */
/* TODO: Include the file cleanly */
#define SUBSTITUTE 1
#define ADD 0
int add_call(char *name, DWORD address, int mode);

#if 0
/* FAT Driver entry point. Called by the FD32 kernel. */
int fat_init()
{
  int Res;
  diag_printf("Installing the FAT Driver...\n");
  /* Register the FAT Driver to the File System Layer */
  if ((Res = fd32_add_fs(fat_request)) < 0) return Res;
  /* Register the FAT Driver request function to the kernel symbol table */
  if (add_call("fat_request", (DWORD) fat_request, ADD) == -1)
  {
    diag_printf("Couldn't add 'fat_request' to the symbol table\n");
    return FD32_ENOMEM; /* TODO: Check if ENOMEM is true... */
  }
  diag_printf("FAT Driver installed.\n");
  return 0;
}
#endif
//==========================================================================
// Forward definitions

// Filesystem operations
static int fat12_mount(cyg_fstab_entry * fste, cyg_mtab_entry * mte);
static int fat16_mount(cyg_fstab_entry * fste, cyg_mtab_entry * mte);
static int fatall_umount(cyg_mtab_entry * mte);
static int fatall_open(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		      int mode, cyg_file * fte);
static int fatall_ops_unlink(cyg_mtab_entry * mte, cyg_dir dir,
			    const char *name);
static int fatall_ops_mkdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name);
static int fatall_ops_rmdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name);
static int fatall_ops_rename(cyg_mtab_entry * mte, cyg_dir dir1,
			    const char *name1, cyg_dir dir2, const char *name2);
static int fatall_ops_link(cyg_mtab_entry * mte, cyg_dir dir1, const char *name1,
			  cyg_dir dir2, const char *name2, int type);
static int fatall_opendir(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
			 cyg_file * fte);
static int fatall_chdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		       cyg_dir * dir_out);
static int fatall_stat(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		      struct stat *buf);
static int fatall_getinfo(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
			 int key, void *buf, int len);
static int fatall_setinfo(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
			 int key, void *buf, int len);

// File operations
static int fatall_fo_read(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatall_fo_write(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatall_fo_lseek(struct CYG_FILE_TAG *fp, off_t * pos, int whence);
static int fatall_fo_ioctl(struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
			  CYG_ADDRWORD data);
static int fatall_fo_fsync(struct CYG_FILE_TAG *fp, int mode);
static int fatall_fo_close(struct CYG_FILE_TAG *fp);
static int fatall_fo_fstat(struct CYG_FILE_TAG *fp, struct stat *buf);
static int fatall_fo_getinfo(struct CYG_FILE_TAG *fp, int key, void *buf,
			    int len);
static int fatall_fo_setinfo(struct CYG_FILE_TAG *fp, int key, void *buf,
			    int len);

// Directory operations
static int fatall_fo_dirread(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatall_fo_dirlseek(struct CYG_FILE_TAG *fp, off_t * pos, int whence);


static int truename(char *Out,cyg_dir dir, const char *name);

//==========================================================================
// Filesystem table entries

// -------------------------------------------------------------------------
// Fstab entry.
// This defines the entry in the filesystem table.
// For simplicity we use _FILESYSTEM synchronization for all accesses since
// we should never block in any filesystem operations.
#pragma arm section rwdata = "fstab"

struct cyg_fstab_entry fat12_fste  =                 
{                                                                       
    "fat12",                                                              
    0,                                                              
    CYG_SYNCMODE_FILE_FILESYSTEM | CYG_SYNCMODE_IO_FILESYSTEM,
    fat12_mount,                                                             
    fatall_umount,
    fatall_open,
    fatall_ops_unlink,
    fatall_ops_mkdir,
    fatall_ops_rmdir,
    fatall_ops_rename,
    NULL,                                                              
	fatall_opendir,
	fatall_chdir,
	NULL,                                                              
    NULL,                                                           
    NULL                                                            
};
struct cyg_fstab_entry fat16_fste  =                 
{                                                                       
    "fat16",                                                              
    0,                                                              
    CYG_SYNCMODE_FILE_FILESYSTEM | CYG_SYNCMODE_IO_FILESYSTEM,
    fat16_mount,                                                             
    fatall_umount,
    fatall_open,
    fatall_ops_unlink,
    fatall_ops_mkdir,
    fatall_ops_rmdir,
    fatall_ops_rename,
    NULL,                                                              
	fatall_opendir,
	fatall_chdir,
	NULL,                                                              
    NULL,                                                           
    NULL                                                            
};
#if 0
FSTAB_ENTRY(fat12_fste, "fat12", 0,
	    CYG_SYNCMODE_FILE_FILESYSTEM | CYG_SYNCMODE_IO_FILESYSTEM,
	    fat12_mount,
	    fatall_umount,
	    fatall_open,
	    fatall_ops_unlink,
	    fatall_ops_mkdir,
	    fatall_ops_rmdir,
	    fatall_ops_rename,
	    NULL,//fatall_ops_link,
	    fatall_opendir,
	    fatall_chdir,
	    NULL,//fatall_stat,
	    NULL,//fatall_getinfo,
	    NULL//fatall_setinfo
);


FSTAB_ENTRY(fat16_fste, "fat16", 0,
	    CYG_SYNCMODE_FILE_FILESYSTEM | CYG_SYNCMODE_IO_FILESYSTEM,
	    fat16_mount,
	    fatall_umount,
	    fatall_open,
	    fatall_ops_unlink,
	    fatall_ops_mkdir,
	    fatall_ops_rmdir,
	    fatall_ops_rename,
	    NULL,//fatall_ops_link,
	    fatall_opendir,
	    fatall_chdir,
	    NULL,//fatall_stat,
	    NULL,//fatall_getinfo,
	    NULL//fatall_setinfo
);
#endif
#pragma arm section rwdata
// -------------------------------------------------------------------------
// File operations.
// This set of file operations are used for normal open files.

static cyg_fileops fatall_fileops = {
	fatall_fo_read,
	fatall_fo_write,
	fatall_fo_lseek,
	NULL,//fatall_fo_ioctl,
	cyg_fileio_seltrue,
	NULL,//fatall_fo_fsync,
	fatall_fo_close,
	NULL,//fatall_fo_fstat,
	NULL,//fatall_fo_getinfo,
	NULL,//fatall_fo_setinfo
};

// -------------------------------------------------------------------------
// Directory file operations.
// This set of operations are used for open directories. Most entries
// point to error-returning stub functions. Only the read, lseek and
// close entries are functional.

static cyg_fileops fatall_dirops = {
	fatall_fo_dirread,
	(cyg_fileop_write *) cyg_fileio_enosys,
	NULL,//fatall_fo_dirlseek,
	(cyg_fileop_ioctl *) cyg_fileio_enosys,
	cyg_fileio_seltrue,
	(cyg_fileop_fsync *) cyg_fileio_enosys,
	fatall_fo_close,
	(cyg_fileop_fstat *) cyg_fileio_enosys,
	(cyg_fileop_getinfo *) cyg_fileio_enosys,
	(cyg_fileop_setinfo *) cyg_fileio_enosys
};

char* itoa(char* dest, int v)
{
    char b[16];
    char* p = &b[16];

    *--p = 0;
    if (v) {
        while (v&&(b-p)<16){
            *--p = (v % 10) + '0';
            v = v / 10;
        }
    } else
        *--p = '0';

    return strcpy(dest, p);
}

static int fat12_mount(cyg_fstab_entry * fste, cyg_mtab_entry * mte)
{
	extern cyg_mtab_entry *mtab, *mtab_end;
	cyg_mtab_entry *m;
	cyg_io_handle_t t;
	Cyg_ErrNo err;
	fd32_mount_t M;

	D2(printf("fat12_mount\n"));
	//printf("name = %s\n",fat12_fste.name);
		
	err = cyg_io_lookup(mte->devname, &t);
	if (err != ENOERR)
		return -err;

	// Iterate through the mount table to see if we're mounted
	// FIXME: this should be done better - perhaps if the superblock
	// can be stored as an inode in the icache.
	for (m = &mtab[0]; m != mtab_end; m++) {
		// stop if there are more than the configured maximum
		if (m - &mtab[0] >= CYGNUM_FILEIO_MTAB_MAX) {
			m = mtab_end;
			break;
		}
		if (m->valid && strcmp(m->fsname, "fat12") == 0 &&
		    strcmp(m->devname, mte->devname) == 0) {
			mte->data = m->data;
			return ENOERR;
		}
	}
	
	M.Size = sizeof(fd32_mount_t);
	M.hDev = (DWORD)t;

	if (fat_request(FD32_MOUNT, &M) != ENOERR)
		return -EINVAL;
	mte->data = (CYG_ADDRWORD)M.FsDev;
	mte->root = (cyg_dir) mte->name;
//	D2(printf("fat_mounted superblock at %x\n", mte->root));

	return ENOERR;
}


static int fat16_mount(cyg_fstab_entry * fste, cyg_mtab_entry * mte)
{
	extern cyg_mtab_entry *mtab, *mtab_end;
	cyg_mtab_entry *m;
	cyg_io_handle_t t;
	Cyg_ErrNo err;
	fd32_mount_t M;

	D2(printf("fat16_mount\n"));
	printf("name = %s\n",fat16_fste.name);

	err = cyg_io_lookup(mte->devname, &t);
	if (err != ENOERR)
		return -err;

	// Iterate through the mount table to see if we're mounted
	// FIXME: this should be done better - perhaps if the superblock
	// can be stored as an inode in the icache.
	for (m = &mtab[0]; m != mtab_end; m++) {
		// stop if there are more than the configured maximum
		if (m - &mtab[0] >= CYGNUM_FILEIO_MTAB_MAX) {
			m = mtab_end;
			break;
		}
		if (m->valid && strcmp(m->fsname, "fat16") == 0 &&
		    strcmp(m->devname, mte->devname) == 0) {
			mte->data = m->data;
			return ENOERR;
		}
	}
	
	M.Size = sizeof(fd32_mount_t);
	M.hDev = (DWORD)t;

	if (fat_request(FD32_MOUNT, &M) != ENOERR)
		return -EINVAL;
	mte->data = (CYG_ADDRWORD)M.FsDev;
//	mte->root = (cyg_dir) jffs2_sb->s_root;
//	D2(printf("fat_mounted superblock at %x\n", mte->root));

	return ENOERR;
}

static int fatall_umount(cyg_mtab_entry * mte)
{
	tVolume *V = (tVolume *)mte->data;
	fd32_unmount_t M;
	
	M.Size = sizeof(fd32_unmount_t);
	M.DeviceId = V;
	
	mte->root = CYG_DIR_NULL;
	mte->fs->data = 0;	// fstab entry, visible to all mounts. No current mount
	
	if (fat_request(FD32_UNMOUNT, &M) != ENOERR)
		return -EINVAL;
	if(V->BlkDev)
		free(V->BlkDev);
	return ENOERR;
}


static int fatall_open(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		      int mode, cyg_file * file)
{
	int iMode=0,ret;
	fd32_openfile_t Of;
	tFile *F;
	char            AuxName[FD32_LFNPMAX];

	if( mode&O_CREAT )
		iMode |= FD32_OCREAT;
	else
		iMode |= FD32_OEXIST;// | FD32_ODIR;
	
	if( (mode&O_ACCMODE) == O_ACCMODE )
		iMode |= FD32_OACCESS;
	else if( (mode&O_RDWR) == O_RDWR )
		iMode |= FD32_ORDWR;
	else if( mode&O_RDONLY )
		iMode |= FD32_OREAD;
	else if( mode&O_WRONLY )
		iMode |= FD32_OWRITE;
	
	if( mode&O_TRUNC )
		iMode |= FD32_OTRUNC;
	
	ret = truename(AuxName, dir, name);
	if(ret != ENOERR )
	{
		printf("fatall_open: ret=%x,AuxName=%s,dir=%s,name=%s\n",ret,AuxName,dir,name);
		return ret;
	}
	
	Of.Size      = sizeof(fd32_openfile_t);
    Of.DeviceId  = (void*)mte->data;
    Of.FileName  = (char *)AuxName;
    Of.Mode      = iMode;
    Of.Attr      = FD32_ANONE;
    Of.AliasHint = 0;
	
	ret = fat_request(FD32_OPENFILE, &Of);
	if(ret < 0)
	{
		printf("fatall_open:ret=%d iMode=0x%x %s\n",ret,iMode,AuxName);
		return ret;
	}
	if( (mode&O_CREAT) && (ret != FD32_ORCREAT))
		return -EINVAL;

	if( (mode&O_TRUNC) && (ret != FD32_ORTRUNC))
		return -EINVAL;
	if(!(mode&O_TRUNC) && !(mode&O_CREAT) &&(ret != FD32_OROPEN))
		return -EINVAL;

	F = (tFile *)Of.FileId;
	F->V = (tVolume*)mte->data;
	file->f_flag |= mode & CYG_FILE_MODE_MASK;
	file->f_type = CYG_FILE_TYPE_FILE;
	file->f_ops = &fatall_fileops;
	file->f_offset = 0;
	file->f_data = (CYG_ADDRWORD) F;
	file->f_xops = 0;

	return ENOERR;
}

static int fatall_fo_read(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
	int ret,i;
	fd32_read_t R;
	ssize_t resid = uio->uio_resid;

	R.Size        = sizeof(fd32_read_t);
  	R.DeviceId    = (void*)fp->f_data;
  	
  	for (i = 0; i < uio->uio_iovcnt; i++) {
		int ret;
		cyg_iovec *iov = &uio->uio_iov[i];
		off_t len = iov->iov_len;

		D2(printf("fatall_fo_read size %d\n", len));

		
		R.Buffer  = iov->iov_base;
  		R.BufferBytes = len;
		if ((ret = fat_request(FD32_READ, &R)) < 0)//!= ENOERR)
		{
			D1(printf
			   ("fatall_fo_read(): failed %d\n", ret));
			uio->uio_resid = resid;
	//		up(&f->sem);
			return -ret;
		}
		resid -= ret;
		if(ret != len)
		{
			uio->uio_resid = resid;
			return ENOERR;
		}
	}

	// We successfully read some data, update the node's access time
	// and update the file offset and transfer residue.

	//inode->i_atime = cyg_timestamp();

	uio->uio_resid = resid;
	//fp->f_offset = pos;
  	
  	
	return ENOERR;
}
static int fatall_fo_write(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
	fd32_write_t  W;
	ssize_t resid = uio->uio_resid;
	int i,err;
	
	W.Size        = sizeof(fd32_write_t);
	W.DeviceId    = (void*)fp->f_data;
	
	// Now loop over the iovecs until they are all done, or
	// we get an error.
	for (i = 0; i < uio->uio_iovcnt; i++) {
		cyg_iovec *iov = &uio->uio_iov[i];
		char *buf = (char *) iov->iov_base;
		off_t len = iov->iov_len;


		D2(printf("fatall_fo_write transfer size %d\n", len));

		W.Buffer      = buf;
		W.BufferBytes = len;
		if ((err=fat_request(FD32_WRITE, &W)) < 0)
			return err;

		// Update working vars
		
		resid -= err;
		if(err != len)
		{
			uio->uio_resid = resid;
			return ENOMEM;
		}
		
	}

	uio->uio_resid = resid;
	//fp->f_offset = pos;

	return ENOERR;
}

//static int fatall_fo_close(struct CYG_FILE_TAG *fp)
static int fatall_fo_close(cyg_file *fp)
{
	fd32_close_t C;
	int err;
	tFile *F= (tFile*)fp->f_data;
	tVolume *V = F->V;

	C.Size     = sizeof(fd32_close_t);
	C.DeviceId = (void*)fp->f_data;

	if ((err=fat_request(FD32_CLOSE, &C)) != ENOERR)
				return err;
	fp->f_data = 0;

	return ENOERR;
}
static int fatall_ops_mkdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name)
{
	fd32_mkdir_t    Md;
	char            AuxName[FD32_LFNPMAX];
	int             Res;
	char *__fatdir = (char *)dir;
	int len = (__fatdir==NULL) ? 0 : strlen(__fatdir);

	Res = truename(AuxName, dir, name);
	if(Res != ENOERR)
	{
		printf("truename error Res=%d,dir=%s,name=%s\n",Res,dir,name);
		return Res;
	}
	Md.Size = sizeof(fd32_mkdir_t);
	Md.DeviceId = (tVolume*)mte->data;
	Md.DirName = AuxName;
	Res = fat_request(FD32_MKDIR, &Md);
	if(Res < 0)
		return Res;
	return ENOERR;
}

static char fatdir1[FD32_LFNPMAX];
static char fatdir2[FD32_LFNPMAX];

static int fatall_chdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		       cyg_dir * dir_out)
{
	char *dest = fatdir1;
	int Res;
	
	if((cyg_uint32*)dir == (cyg_uint32*)fatdir1)
		dest = fatdir2;
	else if((cyg_uint32*)dir == (cyg_uint32*)fatdir2)
		dest = fatdir1;
	
	Res = truename(dest, dir, name);
	if(Res != ENOERR)
	{
		printf("Res = 0x%x,dir=%s,name=%s,dest=%s\n",Res,dir,name,dest);
		return Res;
	}
	*dir_out = (cyg_dir)dest;
	return ENOERR;
}


static int fatall_ops_rmdir(cyg_mtab_entry * mte, cyg_dir dir, const char *name)
{
  fd32_rmdir_t    Rd;
  char            AuxName[FD32_LFNPMAX];
  int             Res;
  char *__fatdir = (char *)dir;
  char *p;
  int len = strlen(__fatdir);
	
  if(len >= FD32_LFNPMAX)
	return ENOMEM;
  if(*name != '/')
  {
	 if(len != 0)
	 {
		strcpy(AuxName,__fatdir);
		if(AuxName[len -1] != '/')
			AuxName[len++] = '/';
	}
  }
  else
  	len = 0;
  if(len + strlen(name) >= FD32_LFNPMAX)
	return EINVAL;
  strcpy(AuxName + len,name);
  //if ((Res = fd32_truename(AuxName, name, FD32_TNSUBST)) < 0) return Res;
  
  Rd.Size = sizeof(fd32_rmdir_t);
  Rd.DeviceId = (tVolume*)mte->data;
  Rd.DirName = AuxName;
  //for (;;)
  {
    Res = fat_request(FD32_RMDIR, &Rd);
    if (Res < 0) return Res;
    return ENOERR;
  }
}
static int fatall_opendir(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
			 cyg_file * fte)
{
	tFile     *F;
	tDirEntry  D;
	int        Res;
	tVolume *V;
	char            AuxName[FD32_LFNPMAX];
	char *__fatdir = (char *)dir;
	char *p;
	int len = (__fatdir==NULL) ? 0 : strlen(__fatdir);
	
	Res = truename(AuxName, dir, name);
	if(Res != 0)
		return Res;
	
	D2(diag_printf("FAT opendir: %s,dir = %x\n", AuxName,dir));
	
	D2(diag_printf("FAT opendir: __fatdir = %s\n", __fatdir));

	V = (tVolume *)mte->data;
	/* First we open the directory to see if removing it is a valid operation */
	Res = fat_open(V, (char *)name, FD32_OREAD | FD32_ODIR | FD32_OEXIST,
                 FD32_ANONE, 0, &F);
	if (Res < 0) return Res;

	F->V = (tVolume   *)mte->data;
	fte->f_type = CYG_FILE_TYPE_FILE;
	fte->f_ops = &fatall_dirops;
	fte->f_offset = 0;
	fte->f_data = (CYG_ADDRWORD) F;
	fte->f_xops = 0;
	return ENOERR;
}
static int fatall_fo_dirread(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
	fd32_readdir_t Rd;
	fd32_fs_lfnfind_t entry;
	struct dirent *ent = (struct dirent *) uio->uio_iov[0].iov_base;
	char *nbuf = ent->d_name;
	int nlen = sizeof (ent->d_name) - 1;
	off_t len = uio->uio_iov[0].iov_len;
	
	int Res;
	
	Rd.Size  = sizeof(fd32_readdir_t);
	Rd.DirId = (void*)fp->f_data;;
	Rd.Entry = (void *) &entry;
	
	D2(printf("fatall_fo_dirread size %d\n", len));
	
  	if ((Res = fat_request(FD32_READDIR, &Rd)) == 0)
  	{
  		if(strlen(entry.LongName) != 0)
  			strcpy(nbuf, entry.LongName);
  		else if(strlen(entry.ShortName) != 0)
  			strcpy(nbuf, entry.ShortName);
  		uio->uio_resid -= sizeof (struct dirent);
 
		return ENOERR;
	}
	if (Res == FD32_ENMFILE)
		return ENOERR;
printf("Res = %x,len = %d\n",Res,len);
	// We successfully read some data, update the node's access time
	// and update the file offset and transfer residue.

	//inode->i_atime = cyg_timestamp();

	return EINVAL;

}
static int fatall_ops_unlink(cyg_mtab_entry * mte, cyg_dir dir,const char *name)
{
	fd32_unlink_t   U;
	char            AuxName[FD32_LFNPMAX];
	int             Res;
	
	Res = truename(AuxName, dir, name);
	if(Res != ENOERR)
		return Res;

	U.Size  = sizeof(fd32_unlink_t);
  	U.Flags = FD32_FAALL | FD32_FRNONE;
  	U.DeviceId = (void*)mte->data;
  	U.FileName = AuxName;
  	Res = fat_request(FD32_UNLINK, &U);

  	if( Res != ENOERR)
  		return Res;
  	return ENOERR;
}

int truename(char *Out,cyg_dir dir, const char *name)
{
	char *__fatdir = (char *) dir;
	int len = (__fatdir) ? strlen(__fatdir) : 0;

	if(!name && !len)
		return EINVAL;

	if(name && (*name == '/' || !len))
		strcpy(Out,name);
	else if (!name && len)
		strcpy(Out,__fatdir);
	else
	{
		strcpy(Out,__fatdir);
		if(Out[len - 1] != '/')
			Out[len++] = '/';
		if(len + strlen(name) > FD32_LFNPMAX)
		{
			printf("%d,FD32_LFNPMAX = %d\n",len+strlen(name),FD32_LFNPMAX);
			return ENOMEM;
		}
		strcpy(Out+len,name);
	}
	if(name)
		Out[len+strlen(name)]= 0;
	else
		Out[len] = 0;
	return ENOERR;
}

static int fatall_ops_rename(cyg_mtab_entry * mte, cyg_dir dir1,
			    const char *name1, cyg_dir dir2, const char *name2)
{
	fd32_rename_t   R;
	char            AuxOld[FD32_LFNPMAX];
	char            AuxNew[FD32_LFNPMAX];
	int             Res;
	
	Res = truename(AuxOld,dir1,name1);
	if(Res != ENOERR)
		return Res;
	Res = truename(AuxNew,dir2,name2);
	if(Res != ENOERR)
		return Res;
	R.Size     = sizeof(fd32_rename_t);
    R.DeviceId = (void*)mte->data;
    R.OldName = AuxOld;
    R.NewName = AuxNew;
    
    Res = fat_request(FD32_RENAME, &R);
    if(Res != ENOERR)
    {
    	printf("fatall_ops_rename:Res=%x,AuxOld=%s,AuxNew=%s\n",Res,AuxOld,AuxNew);
    	return Res;
    }
    
    return ENOERR;
}

static int fatall_ops_link(cyg_mtab_entry * mte, cyg_dir dir1, const char *name1,
			  cyg_dir dir2, const char *name2, int type)
{
	char            AuxName[FD32_LFNPMAX];
	int             Res;
	Res = truename(AuxName, dir1, name1);
	if(Res != ENOERR)
		return Res;
	
}
static int fatall_fo_lseek(struct CYG_FILE_TAG *fp, off_t * pos, int whence)
{
	fd32_lseek_t  S;
	int           Res;
	S.Size     = sizeof(fd32_lseek_t);
	S.DeviceId = (void*)fp->f_data;
	S.Offset   = *pos;
	S.Origin   = whence;
	Res = fat_request(FD32_LSEEK, &S);
	if(Res != ENOERR)
	{
		printf("fatall_fo_lseek Res=%x,whence=%d,pos=%d\n",Res,whence,*pos);
		return Res;
	}
	return ENOERR;
}
static int fatall_stat(cyg_mtab_entry * mte, cyg_dir dir, const char *name,
		      struct stat *buf)
{
	buf->st_mode = 0;
	buf->st_ino = 0;
	buf->st_dev = 0;
	buf->st_nlink = 0;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_size = 0;
	buf->st_atime = 0;
	buf->st_mtime = 0;
	buf->st_ctime = 0;
	return ENOERR;
}
