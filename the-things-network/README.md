# The Things Network payload decoder

This directory contains the LoRaWAN payload decoder of our packet transmission protocol, in the file [decoder.js](./decoder.js).\
This decoder is used by our The Things Network application to translate the LoRaWAN binary payloads sent by the end devices into JSON packets, to be forwarded to our cloud function over HTTPS.
