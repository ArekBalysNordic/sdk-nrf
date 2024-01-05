/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <psa/crypto.h>

namespace ECDSA_Test
{
constexpr uint8_t kTestMessage[] = { 'E', 'C', 'D', 'S', 'A', '_', 's', 'e', 'c', 'p',
				     '2', '5', '6', 'k', '1', '_', 'T', 'E', 'S', 'T' };
constexpr uint8_t kTestPrivKey[] = { 0x9d, 0xe4, 0x9b, 0xe4, 0x35, 0x8c, 0x04, 0x99, 0x01, 0x34, 0xb7,
				     0xe8, 0xa0, 0xf0, 0x18, 0x35, 0x37, 0x7d, 0xd2, 0x77, 0xb1, 0x5a,
				     0xb1, 0x9b, 0xbc, 0xf7, 0xeb, 0x7b, 0xe7, 0xde, 0x5e, 0x18 };
constexpr uint8_t kTestPubKey[] = { 0x04, 0x20, 0xe6, 0x6d, 0x1b, 0xbc, 0x5e, 0x14, 0xb5, 0xe9, 0x4d, 0xf5, 0xf7,
				    0x58, 0x25, 0xfb, 0x44, 0x60, 0xc6, 0x5b, 0x8a, 0x9b, 0xcc, 0xae, 0x6a, 0x02,
				    0xce, 0xdf, 0xf6, 0xf9, 0xcc, 0x08, 0xf3, 0xda, 0x39, 0x3f, 0xb5, 0x8f, 0x09,
				    0x11, 0x0c, 0x7e, 0xcb, 0xcb, 0xd5, 0x95, 0x24, 0x51, 0x01, 0xa7, 0x92, 0x1d,
				    0x01, 0x0f, 0xdf, 0xa4, 0xe1, 0xe8, 0x68, 0x21, 0x76, 0xf5, 0xfe, 0xaa, 0x27 };

constexpr auto kK256_PublicKeyLength = 2 * 32 + 1;
constexpr auto kK256_PrivateKeyLength = 32;
constexpr auto kMax_ECDSA_Signature_Length = 2 * 32;

struct K256ECDSASignature {
	uint8_t data[kMax_ECDSA_Signature_Length];
	size_t size = kMax_ECDSA_Signature_Length;
};

using K256PublicKey = uint8_t[kK256_PublicKeyLength];

struct k256SerializedKeyPair;

class K256Keypair {
public:
	K256Keypair() {}

	psa_status_t Initialize();
	psa_status_t Import(const uint8_t *privKey, size_t privKeyLen, const uint8_t *pubKey, size_t pubKeyLen);
	psa_status_t Export(uint8_t *privKey, size_t &privKeyLen);

	psa_status_t ECDSA_sign_msg(const uint8_t *msg, const size_t msgLength, K256ECDSASignature &outSignature);
	psa_status_t ECDSA_validate_msg_signature(const uint8_t *msg, const size_t msg_length,
						  const K256ECDSASignature &signature, K256PublicKey &pubKey);

	uint8_t *GetPubKey() { return mPublicKey; }
	size_t GetPubKeyLen() { return mPublicKeySize; }

private:
	bool mInitialized = false;
	psa_key_id_t mSecretKeyId = 0;
	K256PublicKey mPublicKey;
	size_t mPublicKeySize = sizeof(K256PublicKey);
};

} // namespace ECDSA_Test
