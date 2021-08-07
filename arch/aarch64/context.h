#ifndef _CONTEXT_H
#define _CONTEXT_H


#ifdef __ASM__

#define CTX_OFFSET_REG_0 0x0
#define CTX_OFFSET_REG_2 0x10
#define CTX_OFFSET_REG_4 0x20
#define CTX_OFFSET_REG_6 0x30
#define CTX_OFFSET_REG_8 0x40
#define CTX_OFFSET_REG_10 0x50
#define CTX_OFFSET_REG_12 0x60
#define CTX_OFFSET_REG_14 0x70
#define CTX_OFFSET_REG_16 0x80
#define CTX_OFFSET_REG_18 0x90
#define CTX_OFFSET_REG_20 0xa0
#define CTX_OFFSET_REG_22 0xb0
#define CTX_OFFSET_REG_24 0xc0
#define CTX_OFFSET_REG_26 0xd0
#define CTX_OFFSET_REG_28 0xe0
#define CTX_OFFSET_REG_30 0xf0
#define CTX_OFFSET_ELR 0xf8
#define CTX_OFFSET_SPSR 0x100
#define CTX_OFFSET_SP 0x108

#else
#include <core/types.h>

struct aarch64_context {
	u64 regs[31];
	u64 elr;
	u64 spsr;
	u64 sp;
} __attribute__ ((packed));
#endif // __ASM__

#endif // _CONTEXT_H
