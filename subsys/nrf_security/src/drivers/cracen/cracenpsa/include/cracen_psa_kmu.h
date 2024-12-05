/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef CRACEN_PSA_KMU_H
#define CRACEN_PSA_KMU_H

#include <psa/crypto.h>

#ifdef CONFIG_BUILD_WITH_TFM
/** A slot number identifying a key in a driver.
 *
 * Values of this type are used to identify built-in keys.
 */
typedef uint64_t psa_drv_slot_number_t;
#endif

#define PSA_KEY_LOCATION_CRACEN_KMU (PSA_KEY_LOCATION_VENDOR_FLAG | ('N' << 8) | 'K')

/* Key id 0x7fffXYZZ */
/* X: Key usage scheme */
/* Y: Reserved (0) */
/* Z: KMU Slot index */

/* Construct a PSA Key handle for key stored in KMU */
#define PSA_KEY_HANDLE_FROM_CRACEN_KMU_SLOT(scheme, slot_id)                                       \
	(0x7fff0000 | ((scheme) << 12) | ((slot_id) & 0xff))

/* Retrieve key usage scheme from PSA key id. */
#define CRACEN_PSA_GET_KEY_USAGE_SCHEME(key_id) (((key_id) >> 12) & 0xf)

/* Retrieve KMU slot number for PSA key id. */
#define CRACEN_PSA_GET_KMU_SLOT(key_id) ((key_id) & 0xff)

enum cracen_kmu_metadata_key_usage_scheme {
	/**
	 * These keys can only be pushed to CRACEN's protected RAM.
	 * The keys are not encrypted. Only AES supported.
	 */
	CRACEN_KMU_KEY_USAGE_SCHEME_PROTECTED,
	/**
	 * CRACEN's IKG seed uses 3 key slots. Pushed to the seed register.
	 */
	CRACEN_KMU_KEY_USAGE_SCHEME_SEED,
	/**
	 * These keys are stored in encrypted form. They will be decrypted
	 * to @ref kmu_push_area for usage.
	 */
	CRACEN_KMU_KEY_USAGE_SCHEME_ENCRYPTED,
	/**
	 * These keys are not encrypted. Pushed to @ref kmu_push_area.
	 */
	CRACEN_KMU_KEY_USAGE_SCHEME_RAW
};

/**
 * @brief Retrieves the slot number for a given key handle.
 *
 * @param[in]  key_id      Key handler.
 * @param[out] lifetime    Lifetime for key.
 * @param[out] slot_number The key's slot number.
 *
 * @return psa_status_t
 */
psa_status_t cracen_kmu_get_key_slot(mbedtls_svc_key_id_t key_id, psa_key_lifetime_t *lifetime,
				     psa_drv_slot_number_t *slot_number);

#ifdef CONFIG_CRACEN_TRANSLATE_ITS_TO_KMU
/**
 * @brief Register the KMU slots range for a given ITS key range.
 *
 * This function registers a KMU slots range starting from kmu_start and of size slot_count.
 *
 * If the provided range overlaps with any previously registered range, the function will return
 * an error.
 *
 * The same scheme is used for all keys in the range. If you want to use different schemes, you need
 * to divide desired slots and define more ranges.
 *
 * @param its_start ITS key id base, for example 0x30000.
 * @param kmu_start KMU slots range start index.
 * @param slot_count Amount of slots in the range.
 * @param kmu_key_size Number of KMU slots needed to store a single key. This value depends on used
 * algorithm and key type.
 * @param scheme A KMU key usage scheme used for all keys in the range.
 * @return PSA_SUCCESS on success.
 * @return PSA_ERROR_STORAGE_FAILURE if there is no space to register a new range.
 * @return PSA_ERROR_INVALID_ARGUMENT if the range overlaps with any previously registered range or
 * the range is beyond the possible KMU slots space
 */
psa_status_t cracen_register_kmu_range(mbedtls_svc_key_id_t its_start,
				       psa_drv_slot_number_t kmu_start, uint16_t slot_count,
				       uint8_t kmu_key_size,
				       cracen_kmu_metadata_key_usage_scheme scheme);

/**
 * @brief Translate the ITS key to KMU slot using one of the registered ranges.
 *
 * This function requires that the range has been registered using @ref cracen_register_kmu_range.
 *
 * Translation will be performed in the following way:
 * 1. The function iterates through all registered ranges and tries to find a matching translation
 *    by comparing the attributes->key_id value base (attributes->key_id & 0xFFFF0000) with the ITS
 *    key base of the range.
 * 2. If the range is found, the function calculates the offset for the slot according to
 *    Encryption, ITS key_id and the range:
 *    - Gets the ITS key without the base (attributes->key_id & 0x0000FFFF).
 *    - Moves the offset by requested key_id multiplied by the single key slot size (defined for the
 *    range).
 *    - If the range is encrypted, the function will move the slot by 2 for each single offset
 *    movement.
 * 3. The function checks if the calculated slot is within the range and returns the slot number.
 *
 *
 * @param attributes The PSA key attributes that contains key_id value.
 * @param slot_id The resulting KMU slot number if the translation finish successfully.
 * @return PSA_SUCCESS if the translation was successful.
 * @return PSA_ERROR_INVALID_ARGUMENT if the attributes or slot_id is NULL.
 * @return PSA_ERROR_INSUFFICIENT_STORAGE if the range was found, but the slot_number is not located within the range.
 * @return PSA_ERROR_INVALID_HANDLE if the any range was found for given PSA key_id base.
 */
psa_status_t cracen_kmu_translate_key_from_its(const psa_key_attributes_t *attributes,
					       psa_drv_slot_number_t *slot_id);
#endif /* CONFIG_CRACEN_TRANSLATE_ITS_TO_KMU */

#endif /* CRACEN_PSA_KMU_H */
