/* minimal CoAP functions
 *
 * Copyright (C) 2018-2024 Olaf Bergmann <bergmann@tzi.org>
 */

#include <coap3/coap.h>

int resolve_address(coap_str_const_t *host, uint16_t port, coap_address_t *dst,
                    int scheme_hint_bits);
