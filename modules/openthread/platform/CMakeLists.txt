# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(openthread_platform)

# Use crypto_psa implementation from sdk-nrf
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_CRYPTO_PSA crypto_psa.c)

# Use nrf radio implementation depending on CONFIG_NET_L2_OPENTHREAD
zephyr_library_sources_ifndef(CONFIG_NET_L2_OPENTHREAD radio_nrf5.c)
zephyr_library_sources_ifdef(CONFIG_NET_L2_OPENTHREAD ${ZEPHYR_BASE}/modules/openthread/platform/radio.c)

# All other sources are from Zephyr
zephyr_library_sources(
  ${ZEPHYR_BASE}/modules/openthread/platform/alarm.c
  ${ZEPHYR_BASE}/modules/openthread/platform/entropy.c
  ${ZEPHYR_BASE}/modules/openthread/platform/misc.c
  ${ZEPHYR_BASE}/modules/openthread/platform/platform.c
  ${ZEPHYR_BASE}/modules/openthread/platform/spi.c
  )

zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_BLE_TCAT ${ZEPHYR_BASE}/modules/openthread/platform/ble.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_DIAG ${ZEPHYR_BASE}/modules/openthread/platform/diag.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_COPROCESSOR ${ZEPHYR_BASE}/modules/openthread/platform/uart.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_EXTERNAL_HEAP ${ZEPHYR_BASE}/modules/openthread/platform/memory.c)
zephyr_library_sources_ifdef(CONFIG_OPENTHREAD_PLATFORM_MESSAGE_MANAGEMENT ${ZEPHYR_BASE}/modules/openthread/platform/messagepool.c)
zephyr_library_sources_ifdef(CONFIG_SETTINGS ${ZEPHYR_BASE}/modules/openthread/platform/settings.c)
zephyr_library_sources_ifndef(CONFIG_LOG_BACKEND_SPINEL ${ZEPHYR_BASE}/modules/openthread/platform/logging.c)

zephyr_include_directories(. ${ZEPHYR_BASE}/modules/openthread/platform)
