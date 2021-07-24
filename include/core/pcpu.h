
#ifndef __CORE_PCPU_H
#define __CORE_PCPU_H

#include <core/pcpu.h>
#include <core/types.h>

enum fullvirtualize_type {
	FULLVIRTUALIZE_NONE,
#ifdef CONFIG_X86
	FULLVIRTUALIZE_VT,
	FULLVIRTUALIZE_SVM,
#endif
#ifdef CONFIG_AARCH64
	// FULLVIRTUALIZE_ARMV8_1
	FULLVIRTUALIZE_ARMV8_2
// FULLVIRTUALIZE_ARMV8_3
// FULLVIRTUALIZE_ARMV8_4
// FULLVIRTUALIZE_ARMV9_0 ?
#endif
};

struct pcpu_func {
};

struct pcpu {
	struct pcpu *next;
	struct pcpu_func func;
	// struct segdesc segdesctbl[NUM_OF_SEGDESCTBL];
	// struct tss32 tss32;
	// struct tss64 tss64;
	// union {
	//    struct vt_pcpu_data vt;
	//    struct svm_pcpu_data svm;
	//};
	// struct cache_pcpu_data cache;
	// struct panic_pcpu_data panic;
	// struct thread_pcpu_data thread;
	enum fullvirtualize_type fullvirtualize;
	// enum apic_mode apic;
	int cpunum;
	int pid;
	void *stackaddr;
	// TODO tsc is hardware dependent.
	u64 tsc, hz, timediff;
	// spinlock_t suspend_lock;
	// phys_t cr3;
	bool pass_vm_created;
	bool use_invariant_tsc;
	// void (*release_process64_msrs) (void *release_process64_msrs_data);
	// void *release_process64_msrs_data;
};

extern struct pcpu pcpu_default;

void pcpu_list_foreach (bool (*func) (struct pcpu *p, void *q), void *q);
void pcpu_list_add (struct pcpu *d);
void pcpu_init (void);

struct pcpu *get_currentcpu_arch (void);

#define currentcpu get_currentcpu_arch ()

#endif // __CORE_PCPU_H
