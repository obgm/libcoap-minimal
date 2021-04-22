/* minimal CoAP functions
 *
 * Copyright (C) 2018-2021 Olaf Bergmann <bergmann@tzi.org>
 */

#include <coap3/coap.h>

int resolve_address(const char *host, const char *service, coap_address_t *dst);
