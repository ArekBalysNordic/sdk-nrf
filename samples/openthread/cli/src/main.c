/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#if defined(CONFIG_CLI_SAMPLE_MULTIPROTOCOL)
#include "ble.h"
#endif

#if defined(CONFIG_CLI_SAMPLE_LOW_POWER)
#include "low_power.h"
#endif

#if defined(CONFIG_CLI_SAMPLE_AUTOSTART)
#include <openthread.h>
#include <openthread/thread.h>
#include <openthread/link.h>
#endif

#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(cli_sample, CONFIG_OT_COMMAND_LINE_INTERFACE_LOG_LEVEL);

#define WELLCOME_TEXT                                                                              \
	"\n\r"                                                                                     \
	"\n\r"                                                                                     \
	"OpenThread Command Line Interface is now running.\n\r"                                    \
	"Use the 'ot' keyword to invoke OpenThread commands e.g. "                                 \
	"'ot thread start.'\n\r"                                                                   \
	"For the full commands list refer to the OpenThread CLI "                                  \
	"documentation at:\n\r"                                                                    \
	"https://github.com/openthread/openthread/blob/master/src/cli/README.md\n\r"

int main(void)
{
#if DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_shell_uart), zephyr_cdc_acm_uart)
	int ret;
	const struct device *dev;
	uint32_t dtr = 0U;

	dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));
	if (dev == NULL) {
		LOG_ERR("Failed to find specific UART device");
		return 0;
	}

	LOG_INF("Waiting for host to be ready to communicate");

	/* Data Terminal Ready - check if host is ready to communicate */
	while (!dtr) {
		ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		if (ret) {
			LOG_ERR("Failed to get Data Terminal Ready line state: %d", ret);
			continue;
		}
		k_msleep(100);
	}

	/* Data Carrier Detect Modem - mark connection as established */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
	/* Data Set Ready - the NCP SoC is ready to communicate */
	(void)uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
#endif

	LOG_INF(WELLCOME_TEXT);

#if defined(CONFIG_CLI_SAMPLE_MULTIPROTOCOL)
	ble_enable();
#endif

#if defined(CONFIG_CLI_SAMPLE_LOW_POWER)
	low_power_enable();
#endif

#if defined(CONFIG_CLI_SAMPLE_AUTOSTART)
	otInstance *instance = openthread_get_default_instance();
	if (instance == NULL) {
		LOG_ERR("Failed to get OpenThread instance");
		return -1;
	}

	// Set the channel
	uint8_t channel = 23;
	otError err = otLinkSetChannel(instance, channel);
	if (err != OT_ERROR_NONE) {
		LOG_ERR("Failed to set channel: %d", err);
	}

	// Set network key
	const uint8_t networkKey[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
				      0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
	err = otThreadSetNetworkKey(instance, (const otNetworkKey *)networkKey);
	if (err != OT_ERROR_NONE) {
		LOG_ERR("Failed to set network key: %d", err);
	}

	// Set PAN ID
	otPanId panid = 0x1234;
	err = otLinkSetPanId(instance, panid);
	if (err != OT_ERROR_NONE) {
		LOG_ERR("Failed to set PAN ID: %d", err);
	}

	openthread_run();

#endif

	return 0;
}
