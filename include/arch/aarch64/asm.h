
#ifndef __ASM
#define __ASM

#include "types.h"

static inline void write_tcr_el2(u64 value)
{
    asm volatile ("msr tcr_el2, %[x]" ::[x]"r"(value));
}

static inline void write_mair_el2(u64 value)
{
    asm volatile ("msr mair_el2, %[x]" ::[x]"r"(value));
}

static inline u64 read_sctlr_el2(void)
{
    u64 value;
    asm volatile ("mrs %[x], sctlr_el2" : [x]"=r"(value));
    return value;
}

static inline void write_sctlr_el2(u64 value)
{
    asm volatile ("msr sctlr_el2, %[x]" ::[x]"r"(value));
}

static inline void write_ttbr0_el2(u64 value)
{
    asm volatile ("msr ttbr0_el2, %[x]" ::[x]"r"(value));
}

static inline void isb(void)
{
    asm volatile ("isb");
}

static inline void write_vbar_el2(u64 value)
{
    asm volatile ("msr vbar_el2, %[x]" ::[x]"r"(value));
}

#define DEF_SYS_REG_READ(name) \
    static inline u64 read_##name(void) \
    { u64 value; asm volatile ("mrs %[x], name" : [x]"=r"(value)); return value; }

#define DEF_SYS_REG_WRITE(name) \
    static inline void write_##name(u64 value) \
    { asm volatile ("msr name, %[x]" ::[x]"r"(value));}

DEF_SYS_REG_READ(hcr_el2)
DEF_SYS_REG_WRITE(hcr_el2)
DEF_SYS_REG_READ(tpidr_el2)
DEF_SYS_REG_WRITE(tpidr_el2)

#define daif_clear(value) asm volatile ("msr daifclr, %0" :: "I"(value));
#define daif_set(value) asm volatile ("msr daifset, %0" :: "I"(value))

#endif
