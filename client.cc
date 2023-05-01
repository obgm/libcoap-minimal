/* minimal CoAP client
 *
 * Copyright (C) 2018-2023 Olaf Bergmann <bergmann@tzi.org>
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "common.hh"

static int have_response = 0;

int
main(void) {
  coap_context_t  *ctx = nullptr;
  coap_session_t *session = nullptr;
  coap_address_t dst;
  coap_pdu_t *pdu = nullptr;
  int result = EXIT_FAILURE;;

  coap_startup();

  /* Set logging level */
  coap_set_log_level(COAP_LOG_WARN);

  /* resolve destination address where server should be sent */
  if (resolve_address("coap.me", "5683", &dst) < 0) {
    coap_log_crit("failed to resolve address\n");
    goto finish;
  }

  /* create CoAP context and a client session */
  if (!(ctx = coap_new_context(nullptr))) {
    coap_log_emerg("cannot create libcoap context\n");
    goto finish;
  }
  /* Support large responses */
  coap_context_set_block_mode(ctx,
                  COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);

  if (!(session = coap_new_client_session(ctx, nullptr, &dst,
                                                  COAP_PROTO_UDP))) {
    coap_log_emerg("cannot create client session\n");
    goto finish;
  }

  /* coap_register_response_handler(ctx, response_handler); */
  coap_register_response_handler(ctx, [](auto, auto,
                                         const coap_pdu_t *received,
                                         auto) {
                                        have_response = 1;
                                        coap_show_pdu(COAP_LOG_WARN, received);
                                        return COAP_RESPONSE_OK;
                                      });
  /* construct CoAP message */
  pdu = coap_pdu_init(COAP_MESSAGE_CON,
                      COAP_REQUEST_CODE_GET,
                      coap_new_message_id(session),
                      coap_session_max_pdu_size(session));
  if (!pdu) {
    coap_log_emerg("cannot create PDU\n" );
    goto finish;
  }

  /* add a Uri-Path option */
  coap_add_option(pdu, COAP_OPTION_URI_PATH, 5,
                  reinterpret_cast<const uint8_t *>("hello"));

  coap_show_pdu(COAP_LOG_WARN, pdu);
  /* and send the PDU */
  if (coap_send(session, pdu) == COAP_INVALID_MID) {
    coap_log_err("cannot send CoAP pdu\n");
    goto finish;
  }

  while (have_response == 0)
    coap_io_process(ctx, COAP_IO_WAIT);

  result = EXIT_SUCCESS;
 finish:

  coap_session_release(session);
  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
