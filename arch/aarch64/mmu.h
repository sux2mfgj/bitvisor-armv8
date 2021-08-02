#ifndef _MMU_H
#define _MMU_H

#define VTTBR_EL2_CNP_MASK 0b1
#define VTTBR_EL2_ADDR_MASK 0x0000fffffffffffeULL
#define VTTBR_EL2_VMID_OFFSET 48
#define VTTBR_EL2_VMID_MASK 0xffff000000000000ULL


// Virtualization Translation Control Register (VTCR_EL2)
// starting level of the stage 2 translation lookup
#define VTCR_EL2_T0SZ_OFFSET 0
//#define VTCR_EL2_T0SZ (16 << VTCR_EL2_T0SZ_OFFSET)
#define VTCR_EL2_T0SZ ((64 - 38) << VTCR_EL2_T0SZ_OFFSET)

// starting level of the stage 2 translation lookup
#define VTCR_EL2_SL0_OFFSET 6
//#define VTCR_EL2_SL0 (0b10 << VTCR_EL2_SL0_OFFSET)
#define VTCR_EL2_SL0 (0b01 << VTCR_EL2_SL0_OFFSET)

#define VTCR_EL2_IRGN0_OFFSET 8
#define VTCR_EL2_IRGN0_NC       (0b00 << VTCR_EL2_IRGN0_OFFSET)
#define VTCR_EL2_IRGN0_WB_RA_WA (0b01 << VTCR_EL2_IRGN0_OFFSET)
#define VTCR_EL2_IRGN0_WT_RA_WRA (0b10 << VTCR_EL2_IRGN0_OFFSET)
#define VTCR_EL2_IRGN0_TB_RA_NRA (0b11 << VTCR_EL2_IRGN0_OFFSET)

#define VTCR_EL2_ORGN0_OFFSET 10
#define VTCR_EL2_ORGN0_NC       (0b00 << VTCR_EL2_ORGN0_OFFSET)
#define VTCR_EL2_ORGN0_WB_RA_WA (0b01 << VTCR_EL2_ORGN0_OFFSET)
#define VTCR_EL2_ORGN0_WT_RA_WRA (0b10 << VTCR_EL2_ORGN0_OFFSET)
#define VTCR_EL2_ORGN0_TB_RA_NRA (0b11 << VTCR_EL2_ORGN0_OFFSET)

#define VTCR_EL2_SH0_OFFSET 12
#define VTCR_EL2_SH0_NS (0b00 << VTCR_EL2_SH0_OFFSET)
#define VTCR_EL2_SH0_OS (0b01 << VTCR_EL2_SH0_OFFSET)
#define VTCR_EL2_SH0_IS (0b11 << VTCR_EL2_SH0_OFFSET)

#define VTCR_EL2_TG0_OFFSET 14
#define VTCR_EL2_TG0_4K (0b00 << VTCR_EL2_TG0_OFFSET)
#define VTCR_EL2_TG0_64K (0b01 << VTCR_EL2_TG0_OFFSET)
#define VTCR_EL2_TG0_16K (0b10 << VTCR_EL2_TG0_OFFSET)

// physical address size
#define VTCR_EL2_PS_OFFSET 16
#define VTCR_EL2_PS_32 (0b000 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_36 (0b001 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_40 (0b010 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_42 (0b011 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_44 (0b100 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_48 (0b101 << VTCR_EL2_PS_OFFSET)
#define VTCR_EL2_PS_52 (0b110 << VTCR_EL2_PS_OFFSET)

#define VTCR_EL2_VS_OFFSET (19)
#define VTCR_EL2_VS_8 (0b0 << VTCR_EL2_VS_OFFSET)
#define VTCR_EL2_VS_16 (0b1 << VTCR_EL2_VS_OFFSET)

#define VTCR_EL2_NSW_OFFSET (29)
#define VTCR_EL2_NSW_S (0b0 << VTCR_EL2_NSW_OFFSET)
#define VTCR_EL2_NSW_NS (0b1 << VTCR_EL2_NSW_OFFSET)

#define VTCR_EL2_NSA_OFFSET (29)
#define VTCR_EL2_NSA_S (0b0 << VTCR_EL2_NSA_OFFSET)
#define VTCR_EL2_NSA_NS (0b1 << VTCR_EL2_NSA_OFFSET)

#endif // _MMU_H
