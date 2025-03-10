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

LOG_MODULE_REGISTER(hmac, LOG_LEVEL_DBG);

/* ====================================================================== */
/*				Global variables/defines for the HMAC example			  */

#define NRF_CRYPTO_EXAMPLE_HMAC_TEXT_SIZE (100)
#define NRF_CRYPTO_EXAMPLE_HMAC_KEY_SIZE  (32)

/* Below text is used as plaintext for signing/verifiction */
static uint8_t m_plain_text[NRF_CRYPTO_EXAMPLE_HMAC_TEXT_SIZE] = {
	"Example string to demonstrate basic usage of HMAC signing/verification."};

static uint8_t hmac[NRF_CRYPTO_EXAMPLE_HMAC_KEY_SIZE];

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

uint8_t input_key_buffer[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};


int generate_key(void)
{
	psa_status_t status;

	LOG_INF("Generating random HMAC key...");

	psa_destroy_key(key_id);

	/* Configure the key attributes */
	psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_usage_flags(&key_attributes,
				PSA_KEY_USAGE_VERIFY_HASH | PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_EXPORT);
	// psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_VOLATILE);
	psa_set_key_lifetime(&key_attributes,
			     PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
				     PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_CRACEN_KMU));
	psa_set_key_algorithm(&key_attributes, PSA_ALG_HMAC(PSA_ALG_SHA_256));
	psa_set_key_type(&key_attributes, PSA_KEY_TYPE_HMAC);
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

	LOG_HEXDUMP_INF(input_key_buffer, sizeof(input_key_buffer), "generated key");

	/* After the key handle is acquired the attributes are not needed */
	psa_reset_key_attributes(&key_attributes);

	LOG_INF("HMAC key generated successfully!");

	return APP_SUCCESS;
}

int hmac_sign(void)
{
	uint32_t olen;
	psa_status_t status;
	psa_mac_operation_t operation = PSA_MAC_OPERATION_INIT;

	LOG_INF("Signing using HMAC ...");

	/* Initialize the HMAC signing operation */
	status = psa_mac_sign_setup(&operation, key_id, PSA_ALG_HMAC(PSA_ALG_SHA_256));
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_sign_setup failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Perform the HMAC signing */
	status = psa_mac_update(&operation, m_plain_text, sizeof(m_plain_text));
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_update failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Finalize the HMAC signing */
	status = psa_mac_sign_finish(&operation, hmac, sizeof(hmac), &olen);
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_sign_finish failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("Signing successful!");
	PRINT_HEX("Plaintext", m_plain_text, sizeof(m_plain_text));
	PRINT_HEX("HMAC", hmac, sizeof(hmac));

	return APP_SUCCESS;
}

int hmac_verify(void)
{
	psa_status_t status;
	psa_mac_operation_t operation = PSA_MAC_OPERATION_INIT;

	LOG_INF("Verifying the HMAC signature...");

	/* Initialize the HMAC verification operation */
	status = psa_mac_verify_setup(&operation, key_id, PSA_ALG_HMAC(PSA_ALG_SHA_256));
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_verify_setup failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Perform the HMAC verification */
	status = psa_mac_update(&operation, m_plain_text, sizeof(m_plain_text));
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_update failed! (Error: %d)", status);
		return APP_ERROR;
	}

	/* Finalize the HMAC verification */
	status = psa_mac_verify_finish(&operation, hmac, sizeof(hmac));
	if (status != PSA_SUCCESS) {
		LOG_INF("psa_mac_verify_finish failed! (Error: %d)", status);
		return APP_ERROR;
	}

	LOG_INF("HMAC verified successfully!");

	return APP_SUCCESS;
}

int hmac_export(){

	uint8_t buffer[16];
	size_t bufferLength = 16;
	size_t keyLength = 16;


	psa_status_t error = psa_export_key(key_id, buffer, bufferLength, &keyLength);
	if(error != PSA_SUCCESS){
		LOG_INF("psa_export_key failed! (Error: %d)", error);
		return APP_ERROR;
	}

	LOG_HEXDUMP_INF(buffer, keyLength, "exported key");

	if(memcmp(buffer, input_key_buffer, keyLength) != 0){
		LOG_INF("Exported key does not match generated key");
		return APP_ERROR;
	}

	return APP_SUCCESS;
}

int main(void)
{
	int status;

	LOG_INF("Starting HMAC example...");

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

	status = hmac_sign();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = hmac_verify();
	if (status != APP_SUCCESS) {
		LOG_INF(APP_ERROR_MESSAGE);
		return APP_ERROR;
	}

	status = hmac_export();
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
