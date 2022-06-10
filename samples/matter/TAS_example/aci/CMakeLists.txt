#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# This check is needed to avoid infinite recursion. This module code will
# be executed for all images in the build, also for the child image being
# added below.
if (CONFIG_INCLUDE_NET_CORE_IMAGE)
set(CHILD_IMAGE_NAME CPUNET)

  add_overlay_config(
    ${CHILD_IMAGE_NAME}
    "${NRF_DIR}/subsys/bootloader/image/secure_boot.conf"
  )

  add_overlay_config(
    mcuboot
    "${NRF_DIR}/subsys/pcd/pcd.conf"
  )
  add_child_image(
    NAME ${CHILD_IMAGE_NAME}
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../cpunet
    DOMAIN ${CHILD_IMAGE_NAME}
    BOARD ${CONFIG_DOMAIN_CPUNET_BOARD}
    )
endif()
