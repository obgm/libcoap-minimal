/* minimal CoAP functions
 *
 * Copyright (C) 2018-2026 Olaf Bergmann <bergmann@tzi.org>
 */

#include <coap3/coap.h>

int resolve_address(coap_str_const_t *host, uint16_t port, coap_address_t *dst,
                    int scheme_hint_bits);
