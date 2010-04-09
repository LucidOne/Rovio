It is a thread library for none-OS environment originally.
Currently it's just an wrapper for mutex, semaphore and condition variable based on eCos thread.


Tiny thread library.


functions list

Thread:
	  tt_create_thread
	? tt_exit_thread
	? tt_get_current_thread
	? tt_yield_thread

Time:
	? tt_msleep
	? tt_usleep
	  tt_get_ticks
	  tt_ticks_to_msec
	  tt_msec_to_ticks
	  tt_get_time
	  tt_set_time

Semaphore:
	  tt_sem_init
	? tt_sem_down
	  tt_sem_try_down
	  tt_sem_can_down
	  tt_sem_up

Mutex(lock):
	  tt_mutex_init
	? tt_mutex_lock
	  tt_mutex_try_lock
	  tt_mutex_can_lock
	  tt_mutex_unlock

Recursive mutex(lock):
	  tt_rmutex_init
	? tt_rmutex_lock
	  tt_rmutex_try_lock
	  tt_rmutex_can_lock
	  tt_rmutex_unlock

Message:
	  tt_msg_queue_init
	? tt_msg_send
	  tt_msg_try_send
	  tt_msg_can_send
	? tt_msg_recv
	  tt_msg_try_recv
	  tt_msg_can_recv


The function start with "?" can not be called in a interrupt handler.
