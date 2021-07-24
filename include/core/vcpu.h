#ifndef __CORE_VCPU_H
#define __CORE_VCPU_H

#include <core/tls.h>
#include <core/types.h>

struct vcpu {
	struct vcpu *next;
	// union {
	//	struct vt vt;
	//	struct svm svm;
	//} u;
	bool halt;
	bool initialized;
	u64 tsc_offset;
	bool updateip;
	bool pass_vm;
	u64 pte_addr_mask;
	// struct cpu_mmu_spt_data spt;
	// struct cpuid_data cpuid;
	// struct exint_func exint;
	// struct gmm_func gmm;
	// struct io_io_data io;
	// struct msr_data msr;
	// struct vmctl_func vmctl;
	const struct mm_as *as;
	/* vcpu0: data per VM */
	struct vcpu *vcpu0;
	// struct mmio_data mmio;
	// struct nmi_func nmi;
	// struct xsetbv_data xsetbv;
	// struct acpi_data acpi;
	// struct localapic_data localapic;
	// struct initipi_func initipi;
	// struct cache_data cache;
};

struct vcpu *get_current_arch ();

#define current get_tls ()->vcpu

#endif // __CORE_VCPU_H

