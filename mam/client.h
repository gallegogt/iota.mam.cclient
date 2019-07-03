#ifndef __SRC_MAM_CLIENT_H__
#define __SRC_MAM_CLIENT_H__

#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "common/trinary/trit_tryte.h"
#include "common/trinary/tryte_ascii.h"
#include "mam/api/api.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct iota_mam_client_s {
  tryte_t const *s_mam_seed;
  char const *s_host;
  uint16_t s_port;
  char const *s_filename;
  retcode_t last_error;
  mam_api_t mam_api;
  iota_client_service_t serv;
  flex_trit_t last_bundle_hash[NUM_FLEX_TRITS_BUNDLE];
  mam_msg_checksum_t checksum;

  // IOTA Service
  uint32_t iota_depth;
  uint32_t iota_mwm;
} mam_client_t;

/**
 * Create new MAM Client
 *
 * @param[in] host
 * @param[in] port
 * @param[in] mam_seed
 * @param[in] filename - The file name where to serialize the API into
 * @param[in] ca_pem
 */
mam_client_t mam_client_new(char const *const host,
                            uint16_t port,
                            tryte_t const *const mam_seed,
                            char const *const filename,
                            char const *const ca_pem);

/**
 * @param client - [in]
 * @param height - [in] the desired height of the underlying merkle tree which
 *                      will determine how many signatures we can generate with
 *                      this channel. Here we choose a height of 5, meaning that
 *                      we will be able to sign 2^5 = 32 packets with this channel.
 * @param channel_id - [out]
 */
retcode_t mam_client_channel_create(mam_client_t* const client,
                                    const size_t height,
                                    tryte_t *const channel_id);

/**
 * @param[in] client
 * @param[in] height    The desired height of the underlying merkle tree which
 *                      will determine how many signatures we can generate with
 *                      this endpoint. Here we choose a height of 5, meaning that
 *                      we will be able to sign 2^5 = 32 packets with this endpoint.
 * @param[in] channel_id
 * @param[out] endpoint_id
 */
retcode_t mam_client_endpoint_create(mam_client_t* const client,
                                    const size_t height,
                                    tryte_t const *const channel_id,
                                    tryte_t *const endpoint_id);

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
                                    trit_t *const msg_id);

/**
 * @param[in] client
 * @param[in] message_id - The payload to write
 * @param[in] payload - The payload to write
 * @param[in] is_last_packet - True if this is the last packet to be written
 */
retcode_t mam_client_attach_packet(mam_client_t* const client,
                                  trit_t const* const message_id,
                                  char const* const payload,
                                  bool is_last_packet);

/**
 * @brief Returns the number of remaining packets
 *
 * @param[in] client      CClient
 * @param[in] channel_id  The parent channel id
 * @param[in] endpoint_id The endpoint id
 *
 * @return the number of remaining packets
 */
size_t mam_client_remaining_packets(mam_client_t* const client,
                                    tryte_t const *const channel_id,
                                    tryte_t const *const endpoint_id);

/**
 *
 * @param[in] client
 * @param[in] bundle_hash
 * @param[out] payload_size
 * @param[out] is_last_packet
 *
 * @return payload
 */
retcode_t mam_client_data_receive(mam_client_t* const client,
                                  tryte_t const *const bundle_hash,
                                  tryte_t **const payload_trytes,
                                  size_t *const payload_size,
                                  bool *const is_last_packet);

/**
 * Add new psk
 *
 * @param client - [int, out]
 * @param psk - [int]
 *
 */
retcode_t mam_client_add_psk(mam_client_t* const client,
                              mam_psk_t const * const psk);

/**
 * Add new psk
 *
 * @param[int, out] client
 * @param[int] channel_pk
 * @param[int] endpoint_pk
 *
 */
retcode_t mam_client_add_trusted_pk(mam_client_t* const client,
                                tryte_t *const channel_pk,
                                tryte_t *const endpoint_pk);

/**
 * Destroy a Client
 *
 * @param client - [int, out]
 */
void mam_client_destroy(mam_client_t *const client);

/**
 * Get Last Error
 *
 * @param client - [in]
 */
retcode_t mam_client_get_last_error(mam_client_t *const client);

/**
 * @param client - [in, out]
 * @param checksum - [in]
 */
void mam_client_set_message_checksum(mam_client_t *const client, mam_msg_checksum_t checksum);

#ifdef __cplusplus
}
#endif

#endif // __MAM_CCLIENT_H__