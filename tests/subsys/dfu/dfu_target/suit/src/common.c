/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "common.h"

/* Mocks - ssf services have to be mocked, as real communication with secure domain
 * cannot take place in these tests because native_posix target does not have the secure domain
 * to communicate with, and nRF54H should not reset the device after calling
 * the dfu_target_suit_schedule_update function.
 */
DEFINE_FFF_GLOBALS;

/* Mocks of functions provided via SSF services */
#ifdef CONFIG_SOC_POSIX
FAKE_VALUE_FUNC(int, suit_plat_component_compatibility_check, suit_manifest_class_id_t *,
		struct zcbor_string *);
FAKE_VALUE_FUNC(int, suit_plat_authorize_sequence_num, enum suit_command_sequence,
		struct zcbor_string *, unsigned int);
FAKE_VALUE_FUNC(int, suit_plat_authorize_unsigned_manifest, struct zcbor_string *);
FAKE_VALUE_FUNC(int, suit_plat_authenticate_manifest, struct zcbor_string *, enum suit_cose_alg,
		struct zcbor_string *, struct zcbor_string *, struct zcbor_string *);

FAKE_VALUE_FUNC(suit_ssf_err_t, suit_check_installed_component_digest, suit_plat_mreg_t *, int,
		suit_plat_mreg_t *);
FAKE_VALUE_FUNC(suit_ssf_err_t, suit_get_supported_manifest_roles, suit_manifest_role_t *,
		size_t *);
FAKE_VALUE_FUNC(suit_ssf_err_t, suit_get_supported_manifest_info, suit_manifest_role_t,
		suit_ssf_manifest_class_info_t *);
FAKE_VALUE_FUNC(suit_ssf_err_t, suit_get_installed_manifest_info, suit_manifest_class_id_t *,
		unsigned int *, suit_semver_raw_t *, suit_digest_status_t *, int *,
		suit_plat_mreg_t *);
#endif

DEFINE_FAKE_VALUE_FUNC(int, suit_trigger_update, suit_plat_mreg_t *, size_t);

void reset_fakes(void)
{
	#ifdef CONFIG_SOC_POSIX
		RESET_FAKE(suit_plat_component_compatibility_check);
		RESET_FAKE(suit_plat_authorize_sequence_num);
		RESET_FAKE(suit_plat_authorize_unsigned_manifest);
		RESET_FAKE(suit_plat_authenticate_manifest);
		RESET_FAKE(suit_check_installed_component_digest);
		RESET_FAKE(suit_get_supported_manifest_roles);
		RESET_FAKE(suit_get_supported_manifest_info);
		RESET_FAKE(suit_get_installed_manifest_info);
	#endif /* CONFIG_SOC_POSIX */
	
	RESET_FAKE(suit_trigger_update);
}