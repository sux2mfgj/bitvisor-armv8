#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <core/types.h>

struct aarch64_context {
	u64 regs[30];
	u64 elr;
	u64 spsr;
	u64 sp;
};

#endif // _CONTEXT_H
