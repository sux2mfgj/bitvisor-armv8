DIR ?= .
V ?= 0
CROSS_COMPILE :=
include Makefile.common
include $(CONFIG)

.PHONY : all
all : build-all

.PHONY : clean
clean : clean-all

NAME   = bitvisor
FORMAT = elf32-i386
elf    = $(NAME).elf
map    = $(NAME).map
ifeq ($(CONFIG_AARCH64_VIRT),1)
lds    = arch/aarch64/board/virt/$(NAME).lds
else
lds    = $(NAME).lds
endif
target = $(elf)

subdirs-1 += arch core #drivers
subdirs-$(CONFIG_STORAGE) += storage
subdirs-$(CONFIG_STORAGE_IO) += storage_io
subdirs-$(CONFIG_VPN) += vpn
subdirs-$(CONFIG_IDMAN) += idman
#subdirs-1 += net
subdirs-$(CONFIG_IP) += ip
asubdirs-$(CONFIG_CRYPTO) += crypto
#psubdirs-1 += process

process-depends-$(CONFIG_CRYPTO) += $(dir)crypto/$(outa_p)
process-depends-$(CONFIG_IDMAN) += $(dir)idman/$(outo_p)
process-depends-$(CONFIG_STORAGE) += $(dir)storage/$(outo_p)
process-depends-$(CONFIG_STORAGE_IO) += $(dir)storage_io/$(outo_p)
process-depends-$(CONFIG_VPN) += $(dir)vpn/$(outo_p)

$(dir)$(elf) : $(defouto) $(dir)$(lds)
	$(V-info) LD $(dir)$(elf)
	$(warning $(CC) $(LDFLAGS) -Wl,-T,$(dir)$(lds) -Wl,--cref -Wl,-Map,$(dir)$(map) -o $(dir)$(elf) $(defouto))
	$(CC) $(LDFLAGS) -Wl,-T,$(dir)$(lds) -Wl,--cref \
		-Wl,-Map,$(dir)$(map) -o $(dir)$(elf) $(defouto)
	#@$(OBJCOPY) --output-format $(FORMAT) $(dir)$(elf)

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
config : $(CONFIG)
	$(MAKE) -f Makefile.config config

$(CONFIG) : Makefile.config
	: >> $(CONFIG)
	$(MAKE) -f Makefile.config update-config

defconfig :
	cp defconfig.tmpl defconfig

$(dir)process/$(outp_p) : $(process-depends-1)
