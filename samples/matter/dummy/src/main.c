/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#ifndef CONFIG_SOC_SERIES_NRF54HX
#include <zephyr/dfu/mcuboot.h>
#endif

int main(void)
{
	printk("Hello world from %s\n", CONFIG_BOARD);

#ifndef CONFIG_SOC_SERIES_NRF54HX
	if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT) {
		if (boot_write_img_confirmed()) {
			printk("Failed to confirm firmware image, it will be reverted on the next boot\n");
		} else {
			printk("New firmware image confirmed\n");
		}
	}
#endif
	return 0;
}
