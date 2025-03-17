/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <psa/crypto.h>
#include <psa/crypto_extra.h>

#include <cracen_psa_kmu.h>

#ifdef CONFIG_BUILD_WITH_TFM
#include <tfm_ns_interface.h>
#endif

#define APP_SUCCESS             (0)
#define APP_ERROR               (-1)
#define APP_SUCCESS_MESSAGE "Example finished successfully!"
#define APP_ERROR_MESSAGE "Example exited with error!"

#define PRINT_HEX(p_label, p_text, len)				  \
	({							  \
		LOG_INF("---- %s (len: %u): ----", p_label, len); \
		LOG_HEXDUMP_INF(p_text, len, "Content:");	  \
		LOG_INF("---- %s end  ----", p_label);		  \
	})

LOG_MODULE_REGISTER(aes_gcm, LOG_LEVEL_DBG);

/* ====================================================================== */
/*			Global variables/defines for the AES GCM mode example		  */

#define NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE (100)
#define NRF_CRYPTO_EXAMPLE_AES_BLOCK_SIZE (16)
#define NRF_CRYPTO_EXAMPLE_AES_IV_SIZE (12)
#define NRF_CRYPTO_EXAMPLE_AES_ADDITIONAL_SIZE (35)
#define NRF_CRYPTO_EXAMPLE_AES_GCM_TAG_LENGTH (16)

/* AES sample IV, DO NOT USE IN PRODUCTION */
static uint8_t m_iv[NRF_CRYPTO_EXAMPLE_AES_IV_SIZE];

/* Below text is used as plaintext for encryption/decryption */
static uint8_t m_plain_text[NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE] = {
	"Example string to demonstrate basic usage of AES GCM mode."
};

/* Below text is used as additional data for authentication */
static uint8_t m_additional_data[NRF_CRYPTO_EXAMPLE_AES_ADDITIONAL_SIZE] = {
	"Example string of additional data"
};

static uint8_t m_encrypted_text[NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE +
				NRF_CRYPTO_EXAMPLE_AES_GCM_TAG_LENGTH];

static uint8_t m_decrypted_text[NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE];

uint8_t input_key_buffer[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static psa_key_id_t key_id =
	PSA_KEY_HANDLE_FROM_CRACEN_KMU_SLOT(CRACEN_KMU_KEY_USAGE_SCHEME_RAW, 80);
/* ====================================================================== */

int crypto_init(void)
{
	psa_status_t status;

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

int generate_key(void)
{
	psa_status_t status;

	LOG_INF("Generating random AES key...");

	psa_destroy_key(key_id);

	/* Configure the key attributes */
	psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT | PSA_KEY_USAGE_EXPORT);
	// psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_lifetime(&key_attributes,
				PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
					PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_CRACEN_KMU));
	psa_set_key_algorithm(&key_attributes, PSA_ALG_GCM);
	psa_set_key_type(&key_attributes, PSA_KEY_TYPE_AES);
	psa_set_key_bits(&key_attributes, 128);
	psa_set_key_id(&key_attributes, key_id);

	/* Generate a random key. The key is not exposed to the application,
	 * we can use it to encrypt/decrypt using the key handle
	 */
	status = psa_import_key(&key_attributes, input_key_buffer, sizeof(input_key_buffer), &key_id);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_generate_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_HEXDUMP_INF(input_key_buffer, sizeof(input_key_buffer), "imported key");

	/* After the key handle is acquired the attributes are not needed */
	psa_reset_key_attributes(&key_attributes);

	LOG_INF("AES key generated successfully!");

	return 0;
}

int encrypt_aes_gcm(void)
{
	uint32_t output_len;
	psa_status_t status;

	LOG_INF("Encrypting using AES GCM MODE...");

	/* Generate a random IV */
	status = psa_generate_random(m_iv, NRF_CRYPTO_EXAMPLE_AES_IV_SIZE);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_generate_random failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Encrypt the plaintext and create the authentication tag */
	status = psa_aead_encrypt(key_id,
				  PSA_ALG_GCM,
				  m_iv,
				  sizeof(m_iv),
				  m_additional_data,
				  sizeof(m_additional_data),
				  m_plain_text,
				  sizeof(m_plain_text),
				  m_encrypted_text,
				  sizeof(m_encrypted_text),
				  &output_len);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_aead_encrypt failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("Encryption successful!");
	PRINT_HEX("IV", m_iv, sizeof(m_iv));
	PRINT_HEX("Additional data", m_additional_data, sizeof(m_additional_data));
	PRINT_HEX("Plaintext", m_plain_text, sizeof(m_plain_text));
	PRINT_HEX("Encrypted text", m_encrypted_text, sizeof(m_encrypted_text));

	return APP_SUCCESS;
}

int decrypt_aes_gcm(void)
{
	uint32_t output_len;
	psa_status_t status;

	LOG_INF("Decrypting using AES GCM MODE...");

	/* Decrypt and authenticate the encrypted data */
	status = psa_aead_decrypt(key_id,
				  PSA_ALG_GCM,
				  m_iv,
				  sizeof(m_iv),
				  m_additional_data,
				  sizeof(m_additional_data),
				  m_encrypted_text,
				  sizeof(m_encrypted_text),
				  m_decrypted_text,
				  sizeof(m_decrypted_text),
				  &output_len);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_aead_decrypt failed! (Error: %d)", status);
		return APP_ERROR;
	}

	PRINT_HEX("Decrypted text", m_decrypted_text, sizeof(m_decrypted_text));

	/* Check the validity of the decryption */
	if (memcmp(m_decrypted_text, m_plain_text, NRF_CRYPTO_EXAMPLE_AES_MAX_TEXT_SIZE) != 0) {
		LOG_INF("Error: Decrypted text doesn't match the plaintext");
		return APP_ERROR;
	}

	LOG_INF("Decryption and authentication successful!");

	return APP_SUCCESS;
}

int aes_gcm_export(void)
{
	uint8_t buffer[16];
	size_t bufferLength = 16;
	size_t keyLength = 16;
	psa_status_t status;

	status = psa_export_key(key_id, buffer, bufferLength, &keyLength);
	LOG_HEXDUMP_INF(buffer, keyLength, "exported key");
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_export_key failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("AES key exported successfully!");

	return APP_SUCCESS;
}

int main(void)
{
	int status;

	LOG_INF("Starting AES-GCM example...");

	status = crypto_init();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = generate_key();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = encrypt_aes_gcm();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = decrypt_aes_gcm();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = aes_gcm_export();
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
