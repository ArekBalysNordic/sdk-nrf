#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

include(${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/src/app/chip_data_model.cmake)

function(ncs_configure_data_model)
  string(CONFIGURE "${CONFIG_NCS_SAMPLE_MATTER_ZAP_FILE_PATH}" zap_file_path)
  cmake_path(GET zap_file_path PARENT_PATH zap_parent_dir)

  cmake_parse_arguments(ARG "" "" "EXTERNAL_CLUSTERS" ${ARGN})

  target_include_directories(matter-data-model
    PUBLIC
    ${zap_parent_dir}
  )

  if(CONFIG_NCS_SAMPLE_MATTER_DOWNLOAD_ZAP_CLI)
    # We require zap tool for this to work, so request downloading it if not present.
    find_program(ZAP_CLI_EXECUTABLE zap-cli)
    if(NOT ZAP_CLI_EXECUTABLE)
      execute_process(COMMAND -c "west zap-install")
      execute_process(
        COMMAND west zap-install --get_path
        OUTPUT_VARIABLE ZAP_CLI_EXECUTABLE
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      message(STATUS "zap-cli installed in: ${ZAP_CLI_EXECUTABLE}")
    else ()
      message(STATUS "zap-cli found at: ${ZAP_CLI_EXECUTABLE}")
    endif()
  endif()

  if(CONFIG_NCS_SAMPLE_MATTER_ZAP_GENERATION_STATIC)
    chip_configure_data_model(matter-data-model
      BYPASS_IDL
      GEN_DIR ${zap_parent_dir}/zap-generated
      ZAP_FILE ${zap_file_path}
      EXTERNAL_CLUSTERS ${ARG_EXTERNAL_CLUSTERS}
    )
  elseif(CONFIG_NCS_SAMPLE_MATTER_ZAP_GENERATION_BUILD_TIME)
    chip_configure_data_model(app
      ZAP_FILE ${zap_file_path}
      EXTERNAL_CLUSTERS ${ARG_EXTERNAL_CLUSTERS}
    )
  endif()

endfunction()
