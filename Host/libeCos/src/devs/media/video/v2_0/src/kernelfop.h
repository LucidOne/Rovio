#ifndef __KERNLFOP_H__
#define __KERNLFOP_H__

#include "linux/fs.h" 
#include "linux/sched.h" 
#include "linux/file.h"
#include "asm/processor.h" 
#include "asm/uaccess.h" 

#define EOF (-1) 
#define SEEK_SET 0 
#define SEEK_CUR 1 
#define SEEK_END 2 

struct file *klib_fopen(const char *filename, int flags, int mode); 
void klib_fclose(struct file *filp); 
int klib_fseek(struct file *filp, int offset, int whence); 
int klib_fread(char *buf, int len, struct file *filp); 
int klib_fgetc(struct file *filp); 
char *klib_fgets(char *str, int size, struct file *filp); 
int klib_fwrite(char *buf, int len, struct file *filp); 
int klib_fputc(int ch, struct file *filp); 
int klib_fputs(char *str, struct file *filp); 
int klib_fprintf(struct file *filp, const char *fmt, ...); 

#endif