#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyg/kernel/kapi.h"
#if 0

typedef struct
{
	int init;
	cyg_mutex_t ptm;
} my_pthread_mutex_t;

#define MY_PTHREAD_MUTEX_INITIALIZER {0, }

static int my_pthread_mutex_lock(my_pthread_mutex_t *mutex)
{
	if (!mutex->init)
	{
		mutex->init = 1;
		cyg_mutex_init(&mutex->ptm);
	}

	return cyg_mutex_lock(&mutex->ptm);
}

static int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
	cyg_mutex_unlock(&mutex->ptm);
	return 0;
}


typedef struct my_mem_list_struct
{
	void *ptr;
	int isize;
	int iline;
	char *pcfile;
	struct my_mem_list_struct *pnext;
} my_mem_list_struct_t;

my_mem_list_struct_t *g_pmemheader = NULL;
my_pthread_mutex_t g_ptmmem = MY_PTHREAD_MUTEX_INITIALIZER;


#define MEM_START_MAGIC "MSTA"
#define MEM_END_MAGIC "MEND"

/* Append memory magic to allocated buffer.
 * Return the virutal allocated pointer. */
static void *append_mem_magic (void *ptr, size_t nbytes)
{
	char *pc = (char *) ptr;

	if (ptr == NULL)
		return NULL;
	
	memcpy (pc, MEM_START_MAGIC, 4);
	memcpy (pc + 4, &nbytes, 4);
	memcpy (pc + 8 + nbytes, MEM_END_MAGIC, 4);
	return pc + 8;
}


/* Check the memory magic for virtual allocated buffer.
 * Return the real allocated pointer if the buffer is not correct,
 * otherwise returns NULL. */
static void *check_mem_magic (void *ptr)
{
	size_t nbytes;
	char *pc = (char *) ptr;
	if ((unsigned int) pc <= 8)
		return NULL;
	
	if (memcmp (pc - 8, MEM_START_MAGIC, 4) != 0)
		return NULL;
	
	memcpy (&nbytes, pc - 4, 4);
	
	if (memcmp (pc + nbytes, MEM_END_MAGIC, 4) != 0)
		return NULL;

	return pc - 8;
}


/* Append memory buffer to memory list.
 * Return the virutal allocated pointer. */
static void *add_mem_list(my_mem_list_struct_t **header, char *ptr, int isize, int iline, const char *pcfile)
{
	my_mem_list_struct_t *p;
	if (ptr == NULL) return;

	my_pthread_mutex_lock(&g_ptmmem);
	
	ptr = append_mem_magic (ptr, isize);
	
	for (p=*header; p!=NULL; p=p->pnext)
	{
		if (check_mem_magic (p->ptr) == NULL
			|| check_mem_magic (p->pcfile) == NULL)
		{
			fprintf(stderr,
				"Buffer %08x allocated in %s:%d is damaged!\n"
				"Checked in %s:%d, add_mem_lis \n",
				ptr, p->pcfile, p->iline,
				__FILE__, __LINE__);
			my_pthread_mutex_unlock(&g_ptmmem);
			exit (-1);
		}
		if (p->ptr == ptr)
		{
			fprintf(stderr,
				"Buffer %08x allocated in %s:%s,\n"
				"Found same buffer pointer allocated in %s:%s\n"
				"Checked in %s:%d, add_mem_list\n",
				ptr, pcfile, iline,
				p->pcfile, p->iline,
				__FILE__, __LINE__);
			my_pthread_mutex_unlock(&g_ptmmem);
			exit(-1);
		}
	}

	p = (my_mem_list_struct_t *)save_malloc(sizeof(my_mem_list_struct_t));
	if (p == NULL)
	{
		fprintf(stderr,
			"Not enough memory in %s:%d,\n"
			"Checked in %s:%d, add_mem_list\n",
			pcfile, iline,
		   	__FILE__, __LINE__);
		my_pthread_mutex_unlock(&g_ptmmem);
		exit(-1);
	}

	p->ptr = ptr;
	p->isize = isize;
	p->iline = iline;
	
	//p->pcfile = save_strdup(pcfile);
	{
		size_t file_len = strlen (pcfile) + 1;
		p->pcfile = save_malloc(file_len + 12);
		if (p->pcfile == NULL)
		{
			fprintf (stderr,
				"Not enough memory in %s:%d,\n"
				"Checked in %s:%d, add_mem_list\n",
				pcfile, iline,
				__FILE__, __LINE__);
		}
		else
		{
			p->pcfile = append_mem_magic (p->pcfile, file_len);
			memcpy (p->pcfile, pcfile, file_len);
		}
	}
	
	p->pnext = *header;
	*header = p;

	my_pthread_mutex_unlock(&g_ptmmem);

	return ptr;
}


/* Delete the memory buffer from memory list.
 * Return the real allocated pointer if the buffer is not correct,
 * otherwise reports a bug and terminates the program. */
static void *del_mem_list(my_mem_list_struct_t **header, char *ptr, int iline, const char *pcfile)
{
	my_mem_list_struct_t *p, *pre;
	void *ptr_real = NULL;

	if (ptr == NULL) return;

	my_pthread_mutex_lock(&g_ptmmem);

	if (check_mem_magic(ptr) == NULL)
	{
		fprintf(stderr,
			"Buffer %08x released in %s:%d is already damaged!\n"
			"Checked in %s:%d, del_mem_list\n",
			ptr, pcfile, iline,
		 	__FILE__, __LINE__);
	}
	
	for (p=*header; p!=NULL; pre=p, p=p->pnext)
	{
		if ((ptr_real = check_mem_magic (p->ptr)) == NULL
			|| check_mem_magic (p->pcfile) == NULL)
		{
			fprintf(stderr,
				"Buffer %08x allocated in %s:%d is damaged!\n"
				"Checked in %s:%d, del_mem_list\n",
				ptr, p->pcfile, p->iline,
				__FILE__, __LINE__);
			my_pthread_mutex_unlock(&g_ptmmem);
			exit (-1);
		}
		if (p->ptr == ptr)
			break;
	}

	if (p == NULL)
	{
		fprintf(stderr,
			"Pointer %08x released in %s:%d is not found in memory record!\n"
			"Check in %s:%d, del_mem_list\n",
			ptr, pcfile, iline,
			__FILE__, __LINE__);
		for (p = *header; p!=NULL; p=p->pnext)
			printf("%d ", p->ptr);
		my_pthread_mutex_unlock(&g_ptmmem);
//		return;
		exit(-1);
	}

	if (p == *header)
		*header = p->pnext;
	else
		pre->pnext = p->pnext;
	save_free(check_mem_magic (p->pcfile));
	save_free(p);
	my_pthread_mutex_unlock(&g_ptmmem);

	return ptr_real;
}


void check_mem_list(void)
{
	my_mem_list_struct_t *p;
	my_pthread_mutex_lock(&g_ptmmem);
	
	for (p=g_pmemheader; p!=NULL; p=p->pnext)
	{
		if (check_mem_magic (p->ptr) == NULL
			|| check_mem_magic (p->pcfile) == NULL)
		{
			fprintf(stderr,
				"Buffer %08x allocated in %s:%d is damaged!\n"
				"Checked in %s:%d, check_mem_list\n",
				p->ptr, p->pcfile, p->iline,
				__FILE__, __LINE__);
			my_pthread_mutex_unlock(&g_ptmmem);
			exit (-1);
		}
	}

	my_pthread_mutex_unlock(&g_ptmmem);
}


static void *__my_memory_new (size_t nbytes, int iline, const char *pcfile)
{
	void *ptr = save_malloc (nbytes + 12);
	if (ptr != NULL)
		ptr = add_mem_list (&g_pmemheader, ptr, nbytes, iline, pcfile);
	else
		fprintf (stderr, "malloc failed at: %s %d\n", pcfile, iline);
	return ptr;
}


static void __my_memory_del (void *ptr, int iline, const char *pcfile)
{
	if (ptr != NULL)
	{
		ptr = del_mem_list (&g_pmemheader, ptr, iline, pcfile);
		if (ptr != NULL)
			save_free (ptr);
	}
}


void *my_calloc(size_t nmemb, size_t nbytes, int iline, const char *pcfile)
{
	return __my_memory_new (nmemb * nbytes, iline, pcfile);
}

void *my_malloc(size_t nbytes, int iline, const char *pcfile)
{
	return __my_memory_new (nbytes, iline, pcfile);
}

void *my_realloc(void *aptr, size_t nbytes, int iline, const char *pcfile)
{
	void *aptr_new = __my_memory_new (nbytes, iline, pcfile);
	if (aptr_new != NULL)
	{
		if (aptr != NULL)
		{
			memcpy (aptr_new, aptr, nbytes);	//Maybe problem here.
			__my_memory_del (aptr, iline, pcfile);
		}
	}
	return aptr_new;
}

void my_free(void * aptr, int iline, const char *pcfile)
{
	if (aptr != NULL)
		__my_memory_del (aptr, iline, pcfile);
}

char *my_strdup(const char *s, int iline, const char *pcfile)
{
	char *p;
	if (s == NULL)
		return NULL;

	p = __my_memory_new (strlen (s) + 1, iline, pcfile);
	if (p != NULL)
		strcpy (p, s);
	return p;
}
#if 0
char *my_strndup(const char *s, size_t n, int iline, const char *pcfile)
{
	char *p;
	size_t len;
	if (s == NULL)
		return NULL;

	len = strlen (s);
	if (len > n)
		len = n;
	p = __my_memory_new (len + 1, iline, pcfile);
	if (p != NULL)
	{
		memcpy (p, s, len);
		p[len] = '\0';
	}
	return p;
}


////////////////////////////////////////////////////////////////////////


typedef struct my_fd_list_struct
{
	int fd;
	int iline;
	char *pcfile;
	pthread_t pid;
	struct my_fd_list_struct *pnext;
} my_fd_list_struct_t;
my_fd_list_struct_t *g_pfdheader = NULL;
my_pthread_mutex_t g_ptmfd = MY_PTHREAD_MUTEX_INITIALIZER;


static void add_fd_list(my_fd_list_struct_t **header, int fd, int iline, const char *pcfile)
{
	pthread_t pid;
	my_fd_list_struct_t *p;
	if (fd == 0) return;

	pid=0;//pid = pthread_self();
	my_pthread_mutex_lock(&g_ptmfd);

	for (p=*header; p!=NULL; p=p->pnext)
	{
		if (p->fd == fd && p->pid == pid)
		{
			fprintf(stderr, "add_fd_list error in %s(%s %d).!", __FILE__, pcfile, iline);
			my_pthread_mutex_unlock(&g_ptmfd);
			exit(-1);
		}
	}

	p = (my_fd_list_struct_t *)save_malloc(sizeof(my_fd_list_struct_t));
	if (p == NULL)
	{
		fprintf(stderr, "not enough memory in %s.\n", __FILE__);
		my_pthread_mutex_unlock(&g_ptmfd);
		exit(-1);
	}

	p->fd = fd;
	p->iline = iline;
	p->pcfile = save_strdup(pcfile);
	p->pnext = *header;
	p->pid = pid;
	*header = p;
	my_pthread_mutex_unlock(&g_ptmfd);
}


static void del_fd_list(my_fd_list_struct_t **header, int fd, int iline, const char *pcfile)
{
	my_fd_list_struct_t *p, *pre;
	pthread_t pid;

	if (fd == 0) return;

	pid=0;//pid = pthread_self();
	my_pthread_mutex_lock(&g_ptmfd);

	for (p=*header; p!=NULL; pre=p, p=p->pnext)
	{
		if (p->fd == fd && p->pid == pid)
			break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "del_fd_list error in %s(%s %d %d).!", __FILE__, pcfile, iline, fd);
		for (p = *header; p!=NULL; p=p->pnext)
			printf("%d ", p->fd);
		my_pthread_mutex_unlock(&g_ptmfd);
//		return;
		exit(-1);
	}

	if (p == *header)
		*header = p->pnext;
	else
		pre->pnext = p->pnext;
	save_free(p->pcfile);
	save_free(p);
	my_pthread_mutex_unlock(&g_ptmfd);
}

int my_open(int iline, const char *pcfile, const char *pathname, int flags, ...)
{
	int r;
        mode_t mode;

        if (flags & O_CREAT)
        {
		va_list arg;
		va_start(arg, flags);
		mode = va_arg(arg, mode_t);
		va_end(arg);
	}
	else mode = 0;
	
	r = save_open(pathname, flags, mode);
	if (r >= 0) add_fd_list(&g_pfdheader, r, iline, pcfile);
	return r;
}

int my_creat(int iline, const char *pcfile, const char *pathname, mode_t mode)
{
	int r;
	r = save_creat(pathname, mode);
	if (r >= 0) add_fd_list(&g_pfdheader, r, iline, pcfile);
	return r;
}

int my_socket(int domain, int type, int protocol, int iline, const char *pcfile)
{
	int r;
	r = save_socket(domain, type, protocol);
	if (r >= 0) add_fd_list(&g_pfdheader, r, iline, pcfile);
	return r;
}

int my_accept(int s, struct sockaddr *addr, int *addrlen, int iline, const char *pcfile)
{
	int r;

	r = save_accept(s, addr, addrlen);
	if (r >= 0) add_fd_list(&g_pfdheader, r, iline, pcfile);
	return r;
}

int my_pipe(int filedes[2], int iline, const char *pcfile)
{
	int r;
	r = save_pipe(filedes);
	if (r != -1)
	{
		add_fd_list(&g_pfdheader, filedes[0], iline, pcfile);
		add_fd_list(&g_pfdheader, filedes[1], iline, pcfile);
	}
	return r;
}

int my_dup(int oldfd, int iline, const char *pcfile)
{
	int r;
	r = save_dup(oldfd);

	if (r != -1) add_fd_list(&g_pfdheader, r, iline, pcfile);
	return r;
}

int my_close(int fd, int iline, const char *pcfile)
{
	int r;
	r = save_close(fd);
	if (r == 0) del_fd_list(&g_pfdheader, fd, iline, pcfile);
	return r;
}



///////////////////////////////////////////

typedef struct my_fp_list_struct
{
	FILE *fp;
	int iline;
	char *pcfile;
	pthread_t pid;
	struct my_fp_list_struct *pnext;
} my_fp_list_struct_t;
my_fp_list_struct_t *g_pfpheader = NULL;
my_pthread_mutex_t g_ptmfp = MY_PTHREAD_MUTEX_INITIALIZER;



static void add_fp_list(my_fp_list_struct_t **header, FILE *fp, int iline, const char *pcfile)
{
	pthread_t pid;
	my_fp_list_struct_t *p;
	if (fp == NULL) return;

	pid=0;//pid = pthread_self();
	my_pthread_mutex_lock(&g_ptmfp);
	for (p=*header; p!=NULL; p=p->pnext)
	{
		if (p->fp == fp && p->pid == pid)
		{
			fprintf(stderr, "add_fp_list error in %s(%s %d).!", __FILE__, pcfile, iline);
			my_pthread_mutex_unlock(&g_ptmfp);
			exit(-1);
		}
	}

	p = (my_fp_list_struct_t *)save_malloc(sizeof(my_fp_list_struct_t));
	if (p == NULL)
	{
		fprintf(stderr, "not enough memory in %s.\n", __FILE__);
		my_pthread_mutex_unlock(&g_ptmfp);
		exit(-1);
	}

	p->fp = fp;
	p->iline = iline;
	p->pcfile = save_strdup(pcfile);
	p->pnext = *header;
	p->pid = pid;
	*header = p;
	my_pthread_mutex_unlock(&g_ptmfp);
}


static void del_fp_list(my_fp_list_struct_t **header, FILE *fp, int iline, const char *pcfile)
{
	my_fp_list_struct_t *p, *pre;
	pthread_t pid;

	if (fp == NULL) return;
	pid=0;//pid = pthread_self();
	my_pthread_mutex_lock(&g_ptmfp);

	for (p=*header; p!=NULL; pre=p, p=p->pnext)
	{
		if (p->fp == fp && p->pid == pid)
			break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "del_fp_list error in %s(%s %d %d).!", __FILE__, pcfile, iline, fp);
		for (p = *header; p!=NULL; p=p->pnext)
			printf("%d ", p->fp);
		my_pthread_mutex_unlock(&g_ptmfp);
//		return;
		exit(-1);
	}

	if (p == *header)
		*header = p->pnext;
	else
		pre->pnext = p->pnext;
	save_free(p->pcfile);
	save_free(p);
	my_pthread_mutex_unlock(&g_ptmfp);
}


FILE *my_fopen(const char *path, const char *mode, int iline, const char *pcfile)
{
	FILE *fp;
	fp = save_fopen(path, mode);
	add_fp_list(&g_pfpheader, fp, iline, pcfile);
	return fp;
}

int my_fclose(FILE *stream, int iline, const char *pcfile)
{
	int r;
	r = save_fclose(stream);
	if (r == 0) del_fp_list(&g_pfpheader, stream, iline, pcfile);
	return r;
}










#endif





//-------------------------
void pt_mem_result()
{
	int i;
	my_mem_list_struct_t *p;
	
	my_pthread_mutex_lock(&g_ptmmem);
	
	if (g_pmemheader == NULL)
		fprintf(stderr, "No memory leaks.\n");

	for (p=g_pmemheader; p!=NULL; p=p->pnext)
	{
		if (check_mem_magic (p->ptr) == NULL
			|| check_mem_magic (p->pcfile) == NULL)
		{
			fprintf(stderr,
				"Buffer %08x allocated in %s:%d is damaged!\n"
				"Checked in %s:%d, check_mem_list\n",
				p->ptr, p->pcfile, p->iline,
				__FILE__, __LINE__);
			exit (-1);
		}
		fprintf(stderr, "Memory used:%s,%d----------- %d bytes\n", p->pcfile, p->iline, p->isize);
	}
	
	my_pthread_mutex_unlock(&g_ptmmem);
}

#if 0
void pt_fd_result()
{
	my_fd_list_struct_t *p;
	if (g_pfdheader == NULL)
		fprintf(stderr, "No fd open()ing.\n");

	for (p=g_pfdheader; p!=NULL; p=p->pnext)
	{
		fprintf(stderr, "\33[1m\33[41mfd non-close:\33[0m \33[1m\33[32m%s\33[0m \33[1m\33[33m%d\33[0m\n", p->pcfile, p->iline);
	}
}

void pt_fp_result()
{
	my_fp_list_struct_t *p;
	if (g_pfpheader == NULL)
		fprintf(stderr, "No fp fopen()ing.\n");

	for (p=g_pfpheader; p!=NULL; p=p->pnext)
	{
		fprintf(stderr, "\33[1m\33[41mfp non-fclose:\33[0m \33[1m\33[32m%s\33[0m \33[1m\33[33m%d\33[0m\n", p->pcfile, p->iline);
	}
}

#endif
#endif
