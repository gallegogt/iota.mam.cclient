#include "mam/client.h"
#include "mam/mam/mam_channel_t_set.h"

//
// Debug Inline
//
#define PRINT_LN(e) printf("DEBUG FILE %s:%d Error: %s \n", __FILE__, __LINE__, error_2_string(e));

// Private functions

/**
 * Init client service
 *
 * @param client - [out]
 * @param host - [in]
 * @param host - [in]
 * @param ca_pem - [in]
 */
static void init_service(mam_client_t *const client, char const *const host, uint16_t port, char const *const ca_pem) {
  client->serv.http.path = "/";
  client->serv.http.content_type = "application/json";
  client->serv.http.accept = "application/json";
  client->serv.http.host = host;
  client->serv.http.port = port;
  client->serv.http.api_version = 1;
  client->serv.serializer_type = SR_JSON;
  client->serv.http.ca_pem = ca_pem;

  iota_client_core_init(&client->serv);
  iota_client_extended_init();
}

/**
 * Send Bundle
 *
 * @param client - [in, out]
 * @param bundle - [out]
 *
 */
static retcode_t send_bundle(mam_client_t *const client, bundle_transactions_t *const bundle) {
  retcode_t ret = RC_OK;
  Kerl kerl;
  kerl_init(&kerl);
  bundle_finalize(bundle, &kerl);
  transaction_array_t *out_tx_objs = transaction_array_new();
  hash8019_array_p raw_trytes = hash8019_array_new();
  iota_transaction_t *curr_tx = NULL;
  flex_trit_t trits_8019[FLEX_TRIT_SIZE_8019];

  BUNDLE_FOREACH(bundle, curr_tx) {
    transaction_serialize_on_flex_trits(curr_tx, trits_8019);
    hash_array_push(raw_trytes, trits_8019);
  }

  if ((ret = iota_client_send_trytes(&client->serv,
                                      raw_trytes,
                                      client->iota_depth,
                                      client->iota_mwm, NULL, true, out_tx_objs)) != RC_OK) {
    PRINT_LN(ret)
    goto done;
  }

  // client->last_bundle_hash = ((iota_transaction_t *)utarray_front(bundle))->essence.bundle;
  // fprintf(stderr, "Bundle: ");
  // flex_trit_t * ft_bundle = ((iota_transaction_t *)utarray_front(bundle))->essence.bundle;

  // for (size_t i = 0; i < FLEX_TRIT_SIZE_243; i++) {
  //   fprintf(stderr, "%c", ft_bundle[i]);
  // }
  // fprintf(stderr, "\n");

done:
  hash_array_free(raw_trytes);
  transaction_array_free(out_tx_objs);
  return ret;
}

/**
 * Get Bundle Transactions
 *
 * @param serv - [in, out]
 * @param bundle_hash - [out]
 * @param recv_req - [out]
 */
static transaction_array_t *get_bundle_transactions(iota_client_service_t *const serv,
                                                    flex_trit_t const *const bundle_hash,
                                                    find_transactions_req_t *const recv_req)
{
  flex_trit_t bundle_hash_flex[FLEX_TRIT_SIZE_243];
  transaction_array_t *out_tx_objs = transaction_array_new();
  recv_req->approvees = NULL;
  recv_req->bundles = NULL;
  recv_req->tags = NULL;
  recv_req->addresses = NULL;
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash, NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
  hash243_queue_push(&recv_req->bundles, bundle_hash);

  // TODO - replace with iota_client_get_bundle when it's implemented
  retcode_t err = iota_client_find_transaction_objects(serv, recv_req, out_tx_objs);
  if (err != RC_OK) {
    transaction_array_free(out_tx_objs);
    hash243_queue_free(&recv_req->bundles);
    PRINT_LN(err)
    fprintf(stderr, "iota_client_find_transaction_objects failed with %d\n", err);
    return NULL;
  }

  return out_tx_objs;
}


/**
 * @param lhs
 * @param rhs
 */
static int idx_sort(void const *lhs, void const *rhs)
{
  iota_transaction_t *_lhs = (iota_transaction_t *)lhs;
  iota_transaction_t *_rhs = (iota_transaction_t *)rhs;

  return (transaction_current_index(_lhs) < transaction_current_index(_rhs))
             ? -1
             : (transaction_current_index(_lhs) > transaction_current_index(_rhs));
}

/**
 * @param transactions
 * @param bundle
 */
static void get_first_bundle_from_transactions(transaction_array_t *const transactions,
                                               bundle_transactions_t *const bundle)
{
  iota_transaction_t *tail = NULL;
  iota_transaction_t *curr_tx = NULL;
  iota_transaction_t *prev = NULL;
  utarray_sort(transactions, idx_sort);
  tail = (iota_transaction_t *)utarray_eltptr(transactions, 0);
  bundle_transactions_add(bundle, tail);
  prev = tail;
  TX_OBJS_FOREACH(transactions, curr_tx) {
    if (transaction_current_index(curr_tx) == (transaction_current_index(prev) + 1) &&
        (memcmp(transaction_hash(curr_tx), transaction_trunk(prev), FLEX_TRIT_SIZE_243) == 0)) {
      bundle_transactions_add(bundle, curr_tx);
      prev = curr_tx;
    }
  }
}

/**
 * Receive Bundle
 *
 * @param serv - [in, out]
 * @param bundle_hash - [in]
 * @param bundle - [out]
 *
 */
static retcode_t receive_bundle(iota_client_service_t *const serv,
                                tryte_t const *const bundle_hash,
                                bundle_transactions_t *const bundle)
{
  find_transactions_req_t recv_req;
  transaction_array_t *out_tx_objs = get_bundle_transactions(serv, bundle_hash, &recv_req);

  if (out_tx_objs == NULL) {
    return RC_ERROR;
  }

  get_first_bundle_from_transactions(out_tx_objs, bundle);
  transaction_array_free(out_tx_objs);
  hash243_queue_free(&recv_req.bundles);
  recv_req.bundles = NULL;

  return RC_OK;
}

// Public functions

mam_client_t mam_client_new(char const *const host,
                            uint16_t port,
                            tryte_t const *const mam_seed,
                            char const *const filename,
                            char const *const ca_pem)
{
  mam_client_t client = {
    .s_host = host,
    .s_mam_seed = mam_seed,
    .s_filename = filename,
    .s_port = port,
    .checksum = MAM_MSG_CHECKSUM_MAC, // MAM_MSG_CHECKSUM_MAC,
    .iota_depth = 1, // 1
    .iota_mwm = 14  // 14
  };
  // Init service
  init_service(&client, host, port, ca_pem);

  if (filename == NULL) {
    client.last_error = mam_api_init(&client.mam_api, client.s_mam_seed);
    return client;
  }

  client.last_error = mam_api_load(client.s_filename, &client.mam_api, NULL, 0);
  // Loading or creating MAM API
  if (client.last_error == RC_UTILS_FAILED_TO_OPEN_FILE) {
    client.last_error = mam_api_init(&client.mam_api, client.s_mam_seed);
  }

  return client;
}

void mam_client_destroy(mam_client_t *const client)
{
  iota_client_extended_destroy();
  iota_client_core_destroy(&client->serv);
  // Saving and destroying MAM API
  client->last_error = mam_api_save(&client->mam_api, client->s_filename, NULL, 0);
  client->last_error = mam_api_destroy(&client->mam_api);
}

retcode_t mam_client_get_last_error(mam_client_t *const client)
{
  return client->last_error;
}

retcode_t mam_client_channel_create(mam_client_t *const client,
                                    const size_t height,
                                    tryte_t *const channel_id)
{
  if (mam_channel_t_set_size(client->mam_api.channels) == 0) {
    mam_api_channel_create(&client->mam_api, height, channel_id);
  } else {
    mam_channel_t *channel = &client->mam_api.channels->value;
    trits_to_trytes(
      trits_begin(mam_channel_id(channel)), channel_id, NUM_TRITS_ADDRESS
    );
  }
  return RC_OK;
}

retcode_t mam_client_endpoint_create(mam_client_t* const client,
                                    const size_t height,
                                    tryte_t const *const channel_id,
                                    tryte_t *const endpoint_id)
{
  return mam_api_endpoint_create(&client->mam_api, height, channel_id, endpoint_id);
}

/**
 * @param[in] client
 * @param[in] channel_id
 * @param[in] endpoint_id Optional
 * @param[in] pks
 * @param[in] ntru_pk
 * @param[out] msg_id
 */
retcode_t mam_client_attach_message(mam_client_t* const client,
                                    tryte_t const *const channel_id,
                                    tryte_t const *const endpoint_id,
                                    mam_psk_t const *const psk,
                                    mam_ntru_pk_t const *const ntru_pk,
                                    trit_t *const msg_id)
{
  retcode_t ret = RC_OK;
  mam_psk_t_set_t psks = NULL;
  mam_ntru_pk_t_set_t ntrus = NULL;

  if (psk != NULL && !mam_psk_t_set_contains(&psks, psk)) {
    if ((ret = mam_psk_t_set_add(&psks, psk)) != RC_OK) {
      goto done;
    }
  }

  if (ntru_pk != NULL && !mam_ntru_pk_t_set_contains(&ntrus, ntru_pk)) {
    if ((ret = mam_ntru_pk_t_set_add(&ntrus, ntru_pk)) != RC_OK) {
      goto done;
    }
  }

  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);

  if (endpoint_id != NULL)
  {
    if ((ret = mam_api_bundle_write_header_on_endpoint(
        &client->mam_api, channel_id, endpoint_id, psks, ntrus,
        bundle, msg_id)) != RC_OK)
    {
      goto done;
    }
  }
  else {
    if ((ret = mam_api_bundle_write_header_on_channel(
        &client->mam_api, channel_id, psks, ntrus, bundle, msg_id)) != RC_OK)
    {
      goto done;
    }
  }

  ret = send_bundle(client, bundle);

  if (ret == RC_OK) {
    // Save the last bundle hash created
    memcpy(
      client->last_bundle_hash,
      ((iota_transaction_t *)utarray_front(bundle))->essence.bundle,
      NUM_FLEX_TRITS_BUNDLE);
  }

  done:
    if (psks != NULL) {
      mam_psk_t_set_free(&psks);
    }
    if (ntrus != NULL) {
      mam_ntru_pk_t_set_free(&ntrus);
    }

    bundle_transactions_free(&bundle);

    return ret;
}

retcode_t mam_client_attach_packet(mam_client_t* const client,
                                  trit_t const* const message_id,
                                  char const* const payload,
                                  bool is_last_packet)
{
  retcode_t ret = RC_OK;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);

  tryte_t* payload_trytes = (tryte_t*)malloc(2 * strlen(payload) * sizeof(tryte_t));
  ascii_to_trytes(payload, payload_trytes);
  if ((ret = mam_api_bundle_write_packet(&client->mam_api,
                                          message_id,
                                          payload_trytes,
                                          strlen(payload) * 2,
                                          client->checksum,
                                          is_last_packet,
                                          bundle)) != RC_OK)
  {
    PRINT_LN(ret)
    goto done;
  }

  {
    ret = send_bundle(client, bundle);
    bundle_transactions_free(&bundle);
  }
  done:
    free(payload_trytes);
    bundle_transactions_free(&bundle);
    return ret;
}

size_t mam_client_remaining_packets(mam_client_t* const client,
                                    tryte_t const *const channel_id,
                                    tryte_t const *const endpoint_id)
{
  if (endpoint_id != NULL) {
    return mam_api_endpoint_remaining_sks(&client->mam_api, channel_id, endpoint_id);
  }

  if (channel_id != NULL) {
    return mam_api_channel_remaining_sks(&client->mam_api, channel_id);
  }

  return 0;
}

retcode_t mam_client_data_receive(mam_client_t* const client,
                                  tryte_t const *const bundle_hash,
                                  tryte_t **const payload_trytes,
                                  size_t *const payload_size,
                                  bool *const is_last_packet)
{
  retcode_t ret = RC_OK;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);

  ret = receive_bundle(&client->serv, bundle_hash, bundle);
  if (ret == RC_OK) {
    ret = mam_api_bundle_read(&client->mam_api, bundle, payload_trytes, payload_size, is_last_packet);
  }
  // Cleanup
  {
    bundle_transactions_free(&bundle);
  }

  return ret;
}

retcode_t mam_client_add_psk(mam_client_t* const client, mam_psk_t const * const psk)
{
  retcode_t ret = RC_OK;

  if (!mam_psk_t_set_contains(&client->mam_api.psks, psk)) {
    ret = mam_psk_t_set_add(&client->mam_api.psks, psk);
  }

  return ret;
}
retcode_t mam_client_add_trusted_pk(mam_client_t* const client,
                                    tryte_t *const channel_pk,
                                    tryte_t *const endpoint_pk)
{
  retcode_t ret = RC_OK;

  if (channel_pk != NULL) {
    ret = mam_api_add_trusted_channel_pk(&client->mam_api, channel_pk);
  }

  if (endpoint_pk != NULL) {
    ret = mam_api_add_trusted_endpoint_pk(&client->mam_api, endpoint_pk);
  }


  return ret;

}
retcode_t mam_client_add_trusted(mam_client_t* const client, tryte_t *const channel_id)
{
  return mam_api_add_trusted_channel_pk(&client->mam_api, channel_id);
}

void mam_client_set_message_checksum(mam_client_t *const client, mam_msg_checksum_t checksum) {
  client->checksum = checksum;
}
