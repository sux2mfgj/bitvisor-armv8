
/*
 * Flattend Device Tree (DTB) Parser
 */

#include <core/types.h>
#include "fdt.h"
#include "initfunc.h"

struct fdt_header {
    u32 magic;
    u32 totalsize;
    u32 off_dt_struct;
    u32 off_dt_strings;
    u32 off_mem_rsvmap;
    u32 version;
    u32 last_comp_version;
    u32 boot_cpuid_phys;
    u32 size_dt_strings;
    u32 size_dt_struct;
};

struct fdt_reserve_entry {
    u64 address;
    u64 size;
};

static struct fdt_header* fdt;

static void fdt_init_global(void)
{
    fdt = get_fdt_base();

    /* parse fdt */
}

INITFUNC ("global0", fdt_init_global);
