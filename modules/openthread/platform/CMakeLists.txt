# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(openthread_platform)

# Source nrf-specific files from sdk-nrf
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_CRYPTO_PSA crypto_psa.c)

# Source platform files from Zephyr
zephyr_library_sources(
  ${ZEPHYR_BASE}/modules/openthread/platform/alarm.c
  ${ZEPHYR_BASE}/modules/openthread/platform/entropy.c
  ${ZEPHYR_BASE}/modules/openthread/platform/misc.c
  ${ZEPHYR_BASE}/modules/openthread/platform/platform.c
  )

if(NOT CONFIG_HDLC_RCP_IF)
  # Radio platform implementation dedicated for L2 Platform adaptation layer
  zephyr_library_sources_ifdef(CONFIG_NET_L2_OPENTHREAD ${ZEPHYR_BASE}/modules/openthread/platform/radio.c)

  # Radio platform implementation dedicated for nrf-sdk
  zephyr_library_sources_ifndef(CONFIG_NET_L2_OPENTHREAD radio_nrf5.c)

  zephyr_library_sources(${ZEPHYR_BASE}/modules/openthread/platform/spi.c)
endif()

zephyr_library_sources_ifdef(CONFIG_HDLC_RCP_IF
  ${ZEPHYR_BASE}/modules/openthread/platform/radio_spinel.cpp
  ${ZEPHYR_BASE}/modules/openthread/platform/hdlc_interface.cpp
)

zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_BLE_TCAT ${ZEPHYR_BASE}/modules/openthread/platform/ble.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_DIAG ${ZEPHYR_BASE}/modules/openthread/platform/diag.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_COPROCESSOR ${ZEPHYR_BASE}/modules/openthread/platform/uart.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_EXTERNAL_HEAP ${ZEPHYR_BASE}/modules/openthread/platform/memory.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_PLATFORM_MESSAGE_MANAGEMENT ${ZEPHYR_BASE}/modules/openthread/platform/messagepool.c)
zephyr_library_sources_ifdef(CONFIG_SETTINGS ${ZEPHYR_BASE}/modules/openthread/platform/settings.c)
zephyr_library_sources_ifndef(CONFIG_LOG_BACKEND_SPINEL ${ZEPHYR_BASE}/modules/openthread/platform/logging.c)

if(CONFIG_OPENTHREAD_RADIO_2P4GHZ_HDR_CLI_TEST)
  target_compile_definitions(ot-config INTERFACE 
    "OPENTHREAD_CONFIG_CLI_MAX_USER_CMD_ENTRIES=2"
    "OPENTHREAD_CONFIG_CLI_VENDOR_COMMANDS_ENABLE=1"
  )
endif()

if(CONFIG_OPENTHREAD_RADIO_2P4GHZ_HDR_ASSUME_NEIGHBOR_SUPPORT)
  target_compile_definitions(ot-config INTERFACE 
    "OPENTHREAD_CONFIG_RADIO_2P4GHZ_HDR_ASSUME_NEIGHBOR_SUPPORT=1"
  )
endif()

zephyr_include_directories(. ${ZEPHYR_BASE}/modules/openthread/platform)
