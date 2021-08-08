# config macros

CFLAGS += -Ivpn/lib
CFLAGS += -Iedk/Foundation/Efi/Include/ -Iedk/Foundation/Framework/Include/
CFLAGS += -Iedk/Foundation/Include/ -Iedk/Foundation/Efi/

objs-$(CONFIG_AARCH64) += aarch64_main.o
objs-y += ap.o
objs-$(CONFIG_X86) += acpi.o acpi_dsdt.o assert.o beep.o cache.o callrealmode.o
objs-$(CONFIG_X86) += calluefi.o config.o cpu.o cpu_emul.o cpu_interpreter.o cpu_mmu.o
objs-$(CONFIG_X86) += cpu_mmu_spt.o cpu_seg.o cpu_stack.o cpuid.o cpuid_pass.o current.o
objs-$(CONFIG_X86) += debug.o exint_pass.o gmm_access.o gmm_pass.o i386-stub.o iccard.o
objs-y += initfunc.o int.o
objs-$(CONFIG_X86) += io_io.o io_iohook.o io_iopass.o keyboard.o
objs-y += mm.o vm.o mmio.o
objs-$(CONFIG_X86) += loadbootsector.o localapic.o main.o msg.o msr.o
objs-y += printf.o panic.o
objs-$(CONFIG_X86) += msr_pass.o nmi_pass.o osloader.o process.o
objs-y += putchar.o pcpu.o
objs-$(CONFIG_X86) += random.o reboot.o savemsr.o seg.o serial.o sleep.o
objs-$(CONFIG_X86) += strtol.o svm.o svm_exitcode.o svm_init.o svm_io.o svm_main.o
objs-$(CONFIG_X86) += svm_msr.o svm_np.o svm_paging.o svm_panic.o svm_regs.o
objs-y += vcpu.o
objs-$(CONFIG_X86) += initipi_pass.o tcg.o thread.o time.o timer.o tty.o uefi.o
objs-$(CONFIG_X86) += vga.o vmmcall.o vmmcall_boot.o vmmcall_dbgsh.o vmmcall_iccard.o
objs-$(CONFIG_X86) += vmmcall_log.o vmmcall_status.o vpn_ve.o vramwrite.o vt.o vt_ept.o
objs-$(CONFIG_X86) += vt_exitreason.o vt_init.o vt_io.o vt_main.o vt_msr.o vt_paging.o
objs-$(CONFIG_X86) += vt_panic.o vt_regs.o wakeup.o xsetbv.o xsetbv_pass.o uefiutil.o
objs-$(CONFIG_X86) += uefi_param_ext.o nmi.o initipi.o vt_shadow_vt.o

objs-$(CONFIG_DEVICE_TREE) += fdt.o
