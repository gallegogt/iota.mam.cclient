#include <unity/unity.h>

#include "mam/client.h"

#define MAM_HOST "localhost"
#define MAM_HOST_PORT 14265
#define DUMMY_SEED                                                             \
  "DUMMYSEEDDUMMYSEEDDUMMYSEEDDUMMYSEEDDUMMYSEEDDUMMYSEEDDUMMYSEEDDUMMYSEED99" \
  "9999999"
#define MAM_FILENAME "/tmp/test.mam.bin"

static void test_create_mam_client_without_filename() {
  // create a new client
  mam_client_t client = mam_client_new(MAM_HOST, MAM_HOST_PORT, (tryte_t *)DUMMY_SEED, NULL, NULL);
  TEST_ASSERT(mam_client_get_last_error(&client) == RC_OK);
  mam_client_destroy(&client);
  TEST_ASSERT(mam_client_get_last_error(&client) == RC_OK);
}

static void test_create_mam_client_with_filename() {
  // create a new client
  mam_client_t client = mam_client_new(MAM_HOST, MAM_HOST_PORT, (tryte_t *)DUMMY_SEED, MAM_FILENAME, NULL);
  TEST_ASSERT(mam_client_get_last_error(&client) == RC_OK);
  mam_client_destroy(&client);
  TEST_ASSERT(mam_client_get_last_error(&client) == RC_OK);
}

static void test_create_channel() {
  mam_client_t client = mam_client_new(MAM_HOST, MAM_HOST_PORT, (tryte_t *)DUMMY_SEED, MAM_FILENAME, NULL);
  tryte_t ch_id[MAM_CHANNEL_ID_TRYTE_SIZE];
  size_t height = 6;

  TEST_ASSERT(mam_client_channel_create(&client, height, ch_id) == RC_OK);
  TEST_ASSERT_EQUAL_INT(mam_client_remaining_packets(&client, ch_id, NULL), MAM_MSS_MAX_SKN(height) + 1);

  mam_client_destroy(&client);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_create_mam_client_without_filename);
  RUN_TEST(test_create_mam_client_with_filename);
  RUN_TEST(test_create_channel);

  return UNITY_END();
}