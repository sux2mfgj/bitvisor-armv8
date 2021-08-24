#ifndef __CORE_VM_H
#define __CORE_VM_H

#include <core/types.h>

bool arch_flush_tlb_entry (phys_t gpst, phys_t gpend);

/* accessing memory */
void *vm_mapmem (u64 physaddr, uint len, u32 flags);
void vm_unmapmem (void *ptr, uint len);

#endif // __CORE_VM_H
