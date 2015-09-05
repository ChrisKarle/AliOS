##############################################################################
# Copyright (c) 2014, Christopher Karle
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#   - Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   - Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   - Neither the name of the author nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER, AUTHOR OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##############################################################################

##############################################################################
#
##############################################################################
LWIP_PATH ?= ../../extras/lwip

##############################################################################
#
##############################################################################
INCLUDES += -I$(LWIP_PATH) -I$(LWIP_PATH)/include

##############################################################################
# API files
##############################################################################
VPATH += $(LWIP_PATH)/api
C_FILES += api_lib.c api_msg.c err.c netbuf.c netdb.c netifapi.c sockets.c \
           tcpip.c

##############################################################################
# arch files
##############################################################################
VPATH += $(LWIP_PATH)/arch
C_FILES += sys_arch.c

##############################################################################
# core files
##############################################################################
VPATH += $(LWIP_PATH)/core
C_FILES += def.c init.c mem.c pbuf.c stats.c tcp_in.c udp.c dhcp.c memp.c \
           raw.c sys.c tcp_out.c dns.c netif.c tcp.c timers.c

##############################################################################
# IPv4
##############################################################################
INCLUDES += -I$(LWIP_PATH)/include/ipv4
VPATH += $(LWIP_PATH)/core/ipv4
C_FILES += autoip.c icmp.c igmp.c inet.c inet_chksum.c ip_addr.c ip.c \
           ip_frag.c

##############################################################################
# IPv6
##############################################################################
#INCLUDES += -I$(LWIP_PATH)/include/ipv6
#VPATH += $(LWIP_PATH)/core/ipv6
#C_FILES += icmp6.c inet6.c ip6_addr.c ip6.c

##############################################################################
# SNMP
##############################################################################
VPATH += $(LWIP_PATH)/core/snmp
C_FILES += asn1_dec.c asn1_enc.c mib2.c mib_structs.c msg_in.c msg_out.c

##############################################################################
# netif
##############################################################################
VPATH += $(LWIP_PATH)/netif
C_FILES += etharp.c ethernetif.c slipif.c

##############################################################################
# PPP
##############################################################################
VPATH += $(LWIP_PATH)/netif/ppp
C_FILES += auth.c chpms.c ipcp.c magic.c pap.c ppp_oe.c vj.c chap.c fsm.c \
           lcp.c md5.c ppp.c randm.c

