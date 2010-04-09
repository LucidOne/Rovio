#ifndef __LINUX_SPINLOCK_H__
#define __LINUX_SPINLOCK_H__


typedef struct { char c; } spinlock_t;
#define SPIN_LOCK_UNLOCKED (spinlock_t) { }
#define spin_lock_init(lock) do{} while (0)
#define spin_lock(lock) do{} while (0)
#define spin_unlock(lock) do{} while (0)
#define spin_lock_bh(lock) do{} while (0)
#define spin_unlock_bh(lock) do{} while (0)
#define read_lock_bh(lock)			do { } while (0)
#define read_unlock_bh(lock)			do {} while (0)
#define read_lock(lock)		(void)(lock) /* Not "unused variable". */
#define read_unlock(lock)	do { } while(0)
#define write_lock(lock)	(void)(lock) /* Not "unused variable". */
#define write_unlock(lock)	do { } while(0)
#define write_lock_bh(lock)			do {} while (0)
#define write_unlock_bh(lock)			do {} while (0)

#endif /* __LINUX_SPINLOCK_H__ */
