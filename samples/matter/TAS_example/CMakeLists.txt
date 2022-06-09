#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

get_filename_component(MATTER_MODULE_ROOT $ENV{ZEPHYR_BASE}/../modules/lib/matter REALPATH)
set(mcuboot_KCONFIG_ROOT ${MATTER_MODULE_ROOT}/config/nrfconnect/chip-module/Kconfig.mcuboot.root)

set(CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/prj.conf)
set(PM_STATIC_YML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configuration/${BOARD}/pm_static_dfu.yml)



find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

target_sources(app PRIVATE src/main.c)
