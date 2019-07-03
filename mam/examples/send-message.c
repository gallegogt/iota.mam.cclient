/***
 *
 * Example
 *
 * ```
 * bazel run -c opt -- //mam/examples:send-message <host> <port> <seed> <payload> <last_packet>
 * ```
 *
 *  ```bash
 * bazel run -c opt \
 *      -- //mam/examples:send-message https://nodes.devnet.thetangle.org 443 \
 *        HELCUFKGVMSDCQCYXSGGGHFGXV9YNNHWSXGDRWFGEHK9RUEOZEYUVOPWHPE9UZJURPSMYQIDZAUIFWVTL 1
 *  ```
 *
 *
 */
#include <stdio.h>
#include "mam/client.h"
#include <stdio.h>
#include <time.h>

#define PRINT_LN(e) printf("DEBUG FILE %s:%d Error: %s \n", __FILE__, __LINE__, error_2_string(e));
#define MAM_HOST "localhost"
#define MAM_HOST_PORT 14265
#define MAM_FILENAME "/tmp/mam-examples.bin"
#define TEST_MSS_DEPTH 5

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
  retcode_t ret = RC_OK;

  if (argc != 6) {
    fprintf(stderr, "usage: send-msg <host> <port> <seed> <payload> <last_packet>\n");
    return EXIT_FAILURE;
  }
  if (strcmp(argv[5], "yes") && strcmp(argv[5], "no")) {
    fprintf(stderr, "Arg <last_packet> should be \"yes\" or \"no\" only\n");
    return EXIT_FAILURE;
  }

  //
  // Config
  //
  const char * host = argv[1];    // HOST
  uint16_t port = atoi(argv[2]);  // PORT
  tryte_t *seed = (tryte_t *)argv[3];
  const char * payload = argv[4]; // PAYLOAD
  bool last_packet = strcmp(argv[5], "yes") == 0;
  //
  // Create the client
  //
  mam_client_t client = mam_client_new(host, port, seed, MAM_FILENAME, (port  == 443) ? amazon_ca1_pem : NULL);

  //
  // Create a Channel
  //
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE];
  ret = mam_client_channel_create(&client, TEST_MSS_DEPTH, channel_id);

  print_flex_trits("Channel Address", channel_id, FLEX_TRIT_SIZE_243);

  if (ret != RC_OK)
  {
    fprintf(stderr, "mam_client_channel_create failed with err %d\n", ret);
    mam_client_destroy(&client);
    return EXIT_FAILURE;
  }
  ///
  /// CREATE A MESSAGE
  ///
  trit_t message_id[MAM_MSG_ID_SIZE];
  ret = mam_client_attach_message(&client, channel_id, NULL, &psk, NULL, message_id);
  if (ret != RC_OK) {
    fprintf(stderr, "mam_client_attach_message failed with err %d\n", ret);
    mam_client_destroy(&client);
    return EXIT_FAILURE;
  }

  print_flex_trits("Bundle Hash", client.last_bundle_hash, FLEX_TRIT_SIZE_243);

  ///
  /// SEND PACKET
  ///
  int cm = 0;
  time_t rawtime;
  struct tm * timeinfo;

  while (cm < 32) {
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    char buffer[256];
    sprintf(buffer, "{\"current_time\": \"%s\", \"out\": %s}", asctime (timeinfo), payload);
    printf("MSG (%d), Payload: %s \n", cm, buffer);

    ret = mam_client_attach_packet(&client, message_id, buffer, last_packet);
    if ( ret != RC_OK )
    {
      PRINT_LN(ret)
      fprintf(stderr, "mam_client_attach_packet failed with err %d, count_msg(%d)\n", ret, cm);
      mam_client_destroy(&client);
      return EXIT_FAILURE;
    }

    /// CHECK REMAIN PACKETS
    printf("Remain packets %zu \n", mam_client_remaining_packets(&client, channel_id, NULL));

    ++cm;
  }

  ///
  /// DEBUG INFO
  ///
  tryte_t msg_id[MAM_MSG_ID_SIZE / 3];
  trits_to_trytes(message_id, msg_id, MAM_MSG_ID_SIZE);

  print_flex_trits("Message ID", msg_id , MAM_MSG_ID_SIZE / 3);
  print_flex_trits("Bundle Hash", client.last_bundle_hash, FLEX_TRIT_SIZE_243);

  //
  // Cleanup
  //
  mam_client_destroy(&client);
  return EXIT_SUCCESS;
}