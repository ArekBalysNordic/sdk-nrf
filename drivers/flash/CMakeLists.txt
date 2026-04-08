#
# Copyright (c) 2024 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

add_subdirectory_ifdef(CONFIG_FLASH_RPC flash_rpc)

if(CONFIG_SOC_FLASH_NRF_NON_SECURE)
  zephyr_library_amend()
  zephyr_library_sources(soc_flash_nrf_non_secure.c)
  zephyr_library_include_directories(${ZEPHYR_BASE}/drivers/flash)
endif()
