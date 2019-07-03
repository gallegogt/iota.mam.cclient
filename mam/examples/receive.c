/***
 *
 * Example
 *
 * ```
 * bazel run -c opt -- //mam/examples:receive <host> <port> <bundle> <chid>
 * ```
 *
 *  ```bash
 * bazel run -c opt \
 *      -- //mam/examples:receive https://nodes.devnet.thetangle.org 443 \
 *        EGXDBCYBKLGQWOLHHYDFVJ9HPPYXKLVVROJERFUTHDJKFHTPDLVCTUPZTWBEKDHXXDTHUWJVSGECUYTPZ 1
 *  ```
 *
 *
 */
#include <stdio.h>
#include "mam/client.h"

#define MAM_FILENAME "/tmp/mam-examples.bin"

mam_psk_t const psk = {
    .id = {1,  0,  -1, -1, 0,  -1, -1, 0,  0, 1,  -1, 0, 1,  0,  0, 1, 1,  1,  -1, 1, 1, 0,  1,  1, 0,  0,  -1,
           1,  -1, -1, -1, -1, -1, -1, -1, 1, -1, -1, 0, -1, -1, 1, 0, -1, -1, -1, 1, 1, 1,  0,  0, -1, 1,  -1,
           -1, -1, 0,  -1, 1,  -1, -1, -1, 1, 1,  -1, 1, 0,  0,  1, 1, 1,  -1, -1, 0, 0, -1, -1, 1, 0,  -1, 1},
    .key = {-1, 1,  -1, -1, 1,  -1, -1, 0,  0,  0,  -1, -1, 1,  1,  1,  -1, -1, -1, 0,  0,  0,  0,  -1, -1, 1,
            1,  1,  0,  -1, -1, -1, 0,  0,  0,  -1, -1, 1,  -1, 0,  0,  1,  0,  0,  -1, 1,  1,  0,  -1, 0,  0,
            1,  -1, 1,  0,  1,  0,  0,  -1, 1,  1,  -1, 1,  0,  -1, 0,  -1, 1,  -1, -1, -1, 0,  -1, -1, 0,  -1,
            -1, 0,  0,  -1, -1, 1,  -1, 0,  0,  -1, -1, -1, -1, 0,  -1, -1, -1, 1,  -1, -1, 1,  1,  1,  1,  1,
            0,  1,  0,  1,  -1, 0,  0,  1,  0,  1,  0,  0,  1,  0,  -1, 0,  1,  1,  0,  0,  -1, -1, 1,  1,  0,
            0,  1,  -1, 1,  1,  1,  0,  1,  1,  1,  0,  0,  -1, -1, -1, -1, 1,  1,  1,  0,  0,  -1, 0,  1,  -1,
            1,  1,  1,  0,  0,  1,  -1, -1, 0,  -1, 1,  -1, 1,  0,  0,  1,  -1, 0,  1,  -1, 0,  0,  1,  1,  1,
            1,  1,  0,  0,  1,  -1, 1,  -1, 1,  0,  1,  1,  1,  -1, 0,  0,  -1, 1,  1,  0,  -1, -1, 0,  0,  -1,
            1,  0,  1,  -1, 0,  0,  -1, 1,  -1, 1,  1,  1,  -1, 0,  1,  1,  0,  0,  -1, -1, -1, 0,  0,  1,  0,
            1,  0,  -1, 1,  -1, 0,  1,  0,  -1, 1,  1,  -1, -1, 0,  0,  -1, 0,  -1}};

static char const *amazon_ca1_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

void print_flex_trits(char const* const name, tryte_t *const chid, size_t length )
{
  fprintf(stderr, "%s: ", name);
  for (size_t i = 0; i < length; i++) {
    fprintf(stderr, "%c", chid[i]);
  }
  fprintf(stderr, "\n");
}


int main(int argc, char **argv) {
  if (argc < 4 || argc > 5) {
    fprintf(stderr, "usage: recv <host> <port> <bundle_hash> <chid> (optional)\n");
    return EXIT_FAILURE;
  }

  //
  // Config
  //
  const char * host = argv[1];   // HOST
  uint16_t port = atoi(argv[2]); // PORT

  //
  // Create the client
  //
  mam_client_t client = mam_client_new(host, port, NULL, MAM_FILENAME, (port  == 443) ? amazon_ca1_pem : NULL);

  //
  // ADD PSK
  //
  mam_client_add_psk(&client, &psk);

  if (argc == 5) {
    mam_client_add_trusted_pk(&client, (tryte_t *)argv[4], NULL);
  }

  //
  // Receive Data
  //
  size_t payload_size = 0;
  bool is_last_packet = false;
  tryte_t *payload_trytes = NULL;

  tryte_t * bundle_hash = (tryte_t *)argv[3];

  if (mam_client_data_receive(&client, bundle_hash, &payload_trytes, &payload_size, &is_last_packet) == RC_OK) {
    if (payload_trytes == NULL || payload_size == 0)
    {
      fprintf(stderr, "No payload size (%zu)\n", payload_size);
    } else {
      char *payload = calloc(payload_size * 2 + 1, sizeof(char));
      trytes_to_ascii(payload_trytes, payload_size, payload);
      fprintf(stderr, "Payload: %s\n\n Is last package(%d) \n", payload, is_last_packet);
      free(payload);
    }
  } else {
    fprintf(stderr, "mam_api_bundle_read_msg failed\n");
  }
  //
  // Cleanup
  //
  mam_client_destroy(&client);

  return EXIT_SUCCESS;
}