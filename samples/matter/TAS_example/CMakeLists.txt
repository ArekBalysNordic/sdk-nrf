#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/prj.conf)
set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)

get_filename_component(MATTER_MODULE_ROOT $ENV{ZEPHYR_BASE}/../modules/lib/matter REALPATH)
set(mcuboot_KCONFIG_ROOT ${MATTER_MODULE_ROOT}/config/nrfconnect/chip-module/Kconfig.mcuboot.root)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_sources(app PRIVATE src/main.c)

set(CHIP_ROOT ${MATTER_MODULE_ROOT})
include(${MATTER_MODULE_ROOT}/config/zephyr/ota-image.cmake)
chip_ota_image(chip-ota-image
    INPUT_FILES ${PROJECT_BINARY_DIR}/zephyr/dfu_multi_image.bin
    OUTPUT_FILE ${PROJECT_BINARY_DIR}/zephyr/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
)

add_dependencies(chip-ota-image dfu_multi_image_pkg)