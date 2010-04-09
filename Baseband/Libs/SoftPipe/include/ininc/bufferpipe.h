#ifndef __BUFFER_PIPE_H__
#define __BUFFER_PIPE_H__


#define SP_GET_PARENT_ADDR(pMe,tParent,eMyName) \
	(tParent *)((char *)(pMe) - (int)&((tParent *)0)->eMyName)

typedef struct
{
	SOFT_PIPE_T		pipe;
	const void		*pBuffer;
	unsigned int	uLen;
} SOURCE_BUFFER_PIPE_T;


typedef struct
{
	SOFT_PIPE_T		pipe;
	void			*pBuffer;
	unsigned int	uLen;
} TARGET_BUFFER_PIPE_T;


void
spSourceBufferInit (SOURCE_BUFFER_PIPE_T *psbpThis,
					const void *pBuffer,
					unsigned int uLen);

void
spTargetBufferInit (TARGET_BUFFER_PIPE_T *ptbpThis,
					void *pBuffer,
					unsigned int uLen);

#endif
