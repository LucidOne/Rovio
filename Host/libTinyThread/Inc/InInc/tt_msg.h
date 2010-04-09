#ifndef __TT_MSG_H__
#define __TT_MSG_H__


/* Producer-consumer */
typedef struct
{
#ifndef ECOS
	TT_SEM_T producer;
	TT_SEM_T consumer;
#else
	cyg_sem_t producer;
	cyg_sem_t consumer;
#endif
} TT_PC_T;

void tt_pc_init (TT_PC_T *pc, int max_produce_count);
void tt_pc_produce (TT_PC_T *pc, void (*produce) (void *arg), void *arg);
int tt_pc_timed_produce (TT_PC_T *pc, void (*produce) (void *arg), void *arg, cyg_tick_count_t abstime);
int tt_pc_try_produce (TT_PC_T *pc, void (*produce) (void *arg), void *arg);
int tt_pc_can_produce__remove_it (TT_PC_T *pc);
void tt_pc_consume (TT_PC_T *pc, void (*consume) (void *arg), void *arg);
int tt_pc_timed_consume (TT_PC_T *pc, void (*consume) (void *arg), void *arg, cyg_tick_count_t abstime);
int tt_pc_try_consume (TT_PC_T *pc, void (*consume) (void *arg), void *arg);
int tt_pc_can_consume__remove_it (TT_PC_T *pc);
int tt_pc_destroy (TT_PC_T *pc);




/* Procedure message */
typedef void (*FUN_TT_MSG_PROC) (void *msg_data);

#ifndef ECOS
typedef struct tagTT_MSG_T
{
	LIST_T			list;
	FUN_TT_MSG_PROC	msg_proc;
	void			*msg_data;
} TT_MSG_T;

typedef struct
{
	LIST_T		*msg_free;
	LIST_T		msg_used;
	int			max_msg_limit;
	void		*msg_buffer;
	size_t		msg_buffer_size;
	TT_PC_T		pc_semaphore;
} TT_MSG_QUEUE_T;

TT_MSG_QUEUE_T *tt_msg_queue_init (void *msg_buffer, size_t msg_buffer_size);
void tt_msg_send (TT_MSG_QUEUE_T *msg_queue, FUN_TT_MSG_PROC msg_proc, void *msg_data);
int tt_msg_try_send (TT_MSG_QUEUE_T *msg_queue, FUN_TT_MSG_PROC msg_proc, void *msg_data);
int tt_msg_can_send__remove_it (TT_MSG_QUEUE_T *msg_queue);
void tt_msg_recv (TT_MSG_QUEUE_T *msg_queue, FUN_TT_MSG_PROC *msg_proc, void **msg_data);
int tt_msg_try_recv (TT_MSG_QUEUE_T *msg_queue, FUN_TT_MSG_PROC *msg_proc, void **msg_data);
int tt_msg_can_recv__remove_it (TT_MSG_QUEUE_T *msg_queue);
#else
typedef struct
{
	FUN_TT_MSG_PROC	msg_proc;
	void			*msg_data;
}TT_MSG_T;

void tt_msg_queue_init (cyg_handle_t *handle, void *msg_buffer, size_t msg_buffer_size);
cyg_bool_t tt_msg_send (cyg_handle_t msg_queue, FUN_TT_MSG_PROC msg_proc, void *msg_data);
int tt_msg_timed_send (cyg_handle_t msg_queue, FUN_TT_MSG_PROC msg_proc, void *msg_data, cyg_tick_count_t abstime);
cyg_bool_t tt_msg_try_send (cyg_handle_t msg_queue, FUN_TT_MSG_PROC msg_proc, void *msg_data);
cyg_bool_t tt_msg_can_send__remove_it (cyg_handle_t msg_queue);
void tt_msg_recv (cyg_handle_t msg_queue, FUN_TT_MSG_PROC *msg_proc, void **msg_data);
cyg_bool_t tt_msg_timed_recv (cyg_handle_t msg_queue, FUN_TT_MSG_PROC *msg_proc, void **msg_data, cyg_tick_count_t abstime);
cyg_bool_t tt_msg_try_recv (cyg_handle_t msg_queue, FUN_TT_MSG_PROC *msg_proc, void **msg_data);
cyg_bool_t tt_msg_can_recv__remove_it (cyg_handle_t msg_queue);
void tt_msg_queue_destroy (cyg_handle_t msg_queue);
#endif


#ifndef ECOS
#define TT_MSG_BUFFER_SIZE(max_msg_num) \
	(sizeof (TT_MSG_QUEUE_T) + MEMORYPOOL_SIZE (sizeof (TT_MSG_T), max_msg_num))
#else
#define TT_THREAD_BUFFER_SIZE(size) (1024+size)
//#define TT_MSG_BUFFER_SIZE(num) (sizeof(MAILBOX_DATA_T) * num) //I don't need it because "cyg_mbox* mbox"
#endif


#ifndef ECOS
/* Block message */
typedef struct
{
	LIST_T			list;
	size_t			block_msg_size;
	int				block_msg_data[1];
} TT_BLOCK_MSG_T;

TT_MSG_QUEUE_T *tt_bmsg_queue_init (void *msg_buffer, size_t msg_buffer_size, size_t block_msg_size);
void tt_bmsg_send (TT_MSG_QUEUE_T *msg_queue, void *block_msg_data, size_t block_msg_size);
int tt_bmsg_try_send (TT_MSG_QUEUE_T *msg_queue, void *block_msg_data, size_t block_msg_size);
void tt_bmsg_recv (TT_MSG_QUEUE_T *msg_queue, void **block_msg_data, size_t *block_msg_size);
int tt_bmsg_try_recv (TT_MSG_QUEUE_T *msg_queue, void **block_msg_data, size_t *block_msg_size);
void tt_bmsg_recv_ok (TT_MSG_QUEUE_T *msg_queue);


#define TT_BLOCK_MSG_BUFFER_SIZE(max_msg_num,block_msg_size) \
	( \
		sizeof (TT_MSG_QUEUE_T) + \
		sizeof (LIST_T) + \
		MEMORYPOOL_SIZE ((block_msg_size) + GetOffsetSize (TT_BLOCK_MSG_T, block_msg_data)), max_msg_num) \
	)
#endif

#endif
