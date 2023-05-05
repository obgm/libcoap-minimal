/* minimal CoAP functions
 *
 * Copyright (C) 2018-2024 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstdio>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "common.hh"

int
resolve_address(coap_str_const_t *host, uint16_t port, coap_address_t *dst,
                int scheme_hint_bits) {
  int ret = 0;
  coap_addr_info_t *addr_info;

  addr_info = coap_resolve_address_info(host, port, port,  port, port,
                                        AF_UNSPEC, scheme_hint_bits,
                                        COAP_RESOLVE_TYPE_REMOTE);
  if (addr_info) {
    ret = 1;
    *dst = addr_info->addr;
  }

  coap_free_address_info(addr_info);
  return ret;
}
