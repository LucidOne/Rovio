#ifndef __LINUX_SCHED_H__
#define __LINUX_SCHED_H__

#define cond_resched() do { } while(0)
#define signal_pending(x) (0)
//#ifndef LONG_MAX//clyu for system redefine
//#define LONG_MAX	((long)(~0UL>>1))
//#endif
#define	MAX_SCHEDULE_TIMEOUT	LONG_MAX
#define capable(a)	1

#endif /* __LINUX_SCHED_H__ */
