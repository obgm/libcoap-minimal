/* minimal CoAP server
 *
 * Copyright (C) 2018-2023 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "common.hh"

int
main(void) {
  coap_context_t  *ctx = nullptr;
  coap_address_t dst;
  coap_resource_t *resource = nullptr;
  coap_endpoint_t *endpoint = nullptr;
  int result = EXIT_FAILURE;;
  coap_str_const_t *ruri = coap_make_str_const("hello");
  coap_startup();

  /* resolve destination address where server should be sent */
  if (resolve_address("localhost", "5683", &dst) < 0) {
    coap_log_crit("failed to resolve address\n");
    goto finish;
  }

  /* create CoAP context and a client session */
  ctx = coap_new_context(nullptr);

  if (!ctx || !(endpoint = coap_new_endpoint(ctx, &dst, COAP_PROTO_UDP))) {
    coap_log_emerg("cannot initialize context\n");
    goto finish;
  }

  resource = coap_resource_init(ruri, 0);
  coap_register_handler(resource, COAP_REQUEST_GET,
                        [](auto, auto,
                           const coap_pdu_t *request,
                           auto,
                           coap_pdu_t *response) {
                          coap_show_pdu(COAP_LOG_WARN, request);
                          coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
                          coap_add_data(response, 5,
                                        (const uint8_t *)"world");
                          coap_show_pdu(COAP_LOG_WARN, response);
                        });
  coap_add_resource(ctx, resource);

  while (true) { coap_io_process(ctx, COAP_IO_WAIT); }

  result = EXIT_SUCCESS;
 finish:

  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
