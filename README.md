Minimal CoAP Client and Server Examples for libcoap
===================================================

These examples require [libcoap-3](https://github.com/obgm/libcoap) to
work.

`client.cc` shows a minimal libcoap client that sends a confirmable
UDP `GET /hello` request to a pre-defined CoAP server
([coap.me](http://coap.me) in this example) and outputs a received
response.

`server.cc` is a minimal CoAP UDP server that binds on
`localhost:5683`. It has a single resource `/hello`.

Copyright (C) 2018-2023 Olaf Bergmann <bergmann@tzi.org>

License
=======

This work has been licensed under MIT. See file LICENSE for details.
