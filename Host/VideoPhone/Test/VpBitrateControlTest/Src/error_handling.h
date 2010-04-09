#ifndef __ERROR_HANDLING_H__
#define __ERROR_HANDLING_H__

#define	DBG_LEVEL_FORCE		0x1
#define	DBG_LEVEL_ERROR		0x2
#define	DBG_LEVEL_WARNING	0x3
#define	DBG_LEVEL_TRACE		0x4
#define	DBG_LEVEL_VERBOSE	0x5
#define	DBG_LEVEL_RELEASE	0x6

#define UART_DBG_LEVEL DBG_LEVEL_TRACE

#if (UART_DBG_LEVEL == DBG_LEVEL_RELEASE)
#	define	DBGPRINTF(level, _x_)
#else
#	define	DBGPRINTF(level, _x_)			\
			{								\
				if(UART_DBG_LEVEL >= level)	\
				{							\
					diag_printf(_x_);		\
				}							\
			}
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#if (UART_DBG_LEVEL == DBG_LEVEL_RELEASE)
#	define	ASSERT(cond)
#else
#	define	ASSERT(cond)						\
			do									\
			{									\
				if ((cond) == FALSE)			\
				{								\
					DBGPRINTF(DBG_LEVEL_ERROR, "ASSERT: "); \
					DBGPRINTF(DBG_LEVEL_ERROR, #cond);			\
					cyg_interrupt_disable();	\
					while(1);					\
				}								\
			}while(0);
#endif


void DbgLog (char *strModuleName, const char *strFormat, ...);


#endif
