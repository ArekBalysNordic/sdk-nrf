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
#define NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS 3

/** Invalid client index (no active client). */
#define NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE UINT32_MAX

#define NRF_802154_EXTENDED_ADDRESS_SIZE 8
#define NRF_802154_PAN_ID_SIZE 2
#define NRF_802154_SHORT_ADDRESS_SIZE 2

struct nrf_802154_radio_client_config {
	uint8_t pan_id[NRF_802154_PAN_ID_SIZE];
	uint8_t short_address[NRF_802154_SHORT_ADDRESS_SIZE];
	uint8_t mac[NRF_802154_EXTENDED_ADDRESS_SIZE];
};

/**
 * @brief Callbacks for the nRF IEEE 802.15.4 radio driver.
 *
 * All callbacks are optional and may be set to NULL if not used.
 */
struct nrf_802154_callbacks {
	void (*init)(void);
	void (*deinit)(void);
	void (*received_timestamp_raw)(uint8_t *data, int8_t power, uint8_t lqi, uint64_t time);
	void (*receive_failed)(nrf_802154_rx_error_t error, uint32_t id);
	void (*tx_ack_started)(const uint8_t *data);
	void (*transmitted_raw)(uint8_t *frame,
				const nrf_802154_transmit_done_metadata_t *metadata);
	void (*transmit_failed)(uint8_t *frame, nrf_802154_tx_error_t error,
				const nrf_802154_transmit_done_metadata_t *metadata);
	void (*energy_detected)(const nrf_802154_energy_detected_t *result);
	void (*energy_detection_failed)(nrf_802154_ed_error_t error);
	void (*set_extended_address)(const uint8_t *address);
	void (*set_short_address)(uint16_t address);
	void (*set_pan_id)(uint16_t pan_id);
	struct nrf_802154_radio_client_config* (*get_config)(void);
#if defined(CONFIG_NRF_802154_SER_HOST)
	void (*serialization_error)(const nrf_802154_ser_err_data_t *err);
#endif
};

#ifdef __ZEPHYR__
/**
 * @brief Entry for static registration in the iterable section.
 */
struct nrf_802154_cb_dispatch_entry {
	const char *name; ///< Unique name for this registration (e.g. openthread).
	const struct nrf_802154_callbacks
		*callbacks;	  ///< Variable of type struct nrf_802154_callbacks.
	bool saved_configuration; ///< Whether the configuration has been saved.
	uint8_t ext_address[8];	  ///< Extended address in format of
				  ///< 0x00:0x01:0x02:0x03:0x04:0x05:0x06:0x07.
	uint16_t short_address;	  ///< Short address in format of 0x0000.
	uint8_t pan_id[2]; ///< PAN ID in little-endian byte format expected by nrf_802154_pan_id_*
			   ///< APIs.
};

/** Helper to stringify after argument expansion */
#define NRF_802154_CALLBACKS_DISPATCHER_NAME_STR(x) #x
#define NRF_802154_CALLBACKS_DISPATCHER_NAME_STR_EXPAND(x)                                         \
	NRF_802154_CALLBACKS_DISPATCHER_NAME_STR(x)

/**
 * @brief Statically register a client.
 *
 * Place this macro in file scope (e.g. in your radio init file). Use
 * @ref nrf_802154_callbacks_dispatcher_activate with the same name (as string)
 * to set the active client.
 *
 * @param _entry_name   Unique name for this registration (e.g. openthread).
 *                      Pass the same string to activate() to switch to this client.
 * @param _callbacks_var Variable of type struct nrf_802154_callbacks.
 */
#define NRF_802154_CALLBACKS_DISPATCHER_REGISTER(_entry_name, _callbacks_var)                      \
	static STRUCT_SECTION_ITERABLE(nrf_802154_cb_dispatch_entry, _entry_name) = {              \
		.name = NRF_802154_CALLBACKS_DISPATCHER_NAME_STR_EXPAND(_entry_name),              \
		.callbacks = &(_callbacks_var),                                                    \
	}
#endif

/**
 * @brief Set the active client by name without restarting the shared driver.
 *
 * This helper preserves the original dispatcher behavior and only changes
 * callback routing. Prefer @ref nrf_802154_callbacks_dispatcher_switch for
 * full owner handover.
 *
 * @param name Client name (same as used in REGISTER, e.g. "zigbee").
 *             Pass NULL or empty string to deactivate (no callbacks will be dispatched).
 *
 * @retval 0       Success.
 * @retval -EINVAL Name not found (no client registered with that name).
 */
int nrf_802154_callbacks_dispatcher_activate(const char *name);

/**
 * @brief Switch active client and optionally restart the shared radio driver.
 *
 * @param name            New client name (same name used in REGISTER).
 *                        Pass NULL or empty string to deactivate all clients.
 * @retval 0         Success.
 * @retval -EINVAL   Name not found.
 */
int nrf_802154_callbacks_dispatcher_switch(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* NRF_802154_CALLBACKS_DISPATCHER_H */
