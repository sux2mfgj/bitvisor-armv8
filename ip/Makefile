LWIP_VERSION=2.0.3

CFLAGS += -I$(DIR)/include -I$(DIR)/include/arch -I$(DIR)/include/lwip \
	-I$(DIR)/lwip-$(LWIP_VERSION)/src/include \
	-I$(DIR)/lwip-$(LWIP_VERSION)/src/include/lwip

objs-y += lwip-$(LWIP_VERSION)/src/core/def.o
objs-y += lwip-$(LWIP_VERSION)/src/core/dns.o
objs-y += lwip-$(LWIP_VERSION)/src/core/init.o
objs-y += lwip-$(LWIP_VERSION)/src/core/mem.o
objs-y += lwip-$(LWIP_VERSION)/src/core/memp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/netif.o
objs-y += lwip-$(LWIP_VERSION)/src/core/pbuf.o
objs-y += lwip-$(LWIP_VERSION)/src/core/raw.o
objs-y += lwip-$(LWIP_VERSION)/src/core/stats.o
objs-y += lwip-$(LWIP_VERSION)/src/core/sys.o
objs-y += lwip-$(LWIP_VERSION)/src/core/tcp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/tcp_in.o
objs-y += lwip-$(LWIP_VERSION)/src/core/tcp_out.o
objs-y += lwip-$(LWIP_VERSION)/src/core/udp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/timeouts.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ip.o
objs-y += lwip-$(LWIP_VERSION)/src/core/inet_chksum.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/autoip.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/dhcp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/etharp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/igmp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/icmp.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/ip4.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/ip4_addr.o
objs-y += lwip-$(LWIP_VERSION)/src/core/ipv4/ip4_frag.o
objs-y += lwip-$(LWIP_VERSION)/src/netif/ethernet.o
objs-y += lwip-$(LWIP_VERSION)/src/api/api_lib.o
objs-y += lwip-$(LWIP_VERSION)/src/api/api_msg.o
objs-y += lwip-$(LWIP_VERSION)/src/api/err.o
objs-y += lwip-$(LWIP_VERSION)/src/api/netbuf.o
objs-y += lwip-$(LWIP_VERSION)/src/api/netdb.o
objs-y += lwip-$(LWIP_VERSION)/src/api/netifapi.o
objs-y += lwip-$(LWIP_VERSION)/src/api/sockets.o
objs-y += lwip-$(LWIP_VERSION)/src/api/tcpip.o

objs-y += ip_sys.o arch/sys_arch.o
objs-y += ip_main.o net_main.o
objs-y += echo-server.o echo-client.o echoctl.o
objs-y += telnet-server.o telnet-dbgsh.o
