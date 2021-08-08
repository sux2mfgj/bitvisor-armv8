#ifndef __CORE_VM_H
#define __CORE_VM_H

#include <core/types.h>

bool arch_flush_tlb_entry (phys_t gpst, phys_t gpend);

/* accessing memory */
void unmapmem (void *virt, uint len);
void *mapmem_hphys (u64 physaddr, uint len, int flags);
void *mapmem_as (const struct mm_as *as, u64 physaddr, uint len, int flags);

#endif // __CORE_VM_H
