
#include "asm.h"
#include "assert.h"
#include "constants.h"
#include "initfunc.h"
#include "mm.h"
#include "tcr.h"
#include "types.h"

#define PTE_PRESENT (1 << 0)
#define PTE_TABLE (1 << 1)
#define PTE_PAGE (1 << 1)
#define PTE_BLOCK (0 << 1)

// memory region attributes
#define MAIR_DEVICE_nGnRnE 0x00
#define MAIR_NORMAL 0xff
#define MAIR_DEVICE_POS 0
#define MAIR_NORMAL_POS 1
#define MAIR_VALUE                                   \
    ((MAIR_DEVICE_nGnRnE << (8 * MAIR_DEVICE_POS)) | \
     (MAIR_NORMAL << (8 * MAIR_NORMAL_POS)))

#define PTE_MRA_OFFSET 2
#define PTE_MRA_DEVICE_nGnRnE (MAIR_DEVICE_POS << PTE_MRA_OFFSET)
#define PTE_MRA_DEVICE PTE_MRA_DEVICE_nGnRnE
#define PTE_MRA_NORMAL (MAIR_DEVICE_POS << PTE_MRA_OFFSET)

// lower attributes
#define LOW_ATTR_ATTR_IDX_OFFSET (2)
#define LOW_ATTR_NS (1 << 5)
#define LOW_ATTR_AP_OFFSET (6)
#define LOW_ATTR_SH_OFFSET (8)
#define LOW_ATTR_AF (1 << 10)
#define LOW_ATTR_NG (1 << 11)
#define LOW_ATTR_OA_OFFSET (12)
#define LOW_ATTR_NT (1 << 16)

extern u8 head[], end[];

static u64 vmm_l1_pt[512] __attribute__((aligned(PAGESIZE)));
static u64 vmm_l2_pt[512] __attribute__((aligned(PAGESIZE)));

static void vm_init_global(void)
{
    //int i;

    //int l2_size = PAGESIZE * 512;
    //int l1_size = l2_size * 512;

    //int l1_index = (phys_t)head / l1_size;
    //int l2_index = (phys_t)head / l2_size;

    //vmm_l1_pt[l1_index] = (u64)vmm_l2_pt | PTE_TABLE | PTE_PRESENT;

    //for (i = 0; i < VMMSIZE_ALL >> PAGESIZE2M_SHIFT; i++) {
    //    ASSERT(l2_index + i < 512);
    //    phys_t base = (phys_t)head + (i * 0x200000);
    //    vmm_l2_pt[l2_index + i] = base | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE |
    //                              PTE_PRESENT | LOW_ATTR_AF;
    //}

    vmm_l1_pt[0] = (u64)vmm_l2_pt | PTE_TABLE | PTE_PRESENT;
    vmm_l2_pt[0] = 0x00000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT | LOW_ATTR_AF;
    vmm_l2_pt[1] = 0x40000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT | LOW_ATTR_AF;
    vmm_l2_pt[2] = 0x80000000 | PTE_BLOCK | PTE_MRA_DEVICE_nGnRnE | PTE_PRESENT | LOW_ATTR_AF;

    // disable mmu
    u64 sctlr_el2 = read_sctlr_el2();
    sctlr_el2 &= ~0x1;  // SCTLR_EL2_MMU_EN
    write_sctlr_el2(sctlr_el2);


    // save to ttbr0
    write_ttbr0_el2((u64)vmm_l1_pt);

    // setup TCR
    write_tcr_el2((u64)TCR_VALUE);

    // setup mair
    write_mair_el2((u64)MAIR_VALUE);

    // call isb
    isb();

    // enable mmu (sctlr_el2)
    sctlr_el2 = read_sctlr_el2();
    sctlr_el2 |= 0x1;  // SCTLR_EL2_MMU_EN
    write_sctlr_el2(sctlr_el2);

    // call isb
    isb();
}

INITFUNC("global2", vm_init_global);
