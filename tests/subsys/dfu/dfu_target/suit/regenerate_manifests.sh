#!/bin/bash
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Scrtipt to regenerate SUIT manifests for DFU target tests.
# To use this script you must have the following tools installed:
# - suit-generator
# - zcbor
#
# The script will update the following files:
# - dfu_cache_partition_1.c which conatins the binary cache as C code
# - manifest.c which contains the signed manifest as C code
#
# Before using this script paste the following files into the nrf/tests/subsys/suit/manifest_common directory:
# - file.bin, the testing binary file.


cd ../../../suit/manifest_common

### Generate testing cache file ###
./generate_dfu_cache.sh file.bin

### Generate Manifests ###
./regenerate.sh ../../dfu/dfu_target/suit/manifests/manifest_root.yaml
./regenerate.sh ../../dfu/dfu_target/suit/manifests/manifest_app.yaml

### Creating C source files ###
echo "Converting binary files into C code files ..."
python3 replace_manifest.py ../../dfu/dfu_target/suit/src/manifest.c sample_signed.suit
python3 replace_manifest.py ../../dfu/dfu_target/suit/src/dfu_cache_partition_1.c sample_cache.bin
