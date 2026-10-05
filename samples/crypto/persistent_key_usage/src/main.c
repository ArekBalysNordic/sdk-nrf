/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>
#include <psa/crypto.h>

#ifdef CONFIG_FLASH
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#endif

#ifdef CONFIG_BUILD_WITH_TFM
#include <tfm_ns_interface.h>
#endif

#include "init.h"

#define APP_SUCCESS	    (0)
#define APP_ERROR	    (-1)
#define APP_SUCCESS_MESSAGE "Example finished successfully!"
#define APP_ERROR_MESSAGE   "Example exited with error!"

#define PRINT_HEX(p_label, p_text, len)                                                            \
	({                                                                                         \
		LOG_INF("---- %s (len: %u): ----", p_label, len);                                  \
		LOG_HEXDUMP_INF(p_text, len, "Content:");                                          \
		LOG_INF("---- %s end  ----", p_label);                                             \
	})

LOG_MODULE_REGISTER(persistent_key_usage, LOG_LEVEL_DBG);

/* ====================================================================== */
/*			Global variables/defines for the persistent key  example	  */

/* The key id for the persistent key. The macros PSA_KEY_ID_USER_MIN and
 * PSA_KEY_ID_USER_MAX define the range of freely available key ids.
 */
#define SAMPLE_PERS_KEY_ID				PSA_KEY_ID_USER_MIN
#define SAMPLE_KEY_TYPE					PSA_KEY_TYPE_AES
#define SAMPLE_ALG					PSA_ALG_CTR
#define NRF_CRYPTO_EXAMPLE_PERSISTENT_KEY_MAX_TEXT_SIZE (100)

/*
 * Simulated factory-data secret (same length and flash alignment pattern as Matter
 * FactoryDataProvider::MoveDACPrivateKeyToSecureStorage()).
 */
#define SIM_SECRET_LEN 32
#define SIM_SECRET_OFFSET 7
#define SIM_FACTORY_BLOB_SIZE 128

#define ROUND_DOWN(value, divisor) ((value) / (divisor) * (divisor))
#define ROUND_UP(value, divisor)   (ROUND_DOWN((value) + (divisor) - 1, (divisor)))

static psa_key_id_t key_id;

/* Below text is used as plaintext for encryption/decryption */
static uint8_t m_plain_text[NRF_CRYPTO_EXAMPLE_PERSISTENT_KEY_MAX_TEXT_SIZE] = {
	"Example string to demonstrate basic usage of a persistent key."};

static uint8_t m_encrypted_text[PSA_CIPHER_ENCRYPT_OUTPUT_SIZE(
	SAMPLE_KEY_TYPE, SAMPLE_ALG, NRF_CRYPTO_EXAMPLE_PERSISTENT_KEY_MAX_TEXT_SIZE)];
static uint8_t m_decrypted_text[NRF_CRYPTO_EXAMPLE_PERSISTENT_KEY_MAX_TEXT_SIZE];
/* ====================================================================== */

int crypto_init(void)
{
	psa_status_t status;

#if	defined(CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK) || \
	defined(CONFIG_SECURE_STORAGE_ITS_TRANSFORM_AEAD_KEY_PROVIDER_HUK_LIBRARY)
	write_huk();
#endif

	/* Initialize PSA Crypto */
	status = psa_crypto_init();
	if (status != PSA_SUCCESS) {
		return APP_ERROR;
	}

	return APP_SUCCESS;
}

int crypto_finish(void)
{
	psa_status_t status;

	/* Destroy the key handle */
	status = psa_destroy_key(key_id);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_destroy_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	return APP_SUCCESS;
}

#if FIXED_PARTITION_EXISTS(storage_partition)

static int prepare_simulated_factory_blob(uint8_t *blob, size_t blob_size,
					  uint8_t secret[static SIM_SECRET_LEN])
{
	if (blob_size < SIM_SECRET_OFFSET + SIM_SECRET_LEN) {
		return APP_ERROR;
	}

	memset(blob, 0xA5, blob_size);
	for (size_t i = 0; i < SIM_SECRET_LEN; i++) {
		secret[i] = (uint8_t)(0x5A + i);
	}
	memcpy(blob + SIM_SECRET_OFFSET, secret, SIM_SECRET_LEN);

	return APP_SUCCESS;
}

static int write_simulated_factory_data(const struct flash_area *fa,
					const uint8_t *blob, size_t blob_size)
{
	int err;

	err = flash_area_erase(fa, 0, fa->fa_size);
	if (err != 0) {
		LOG_ERR("flash_area_erase failed: %d", err);
		return APP_ERROR;
	}

	err = flash_area_write(fa, 0, blob, blob_size);
	if (err != 0) {
		LOG_ERR("flash_area_write failed: %d", err);
		return APP_ERROR;
	}

	LOG_INF("Wrote simulated factory-data blob to storage partition (%u bytes)", blob_size);
	PRINT_HEX("Factory blob", blob, blob_size);

	return APP_SUCCESS;
}

static int import_secret_to_persistent_storage(const uint8_t *secret)
{
	psa_status_t status;
	psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

	(void)psa_destroy_key(SAMPLE_PERS_KEY_ID);

	psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
	psa_set_key_algorithm(&key_attributes, SAMPLE_ALG);
	psa_set_key_type(&key_attributes, SAMPLE_KEY_TYPE);
	psa_set_key_bits(&key_attributes, SIM_SECRET_LEN * 8);
	psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_PERSISTENT);
	psa_set_key_id(&key_attributes, SAMPLE_PERS_KEY_ID);

	status = psa_import_key(&key_attributes, secret, SIM_SECRET_LEN, &key_id);
	psa_reset_key_attributes(&key_attributes);
	if (status != PSA_SUCCESS) {
		LOG_ERR("psa_import_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("Copied secret from flash to PSA persistent storage (id: 0x%x)", key_id);

	status = psa_purge_key(key_id);
	if (status != PSA_SUCCESS) {
		LOG_ERR("psa_purge_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	return APP_SUCCESS;
}

static int zeroize_secret_in_flash(const struct flash_area *fa)
{
	const struct device *flash_dev = flash_area_get_device(fa);
	const struct flash_parameters *flash_params = flash_get_parameters(flash_dev);
	size_t write_block_size = flash_params->write_block_size;
	size_t aligned_secret_offset = ROUND_DOWN(SIM_SECRET_OFFSET, write_block_size);
	size_t bytes_before = SIM_SECRET_OFFSET % write_block_size;
	size_t required_flash_space =
		ROUND_UP(SIM_SECRET_LEN + bytes_before, write_block_size);
	size_t bytes_after = required_flash_space - bytes_before - SIM_SECRET_LEN;
	uint8_t *write_buffer;
	int err;

	if (SIM_SECRET_OFFSET + SIM_SECRET_LEN > SIM_FACTORY_BLOB_SIZE) {
		LOG_ERR("Simulated factory blob too small for secret");
		return APP_ERROR;
	}

	write_buffer = k_malloc(required_flash_space);
	if (write_buffer == NULL) {
		LOG_ERR("Failed to allocate write buffer (%u bytes)", required_flash_space);
		return APP_ERROR;
	}

	err = flash_area_read(fa, aligned_secret_offset, write_buffer, bytes_before);
	if (err != 0) {
		LOG_ERR("flash_area_read (prefix) failed: %d", err);
		k_free(write_buffer);
		return APP_ERROR;
	}

	memset(write_buffer + bytes_before, 0x00, SIM_SECRET_LEN);

	err = flash_area_read(fa, SIM_SECRET_OFFSET + SIM_SECRET_LEN,
			      write_buffer + bytes_before + SIM_SECRET_LEN, bytes_after);
	if (err != 0) {
		LOG_ERR("flash_area_read (suffix) failed: %d", err);
		k_free(write_buffer);
		return APP_ERROR;
	}

	LOG_INF("Zeroizing secret in flash (write_block_size=%u, aligned_offset=%u, "
		"write_len=%u)",
		write_block_size, aligned_secret_offset, required_flash_space);

	err = flash_write(flash_dev, fa->fa_off + aligned_secret_offset,
			  write_buffer, required_flash_space);
	k_free(write_buffer);
	if (err != 0) {
		LOG_ERR("flash_write failed: %d", err);
		return APP_ERROR;
	}

	return APP_SUCCESS;
}

static int verify_secret_zeroized_in_flash(const struct flash_area *fa)
{
	uint8_t cleared_secret[SIM_SECRET_LEN];
	uint8_t expected_zero[SIM_SECRET_LEN];
	int err;

	memset(expected_zero, 0x00, sizeof(expected_zero));

	err = flash_area_read(fa, SIM_SECRET_OFFSET, cleared_secret, sizeof(cleared_secret));
	if (err != 0) {
		LOG_ERR("flash_area_read (verify) failed: %d", err);
		return APP_ERROR;
	}

	if (memcmp(cleared_secret, expected_zero, sizeof(expected_zero)) != 0) {
		LOG_ERR("Secret was not zeroized in flash");
		PRINT_HEX("Remaining secret bytes", cleared_secret, sizeof(cleared_secret));
		return APP_ERROR;
	}

	LOG_INF("MRAM secret zeroized successfully");
	return APP_SUCCESS;
}

static int migrate_simulated_factory_secret(void)
{
	const struct flash_area *fa;
	uint8_t factory_blob[SIM_FACTORY_BLOB_SIZE];
	uint8_t secret[SIM_SECRET_LEN];
	int err;

	if (flash_area_open(FIXED_PARTITION_ID(storage_partition), &fa) != 0) {
		LOG_ERR("flash_area_open(storage_partition) failed");
		return APP_ERROR;
	}

	err = prepare_simulated_factory_blob(factory_blob, sizeof(factory_blob), secret);
	if (err != APP_SUCCESS) {
		flash_area_close(fa);
		return err;
	}

	err = write_simulated_factory_data(fa, factory_blob, sizeof(factory_blob));
	if (err != APP_SUCCESS) {
		flash_area_close(fa);
		return err;
	}

	err = import_secret_to_persistent_storage(secret);
	if (err != APP_SUCCESS) {
		flash_area_close(fa);
		return err;
	}

	err = zeroize_secret_in_flash(fa);
	if (err != APP_SUCCESS) {
		flash_area_close(fa);
		return err;
	}

	err = verify_secret_zeroized_in_flash(fa);
	flash_area_close(fa);

	return err;
}

#else /* !FIXED_PARTITION_EXISTS(storage_partition) */

static int migrate_simulated_factory_secret(void)
{
	LOG_WRN("storage_partition is unavailable; falling back to psa_generate_key");
	return APP_ERROR;
}

#endif /* FIXED_PARTITION_EXISTS(storage_partition) */

int generate_persistent_key(void)
{
	psa_status_t status;

	LOG_INF("Generating random persistent AES key...");

	/* Configure the key attributes */
	psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
	psa_set_key_algorithm(&key_attributes, SAMPLE_ALG);
	psa_set_key_type(&key_attributes, SAMPLE_KEY_TYPE);
	psa_set_key_bits(&key_attributes, 128);

	/* Persistent key specific settings */
	psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_PERSISTENT);
	psa_set_key_id(&key_attributes, SAMPLE_PERS_KEY_ID);

	/* Generate a random AES key with persistent lifetime. The key can be used for
	 * encryption/decryption using the key_id.
	 */
	status = psa_generate_key(&key_attributes, &key_id);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_generate_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Make sure the key is not in memory anymore, has the same affect then resetting the device
	 */
	status = psa_purge_key(key_id);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_purge_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* After the key handle is acquired the attributes are not needed */
	psa_reset_key_attributes(&key_attributes);

	LOG_INF("Persistent key generated successfully!");

	return APP_SUCCESS;
}

int use_persistent_key(void)
{
	uint32_t olen;
	psa_status_t status;

	status = psa_cipher_encrypt((psa_key_id_t)SAMPLE_PERS_KEY_ID, SAMPLE_ALG, m_plain_text,
				    sizeof(m_plain_text), m_encrypted_text,
				    sizeof(m_encrypted_text), &olen);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_cipher_encrypt failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("Encryption successful!");
	PRINT_HEX("Plaintext", m_plain_text, sizeof(m_plain_text));
	PRINT_HEX("Encrypted text", m_encrypted_text, sizeof(m_encrypted_text));

	status = psa_cipher_decrypt((psa_key_id_t)SAMPLE_PERS_KEY_ID, SAMPLE_ALG, m_encrypted_text,
				    sizeof(m_encrypted_text), m_decrypted_text,
				    sizeof(m_decrypted_text), &olen);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_cipher_decrypt failed! (Error: %d)", status);
		return APP_ERROR;
	}

	PRINT_HEX("Decrypted text", m_decrypted_text, sizeof(m_decrypted_text));

	/* Check the validity of the decryption */
	if (memcmp(m_decrypted_text, m_plain_text,
		   NRF_CRYPTO_EXAMPLE_PERSISTENT_KEY_MAX_TEXT_SIZE) != 0) {
		LOG_INF("Error: Decrypted text doesn't match the plaintext");
		return APP_ERROR;
	}

	LOG_INF("Decryption successful!");

	return APP_SUCCESS;
}

int main(void)
{
	int status;

	LOG_INF("Starting persistent key example...");

	status = crypto_init();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

#if FIXED_PARTITION_EXISTS(storage_partition)
	status = migrate_simulated_factory_secret();
#else
	status = APP_ERROR;
#endif
	if (status != APP_SUCCESS) {
		status = generate_persistent_key();
		if (status != APP_SUCCESS) {
			LOG_INF(APP_ERROR_MESSAGE);
			return APP_ERROR;
		}
	}

	status = use_persistent_key();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = crypto_finish();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	LOG_INF(APP_SUCCESS_MESSAGE);

	return APP_SUCCESS;
}
