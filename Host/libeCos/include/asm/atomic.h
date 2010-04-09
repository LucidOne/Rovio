#ifndef __ASM_ATOMIC_H__
#define __ASM_ATOMIC_H__

#define atomic_t int
#define atomic_inc(atom) (*atom)++
#define atomic_dec(atom) (*atom)--
#define atomic_read(atom) (*atom)
#define atomic_set(v,i)	(atomic_read(v) = (i))
static __inline int atomic_dec_and_test(volatile atomic_t *v)
{
	//unsigned long flags;
	int result;

	//__save_flags_cli(flags);
	atomic_dec(v);
	result = (atomic_read(v) == 0);
	//__restore_flags(flags);

	return result;
}
static __inline void atomic_add(int i, volatile atomic_t *v)
{
	//unsigned long flags;

	//__save_flags_cli(flags);
	atomic_read(v) += i;
	//__restore_flags(flags);
}
static __inline void atomic_sub(int i, volatile atomic_t *v)
{
	//unsigned long flags;

	//__save_flags_cli(flags);
	atomic_read(v) -= i;
	//__restore_flags(flags);
}


#endif /* __ASM_ATOMIC_H__ */
