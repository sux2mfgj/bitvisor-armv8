# config macros
CONSTANTS-$(CONFIG_ENABLE_ASSERT) += -DENABLE_ASSERT
CONSTANTS-$(CONFIG_IDMAN) += -DIDMAN
CONSTANTS-$(CONFIG_VPN_PD) += -DVPN_PD

CFLAGS += -Iinclude/core -Ivpn

objs-y += crypt.o user.o
subdirs-y += Se
