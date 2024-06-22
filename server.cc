/* minimal CoAP server
 *
 * Copyright (C) 2018-2024 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "common.hh"

/*
 * This server listens to Unicast CoAP traffic coming in on port 5683 and handles it
 * as appropriate.
 *
 * If support for multicast traffic is not required, comment out the COAP_LISTEN_MCAST_IPV*
 * definitions.
 */

#define COAP_LISTEN_UCAST_IP "::"

#define COAP_LISTEN_MCAST_IPV4 "224.0.1.187"
#define COAP_LISTEN_MCAST_IPV6 "ff02::fd"

int
main(void) {
  coap_context_t  *ctx = nullptr;
  coap_resource_t *resource = nullptr;
  int result = EXIT_FAILURE;;
  uint32_t scheme_hint_bits;
  coap_addr_info_t *info = nullptr;
  coap_addr_info_t *info_list = nullptr;
  coap_str_const_t *my_address = coap_make_str_const(COAP_LISTEN_UCAST_IP);
  bool have_ep = false;

  /* Initialize libcoap library */
  coap_startup();

  /* Set logging level */
  coap_set_log_level(COAP_LOG_WARN);

  /* Create CoAP context */
  ctx = coap_new_context(nullptr);
  if (!ctx) {
    coap_log_emerg("cannot initialize context\n");
    goto finish;
  }

  /* Let libcoap do the multi-block payload handling (if any) */
  coap_context_set_block_mode(ctx, COAP_BLOCK_USE_LIBCOAP|COAP_BLOCK_SINGLE_BODY);

  scheme_hint_bits = coap_get_available_scheme_hint_bits(0, 0, COAP_PROTO_NONE);
  info_list = coap_resolve_address_info(my_address, 0, 0, 0, 0,
                                        0,
                                        scheme_hint_bits, COAP_RESOLVE_TYPE_LOCAL);
  /* Create CoAP listening endpoint(s) */
  for (info = info_list; info != NULL; info = info->next) {
    coap_endpoint_t *ep;

    ep = coap_new_endpoint(ctx, &info->addr, info->proto);
    if (!ep) {
      coap_log_warn("cannot create endpoint for CoAP proto %u\n",
                    info->proto);
    } else {
      have_ep = true;
    }
  }
  coap_free_address_info(info_list);
  if (have_ep == false) {
    coap_log_err("No context available for interface '%s'\n",
                 (const char *)my_address->s);
    goto finish;
  }

  /* Add in Multicast listening as appropriate */
#ifdef COAP_LISTEN_MCAST_IPV4
  coap_join_mcast_group_intf(ctx, COAP_LISTEN_MCAST_IPV4, NULL);
#endif /* COAP_LISTEN_MCAST_IPV4 */
#ifdef COAP_LISTEN_MCAST_IPV6
  coap_join_mcast_group_intf(ctx, COAP_LISTEN_MCAST_IPV6, NULL);
#endif /* COAP_LISTEN_MCAST_IPV6 */

  /* Create a resource that the server can respond to with information */
  resource = coap_resource_init(coap_make_str_const("hello"), 0);
  coap_register_handler(resource, COAP_REQUEST_GET,
                        [](auto, auto,
                           const coap_pdu_t *request,
                           auto, coap_pdu_t *response) {
                           coap_show_pdu(COAP_LOG_WARN, request);
                           coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
                           coap_add_data(response, 5,
                                         (const uint8_t *)"world");
                           coap_show_pdu(COAP_LOG_WARN, response);
                         });
  coap_add_resource(ctx, resource);

  /* Create another resource that the server can respond to with information */
  resource = coap_resource_init(coap_make_str_const("hello/my"), 0);
  coap_register_handler(resource, COAP_REQUEST_GET,
                        [](auto, auto,
                           const coap_pdu_t *request,
                           auto, coap_pdu_t *response) {
                           coap_show_pdu(COAP_LOG_WARN, request);
                           coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
                           coap_add_data(response, 8,
                                         (const uint8_t *)"my world");
                           coap_show_pdu(COAP_LOG_WARN, response);
                         });
  coap_add_resource(ctx, resource);

  /* Handle any libcoap I/O requirements */
  while (true) {
    coap_io_process(ctx, COAP_IO_WAIT);
  }

  result = EXIT_SUCCESS;
finish:

  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
