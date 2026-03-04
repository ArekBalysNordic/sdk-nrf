/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

 #ifndef NRF_802154_CALLBACKS_DISPATCHER_H
 #define NRF_802154_CALLBACKS_DISPATCHER_H
 
 #include <nrf_802154_const.h>
 #include <nrf_802154_types.h>
 #include <stdint.h>

 /** Maximum number of clients that can be registered. */
 #define NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS 8

 /** Invalid client index (no active client). */
 #define NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE   UINT32_MAX

 /**
  * @brief Callbacks for the nRF IEEE 802.15.4 radio driver.
  *
  * All callbacks are optional and may be set to NULL if not used.
  */
 struct nrf_802154_callbacks {

    /** Radio driver initialized. */
    void (*init)(void);

 	/** Frame received with timestamp. */
 	void (*received_timestamp_raw)(uint8_t *data, int8_t power, uint8_t lqi, uint64_t time);

 	/** Receive operation failed. */
 	void (*receive_failed)(nrf_802154_rx_error_t error, uint32_t id);

 	/** Transmission of ACK started. */
 	void (*tx_ack_started)(const uint8_t *data);

 	/** Frame transmitted. */
 	void (*transmitted_raw)(uint8_t *frame,
 				const nrf_802154_transmit_done_metadata_t *metadata);

 	/** Transmission failed. */
 	void (*transmit_failed)(uint8_t *frame, nrf_802154_tx_error_t error,
 				const nrf_802154_transmit_done_metadata_t *metadata);

 	/** Energy detection completed. */
 	void (*energy_detected)(const nrf_802154_energy_detected_t *result);

 	/** Energy detection failed. */
 	void (*energy_detection_failed)(nrf_802154_ed_error_t error);

 #if defined(CONFIG_NRF_802154_SER_HOST)
 	/** Serialization error (host only). */
 	void (*serialization_error)(const nrf_802154_ser_err_data_t *err);
 #endif
 };

 /**
  * @brief Register a client and get an assigned index.
  *
  * The dispatcher assigns the first free slot and returns its index.
  * Only one client can be active at a time. Use
  * @ref nrf_802154_callbacks_dispatcher_activate with the returned index to set the active client.
  *
  * @param callbacks  Pointer to the client callbacks. Must not be NULL.
  * @param out_index  On success, the assigned client index (0 .. NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS - 1)
  *                   is written here. Must not be NULL.
  *
  * @retval 0        Success; @p out_index contains the assigned index.
  * @retval -EINVAL  @p callbacks or @p out_index is NULL.
  * @retval -ENOMEM  No free slot; maximum number of clients already registered.
  */
 int nrf_802154_callbacks_dispatcher_register(const struct nrf_802154_callbacks *callbacks,
 					     uint32_t *out_index);

 /**
  * @brief Set the active client by index.
  *
  * Radio driver callbacks are dispatched only to the active client.
  * The client must be registered before it can be activated.
  *
  * @param index Client index to activate, or NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE
  *              to deactivate (no callbacks will be dispatched).
  *
  * @retval 0       Success.
  * @retval -EINVAL Invalid index or no client registered at that index.
  */
 int nrf_802154_callbacks_dispatcher_activate(uint32_t index);

 /**
  * @brief Unregister a client by index.
  *
  * Frees the slot so it can be reused by a subsequent register.
  * If the unregistered client was active, no client is active afterward.
  *
  * @param index Client index previously returned by @ref nrf_802154_callbacks_dispatcher_register.
  *
  * @retval 0       Success.
  * @retval -EINVAL Invalid index or no client registered at that index.
  */
 int nrf_802154_callbacks_dispatcher_unregister(uint32_t index);

 #endif /* NRF_802154_CALLBACKS_DISPATCHER_H */
 