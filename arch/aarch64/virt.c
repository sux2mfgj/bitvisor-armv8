#include <core/vmctl.h>
#include <core/virt.h>
#include <core/vcpu.h>
#include <core/string.h>
#include <core/pcpu.h>
#include <core.h>

static void arch64_vminit(void) {

}

static void arch64_startvm(void) {

}

static struct vmctl_func func = {
    .vminit = arch64_vminit,
    .start_vm = arch64_startvm,
};

int arch_check_virt_available (void) {
    //TODO
    return 0;
}

int arch_virt_init(void) {
    return FULLVIRTUALIZE_ARMV8_2;
}

void arch_vmctl_init(void) {
    memcpy ((void*)&current->vmctl, (void*)&func, sizeof func);
}
