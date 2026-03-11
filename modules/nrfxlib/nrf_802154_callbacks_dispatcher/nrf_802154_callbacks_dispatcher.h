/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRF_802154_CALLBACKS_DISPATCHER_H
#define NRF_802154_CALLBACKS_DISPATCHER_H

#include <nrf_802154_const.h>
#include <nrf_802154_types.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __ZEPHYR__
#include <zephyr/sys/iterable_sections.h>
#endif

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

	/** Radio driver deinitialized. */
	void (*deinit)(void);

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

#ifdef __ZEPHYR__
/**
 * @brief Entry for static registration in the iterable section.
 */
struct nrf_802154_cb_dispatch_entry {
	const char *name;
	const struct nrf_802154_callbacks *callbacks;
};

/** Helper to stringify after argument expansion */
#define NRF_802154_CALLBACKS_DISPATCHER_NAME_STR(x)  #x
#define NRF_802154_CALLBACKS_DISPATCHER_NAME_STR_EXPAND(x) \
	NRF_802154_CALLBACKS_DISPATCHER_NAME_STR(x)

/**
 * @brief Statically register a client.
 *
 * Place this macro in file scope (e.g. in your radio init file). Use
 * @ref nrf_802154_callbacks_dispatcher_activate with the same name (as string)
 * to set the active client.
 *
 * @param _entry_name   Unique name for this registration (e.g. openthread_nrf_802154_radio).
 *                      Pass the same string to activate() to switch to this client.
 * @param _callbacks_var Variable of type struct nrf_802154_callbacks.
 */
#define NRF_802154_CALLBACKS_DISPATCHER_REGISTER(_entry_name, _callbacks_var) \
	static const STRUCT_SECTION_ITERABLE(nrf_802154_cb_dispatch_entry, _entry_name) = { \
		.name = NRF_802154_CALLBACKS_DISPATCHER_NAME_STR_EXPAND(_entry_name), \
		.callbacks = &(_callbacks_var), \
	}
#endif

/**
 * @brief Set the active client by name without restarting the shared driver.
 *
 * This helper preserves the original dispatcher behavior and only changes
 * callback routing. Prefer @ref nrf_802154_callbacks_dispatcher_switch for
 * full owner handover.
 *
 * @param name Client name (same as used in REGISTER, e.g. "zigbee_nrf_802154_radio").
 *             Pass NULL or empty string to deactivate (no callbacks will be dispatched).
 *
 * @retval 0       Success.
 * @retval -EINVAL Name not found (no client registered with that name).
 */
int nrf_802154_callbacks_dispatcher_activate(const char *name);

/**
 * @brief Switch active client and optionally restart the shared radio driver.
 *
 * When @p reinit_clients is true, this API performs a full handover:
 * 1. Stops callback routing to the current client.
 * 2. Calls the current client's @c deinit callback (if present) so it can
 *    cancel delayed operations and release stack-owned buffers.
 * 3. Quiesces and deinitializes the shared nRF 802.15.4 driver.
 * 4. Initializes the shared driver again for the next owner.
 * 5. Calls the next client's @c init callback (if present).
 * 6. Activates callback routing for the next client.
 *
 * When @p reinit_clients is false, this API only changes callback routing.
 *
 * @param name            New client name (same name used in REGISTER).
 *                        Pass NULL or empty string to deactivate all clients.
 * @param reinit_clients  If true, perform a full driver teardown/startup for
 *                        the handover. If false, only switch callback routing.
 *
 * @retval 0         Success.
 * @retval -EINVAL   Name not found.
 * @retval -EBUSY    The current radio owner could not be quiesced safely.
 */
int nrf_802154_callbacks_dispatcher_switch(const char *name, bool reinit_clients);

#ifdef __cplusplus
}
#endif

#endif /* NRF_802154_CALLBACKS_DISPATCHER_H */
 