/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_task.h"

#include "app/matter_init.h"
#include "app/task_executor.h"
#include "board/board.h"

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include "dfu/ota/ota_util.h"
#endif

/*----------------------------------------------------------
ECDSA signing with secp256k1 TEST
-----------------------------------------------------------*/
#ifdef CONFIG_ECDSA_SECP256K1_TEST
#include "ECDSA_secp256k1.h"
#endif
/*----------------------------------------------------------
END
-----------------------------------------------------------*/

#include <app/server/OnboardingCodesUtil.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;

CHIP_ERROR AppTask::Init()
{
	/* Initialize Matter stack */
	ReturnErrorOnFailure(Nrf::Matter::PrepareServer());

	if (!Nrf::GetBoard().Init()) {
		LOG_ERR("User interface initialization failed.");
		return CHIP_ERROR_INCORRECT_STATE;
	}

	/* Register Matter event handler that controls the connectivity status LED based on the captured Matter network
	 * state. */
	ReturnErrorOnFailure(Nrf::Matter::RegisterEventHandler(Nrf::Board::DefaultMatterEventHandler, 0));

	return Nrf::Matter::StartServer();
}

CHIP_ERROR AppTask::StartApp()
{
	ReturnErrorOnFailure(Init());

	/*----------------------------------------------------------
	ECDSA signing with secp256k1 TEST
	-----------------------------------------------------------*/
#ifdef CONFIG_ECDSA_SECP256K1_TEST

	psa_status_t status = PSA_SUCCESS;

	{
		LOG_ERR("\n\nECDSA SECP256K1 TEST\n\n");
		ECDSA_Test::K256Keypair keyPair;

		LOG_INF("Initializing Keypair...");
		status = keyPair.Initialize();
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("Signing a message...");
		ECDSA_Test::K256ECDSASignature signature;
		status = keyPair.ECDSA_sign_msg(ECDSA_Test::kTestMessage, sizeof(ECDSA_Test::kTestMessage), signature);
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("Validating message signature...");
		ECDSA_Test::K256PublicKey testPubKey;
		memcpy(testPubKey, keyPair.GetPubKey(), keyPair.GetPubKeyLen());
		status = keyPair.ECDSA_validate_msg_signature(ECDSA_Test::kTestMessage,
							      sizeof(ECDSA_Test::kTestMessage), signature, testPubKey);
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}
	}

	{
		LOG_INF("Testing with known keys generated externally using OpenSSL\n");

		ECDSA_Test::K256Keypair keyPair;

		LOG_INF("Importing keypair...");
		status = keyPair.Import(ECDSA_Test::kTestPrivKey, sizeof(ECDSA_Test::kTestPrivKey),
					ECDSA_Test::kTestPubKey, sizeof(ECDSA_Test::kTestPubKey));
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("Comparing Public Key...");
		if (memcmp(ECDSA_Test::kTestPubKey, keyPair.GetPubKey(), keyPair.GetPubKeyLen()) != 0) {
			LOG_ERR("Failed\n");
		} else {
			LOG_INF("Success!\n");
		}

		uint8_t exportedPrivKey[ECDSA_Test::kK256_PrivateKeyLength];
		size_t exportedSizeLen = sizeof(exportedPrivKey);
		LOG_INF("Exporting Private Key...");
#ifdef CONFIG_ECDSA_SECP256K1_TEST_ALLOW_KEY_EXPORT
		status = keyPair.Export(exportedPrivKey, exportedSizeLen);
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("Comparing Private Keys...");
		if (memcmp(exportedPrivKey, ECDSA_Test::kTestPrivKey, ECDSA_Test::kK256_PrivateKeyLength) != 0) {
			LOG_ERR("Failed\n");
		} else {
			LOG_INF("Success!\n");
		}
#else
		status = keyPair.Export(exportedPrivKey, exportedSizeLen);
		if (PSA_SUCCESS == status) {
			LOG_ERR("ECDSA SECP256K1 Export keypair should fail but got success");
		} else {
			LOG_INF("Success!\n");
		}
#endif

		LOG_INF("Signing a message...");
		ECDSA_Test::K256ECDSASignature signature = {};
		auto signingTimeStart = System::SystemClock().GetMonotonicTimestamp();
		status = keyPair.ECDSA_sign_msg(ECDSA_Test::kTestMessage, sizeof(ECDSA_Test::kTestMessage), signature);
		auto signingTimeEnd = System::SystemClock().GetMonotonicTimestamp();
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("Validating message signature...");
		ECDSA_Test::K256PublicKey pubKey;
		memcpy(pubKey, ECDSA_Test::kTestPubKey, sizeof(pubKey));
		auto validationTimeStart = System::SystemClock().GetMonotonicTimestamp();
		status = keyPair.ECDSA_validate_msg_signature(ECDSA_Test::kTestMessage,
							      sizeof(ECDSA_Test::kTestMessage), signature, pubKey);
		auto validationTimeEnd = System::SystemClock().GetMonotonicTimestamp();
		if (PSA_SUCCESS != status) {
			LOG_ERR("Failed %d\n", status);
		} else {
			LOG_INF("Success!\n");
		}

		LOG_INF("ECDSA SECP256K1 signing time: %llu ms",
			validationTimeEnd.count() - validationTimeStart.count());
		LOG_INF("ECDSA SECP256K1 validating time: %llu ms", signingTimeEnd.count() - signingTimeStart.count());
	}

	LOG_INF("\n\nECDSA SECP256K1 TEST FINISHED\n\n");
#endif
	/*----------------------------------------------------------
	END
	-----------------------------------------------------------*/

	while (true) {
		Nrf::DispatchNextTask();
	}

	return CHIP_NO_ERROR;
}
