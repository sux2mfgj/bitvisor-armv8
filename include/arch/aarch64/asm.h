
#ifndef __ASM
#define __ASM

#include <core/types.h>

static inline void
isb (void)
{
	asm volatile("isb");
}

static inline void
at_st1_el1_read (u64 vaddr)
{
	asm volatile("at s1e1r, %[x]" ::[x] "r"(vaddr));
}

static inline void
at_st1_el1_write (u64 vaddr)
{
	asm volatile("at s1e1w, %[x]" ::[x] "r"(vaddr));
}

#define DEF_SYS_REG_READ(name)                                      \
	static inline u64 read_##name (void)                        \
	{                                                           \
		u64 value;                                          \
		asm volatile("mrs %[x], " #name : [x] "=r"(value)); \
		return value;                                       \
	}

#define DEF_SYS_REG_WRITE(name)                                       \
	static inline void write_##name (u64 value)                   \
	{                                                             \
		asm volatile("msr " #name ", %[x]" ::[x] "r"(value)); \
	}

DEF_SYS_REG_READ (tcr_el2)
DEF_SYS_REG_WRITE (tcr_el2)
DEF_SYS_REG_READ (mair_el2)
DEF_SYS_REG_WRITE (mair_el2)
DEF_SYS_REG_READ (sctlr_el2)
DEF_SYS_REG_WRITE (sctlr_el2)
DEF_SYS_REG_READ (ttbr0_el2)
DEF_SYS_REG_WRITE (ttbr0_el2)
DEF_SYS_REG_READ (vbar_el2)
DEF_SYS_REG_WRITE (vbar_el2)
DEF_SYS_REG_READ (hcr_el2)
DEF_SYS_REG_WRITE (hcr_el2)
DEF_SYS_REG_READ (tpidr_el2)
DEF_SYS_REG_WRITE (tpidr_el2)
DEF_SYS_REG_READ (vtcr_el2)
DEF_SYS_REG_WRITE (vtcr_el2)
DEF_SYS_REG_READ (vttbr_el2)
DEF_SYS_REG_WRITE (vttbr_el2)
DEF_SYS_REG_READ (esr_el2)
DEF_SYS_REG_WRITE (esr_el2)
DEF_SYS_REG_READ (sp_el0)
DEF_SYS_REG_WRITE (sp_el0)
DEF_SYS_REG_READ (sp_el1)
DEF_SYS_REG_WRITE (sp_el1)
DEF_SYS_REG_READ (sp_el2)
DEF_SYS_REG_WRITE (sp_el2)
DEF_SYS_REG_READ (spsel)
DEF_SYS_REG_WRITE (spsel)
DEF_SYS_REG_READ (far_el2)
DEF_SYS_REG_READ (par_el1)

#define daif_clear(value) asm volatile("msr daifclr, %0" ::"I"(value));
#define daif_set(value) asm volatile("msr daifset, %0" ::"I"(value))

static inline void
asm_set_stack_and_jump (ulong sp, void *jmpto)
{
	asm volatile("mov sp, %0\n\t"
		     "mov x30, %1\n\t"
		     "ret\n\t" ::"r"(sp),
		     "r"(jmpto));
}

static inline ulong
asm_lock_ulong_swap (ulong *mem, ulong newval)
{
	// ulong oldval;

	// TODO: fix to use cas inst before multiprocessing. currently it causes
	// an exception. asm volatile("cas %0, %2, %1" 	     : "=r"(oldval),
	// "+Q"(*mem)
	//	     : "r"(newval));
	// return oldval;
	return 1;
}

#endif
