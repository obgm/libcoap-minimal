/* minimal CoAP client
 *
 * Copyright (C) 2018-2024 Olaf Bergmann <bergmann@tzi.org>
 */

/*
 * The client can be run as
 *   ./client
 * in which case the CoAP URI to make the request against is defined
 * by COAP_CLIENT_URI, or
 *   ./client CoAP_Uri
 * where CoAP_Uri is a correctly formatted CoAP URI.
 */

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "common.hh"

static int have_response = 0;

#ifndef COAP_CLIENT_URI
#define COAP_CLIENT_URI "coap://coap.me/hello"
#endif

int
main(int argc, char *argv[]) {
  coap_context_t *ctx = nullptr;
  coap_session_t *session = nullptr;
  coap_optlist_t *optlist = nullptr;
  coap_address_t dst;
  coap_pdu_t *pdu = nullptr;
  int result = EXIT_FAILURE;;
  int len;
  int res;
  unsigned int wait_ms;
  coap_uri_t uri;
  const char *coap_uri = COAP_CLIENT_URI;
  int is_mcast;
#define BUFSIZE 100
  unsigned char scratch[BUFSIZE];

  /* Support run-time defining of CoAP URIs */
  if (argc > 1) {
    coap_uri = argv[1];
  }

  /* Initialize libcoap library */
  coap_startup();

  /* Set logging level */
  coap_set_log_level(COAP_LOG_WARN);

  /* Parse the URI */
  len = coap_split_uri((const unsigned char *)coap_uri, strlen(coap_uri), &uri);
  if (len != 0) {
    coap_log_warn("Failed to parse uri %s\n", coap_uri);
    goto finish;
  }

  /* resolve destination address where server should be sent */
  len = resolve_address(&uri.host, uri.port, &dst, 1 << uri.scheme);
  if (len <= 0) {
    coap_log_warn("Failed to resolve address %*.*s\n", (int)uri.host.length,
                  (int)uri.host.length, (const char *)uri.host.s);
    goto finish;
  }
  is_mcast = coap_is_mcast(&dst);

  /* create CoAP context and a client session */
  if (!(ctx = coap_new_context(nullptr))) {
    coap_log_emerg("cannot create libcoap context\n");
    goto finish;
  }
  /* Support large responses */
  coap_context_set_block_mode(ctx,
                              COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);

  if (uri.scheme == COAP_URI_SCHEME_COAP) {
    session = coap_new_client_session(ctx, NULL, &dst,
                                      COAP_PROTO_UDP);
  } else if (uri.scheme == COAP_URI_SCHEME_COAP_TCP) {
    session = coap_new_client_session(ctx, NULL, &dst,
                                      COAP_PROTO_TCP);
  }
  if (!session) {
    coap_log_emerg("cannot create client session\n");
    goto finish;
  }

  /* coap_register_response_handler(ctx, response_handler); */
  coap_register_response_handler(ctx,
                                 [](auto, auto, const coap_pdu_t *received, auto) {
                                    size_t len;
                                    const uint8_t *databuf;
                                    size_t offset;
                                    size_t total;

                                    have_response = 1;
                                    coap_show_pdu(COAP_LOG_WARN, received);
                                    if (coap_get_data_large(received, &len, &databuf, &offset, &total)) {
                                      fwrite(databuf, 1, len, stdout);
                                      fwrite("\n", 1, 1, stdout);
                                    }
                                    return COAP_RESPONSE_OK;
                                  });
  /* construct CoAP message */
  pdu = coap_pdu_init(is_mcast ? COAP_MESSAGE_NON : COAP_MESSAGE_CON,
                      COAP_REQUEST_CODE_GET,
                      coap_new_message_id(session),
                      coap_session_max_pdu_size(session));
  if (!pdu) {
    coap_log_emerg("cannot create PDU\n");
    goto finish;
  }

  /* Add option list (which will be sorted) to the PDU */
  len = coap_uri_into_options(&uri, &dst, &optlist, 1, scratch, sizeof(scratch));
  if (len) {
    coap_log_warn("Failed to create options\n");
    goto finish;
  }

  if (optlist) {
    res = coap_add_optlist_pdu(pdu, &optlist);
    if (res != 1) {
      coap_log_warn("Failed to add options to PDU\n");
      goto finish;
    }
  }

  coap_show_pdu(COAP_LOG_WARN, pdu);

  /* and send the PDU */
  if (coap_send(session, pdu) == COAP_INVALID_MID) {
    coap_log_err("cannot send CoAP pdu\n");
    goto finish;
  }

  wait_ms = (coap_session_get_default_leisure(session).integer_part + 1) * 1000;

  while (have_response == 0 || is_mcast) {
    res = coap_io_process(ctx, 1000);
    if (res >= 0) {
      if (wait_ms > 0) {
        if ((unsigned)res >= wait_ms) {
          fprintf(stdout, "timeout\n");
          break;
        } else {
          wait_ms -= res;
        }
      }
    }
  }

  result = EXIT_SUCCESS;
finish:
  coap_delete_optlist(optlist);
  coap_session_release(session);
  coap_free_context(ctx);
  coap_cleanup();

  return result;
}
