#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# CMake file for HDR PHY isolated test CLI vendor extension.
# Used when CONFIG_OPENTHREAD_RADIO_2P4GHZ_HDR_CLI_TEST is enabled.
#

target_compile_definitions(ot-config INTERFACE 
  "OPENTHREAD_CONFIG_CLI_MAX_USER_CMD_ENTRIES=2"
  "OPENTHREAD_CONFIG_CLI_VENDOR_COMMANDS_ENABLE=1"
)

add_library(cli-hdr-test STATIC
  ${CMAKE_CURRENT_LIST_DIR}/cli_hdr_test.c
)

target_link_libraries(cli-hdr-test PRIVATE ot-config)

target_include_directories(cli-hdr-test PUBLIC
  ${OT_PUBLIC_INCLUDES}
  ${CMAKE_CURRENT_LIST_DIR}
)

set(OT_CLI_VENDOR_TARGET cli-hdr-test)
