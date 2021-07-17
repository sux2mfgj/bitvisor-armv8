DIR ?= .
V ?= 0
ARCH	:= x86_64
CROSS_COMPILE	:=
include Makefile.common

.PHONY : all
all : build-all

.PHONY : clean
clean : clean-all

NAME   = bitvisor
FORMAT = elf32-i386
elf    = $(NAME).elf
map    = $(NAME).map
lds    = arch/$(ARCH)/$(LDSCRIPT)
kconfig = Kconfig
target = $(elf)

subdirs-y += arch core drivers
subdirs-$(CONFIG_STORAGE) += storage
subdirs-$(CONFIG_STORAGE_IO) += storage_io
subdirs-$(CONFIG_VPN) += vpn
subdirs-$(CONFIG_IDMAN) += idman
subdirs-$(CONFIG_NETAPI) += net
subdirs-$(CONFIG_IP) += ip
asubdirs-$(CONFIG_CRYPTO) += crypto
#psubdirs-y += process

process-depends-$(CONFIG_CRYPTO) += $(dir)crypto/$(outa_p)
process-depends-$(CONFIG_IDMAN) += $(dir)idman/$(outo_p)
process-depends-$(CONFIG_STORAGE) += $(dir)storage/$(outo_p)
process-depends-$(CONFIG_STORAGE_IO) += $(dir)storage_io/$(outo_p)
process-depends-$(CONFIG_VPN) += $(dir)vpn/$(outo_p)


$(dir)$(elf) : $(defouto) $(dir)$(lds)
	$(V-info) LD $(dir)$(elf)
	$(CC) $(LDFLAGS) -Wl,-T,$(dir)$(lds) -Wl,--cref \
		-Wl,-Map,$(dir)$(map) -o $(dir)$(elf) $(defouto)
	#$(OBJCOPY) --output-format $(FORMAT) $(dir)$(elf)

.PHONY : build-all
build-all : $(CONFIG) defconfig
	case "`$(SED) 1q $(DIR)/$(depends) 2>/dev/null`" in \
	''): >> $(DIR)/$(depends);; esac
	$(MAKE) $(V-makeopt-$(V)) -f Makefile.build build-dir DIR=$(DIR) V=$(V)
	$(SIZE) $(dir)$(elf)

.PHONY : clean-all
clean-all :
	$(MAKE) $(V-makeopt-$(V)) -f Makefile.clean clean-dir DIR=$(DIR) V=$(V)

.PHONY : config
#$(CONFIG) : config
config :
ifeq ($(MCONF),)
	$(error mconf is not defined. Please setup the MCONF variable)
endif
	 ARCH=$(ARCH) $(MCONF) $(kconfig)

$(dir)process/$(outp_p) : $(process-depends-y)

include include/config/auto.conf.cmd
export ARCH
