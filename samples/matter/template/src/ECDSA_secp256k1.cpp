/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <psa/crypto.h>

#include "ECDSA_secp256k1.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace ECDSA_Test
{

psa_status_t K256Keypair::Initialize()
{
	if (mInitialized) {
		return PSA_ERROR_BAD_STATE;
	}

	psa_status_t status = PSA_SUCCESS;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
	size_t publicKeyLength = 0;

	// Type based on ECC with the elliptic curve SECP256k1
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_K1));
	psa_set_key_bits(&attributes, kK256_PrivateKeyLength * 8);
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
#ifdef CONFIG_ECDSA_SECP256K1_TEST_ALLOW_KEY_EXPORT
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_MESSAGE);
#else
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
#endif
	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);

	status = psa_generate_key(&attributes, &mSecretKeyId);
	if (PSA_SUCCESS != status) {
		psa_destroy_key(mSecretKeyId);
		psa_reset_key_attributes(&attributes);
		return status;
	}

	status = psa_export_public_key(mSecretKeyId, mPublicKey, mPublicKeySize, &publicKeyLength);

	if (PSA_SUCCESS != status) {
		psa_destroy_key(mSecretKeyId);
		psa_reset_key_attributes(&attributes);
		return status;
	}

	if (publicKeyLength != mPublicKeySize) {
		return PSA_ERROR_DATA_INVALID;
	}

	mInitialized = true;

	return PSA_SUCCESS;
}

psa_status_t K256Keypair::Import(const uint8_t *privKey, size_t privKeyLen, const uint8_t *pubKey, size_t pubKeyLen)
{
	if (!privKey || privKeyLen == 0 || !pubKey || pubKeyLen == 0) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	psa_status_t status = PSA_SUCCESS;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	if (mInitialized) {
		psa_destroy_key(mSecretKeyId);
		psa_reset_key_attributes(&attributes);
	}

	// Type based on ECC with the elliptic curve SECP256k1
	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_K1));
	psa_set_key_bits(&attributes, kK256_PrivateKeyLength * 8);
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
#ifdef CONFIG_ECDSA_SECP256K1_TEST_ALLOW_KEY_EXPORT
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_MESSAGE);
#else
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
#endif
	psa_set_key_lifetime(&attributes, PSA_KEY_LIFETIME_VOLATILE);

	status = psa_import_key(&attributes, privKey, privKeyLen, &mSecretKeyId);
	if (PSA_SUCCESS != status) {
		psa_destroy_key(mSecretKeyId);
		psa_reset_key_attributes(&attributes);
		return status;
	}

	if (mPublicKeySize != pubKeyLen) {
		return PSA_ERROR_DATA_INVALID;
	}

	memcpy(mPublicKey, pubKey, mPublicKeySize);

	mInitialized = true;

	return PSA_SUCCESS;
}

psa_status_t K256Keypair::Export(uint8_t *privKey, size_t &privKeyLen)
{
	if (!mInitialized || !privKey || privKeyLen == 0) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}
	psa_status_t status = PSA_SUCCESS;
	size_t privateKeyLength = 0;

	status = psa_export_key(mSecretKeyId, privKey, privKeyLen, &privateKeyLength);
	if (PSA_SUCCESS != status) {
		return status;
	}

	privKeyLen = privateKeyLength;

	return PSA_SUCCESS;
}

psa_status_t K256Keypair::ECDSA_sign_msg(const uint8_t *msg, const size_t msgLength, K256ECDSASignature &outSignature)
{
	if (!mInitialized || !msg || msgLength == 0) {
		return false;
	}

	psa_status_t status = PSA_SUCCESS;
	size_t outputLen = 0;

	status = psa_sign_message(mSecretKeyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msgLength, outSignature.data,
				  outSignature.size, &outputLen);
	if (status != PSA_SUCCESS || outputLen != kMax_ECDSA_Signature_Length) {
		return status;
	}

	outSignature.size = outputLen;

	return PSA_SUCCESS;
}

psa_status_t K256Keypair::ECDSA_validate_msg_signature(const uint8_t *msg, const size_t msgLength,
					       const K256ECDSASignature &signature, K256PublicKey &pubKey)
{
	if (!msg || msgLength == 0) {
		return PSA_ERROR_INVALID_ARGUMENT;
	}

	psa_status_t status = PSA_SUCCESS;
	psa_key_id_t keyId = 0;
	psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

	psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_K1));
	psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
	psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_MESSAGE);

	status = psa_import_key(&attributes, pubKey, sizeof(pubKey), &keyId);

	if (status == PSA_SUCCESS) {
		status = psa_verify_message(keyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msgLength, signature.data,
					    signature.size);
	}

	psa_destroy_key(keyId);
	psa_reset_key_attributes(&attributes);
	return status;
}

} // namespace ECDSA_Test
