/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief OpenThread CLI vendor extension for UDP transmission testing.
 *
 * Provides `hdr` command to send large UDP datagrams through the complete
 * OpenThread stack (UDP -> IP6 -> Mesh Forwarder -> Radio).
 *
 * Usage:
 *   hdr <length>  - Send UDP datagram of specified length bytes (1-1024)
 */

#include <stdlib.h>
#include <string.h>

#include <zephyr/logging/log.h>

#include <openthread.h>
#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/thread.h>
#include <openthread/udp.h>

LOG_MODULE_REGISTER(cli_hdr_test, CONFIG_LOG_DEFAULT_LEVEL);

#define UDP_TX_MAX_PAYLOAD_SIZE 1024

/**
 * @brief Send a UDP datagram using OpenThread UDP API (like otperf).
 * 
 * This sends data through the full OpenThread stack: UDP -> IP6 -> mesh forwarder -> radio.
 * For HDR testing, this exercises the complete path including TryTransmitHdrPacket when
 * the packet is large enough and sent to an HDR-capable neighbor.
 * 
 * Usage: hdr <length>
 */
static otError hdrCommand(void *aContext, uint8_t aArgsLength, char *aArgs[])
{
	otInstance *instance;
	otError error;
	otMessage *message = NULL;
	otMessageInfo messageInfo;
	uint16_t length = 128;  /* Default: large enough to trigger HDR if available */
	uint8_t *payload = NULL;
	long val;
	
	ARG_UNUSED(aContext);
	
	instance = openthread_get_default_instance();
	if (instance == NULL) {
		otCliOutputFormat("hdr: OpenThread instance not available\r\n");
		return OT_ERROR_INVALID_STATE;
	}
	
	/* Parse length argument */
	if (aArgsLength > 0 && aArgs[0] != NULL) {
		val = strtol(aArgs[0], NULL, 0);
		if (val < 1 || val > UDP_TX_MAX_PAYLOAD_SIZE) {
			otCliOutputFormat("hdr: length must be 1-%u (got %ld)\r\n",
					  UDP_TX_MAX_PAYLOAD_SIZE, val);
			return OT_ERROR_INVALID_ARGS;
		}
		length = (uint16_t)val;
	} else {
		otCliOutputFormat("hdr: usage: hdr <length>\r\n");
		return OT_ERROR_INVALID_ARGS;
	}
	
	/* Allocate payload buffer */
	payload = (uint8_t *)malloc(length);
	if (payload == NULL) {
		otCliOutputFormat("hdr: failed to allocate payload buffer\r\n");
		return OT_ERROR_NO_BUFS;
	}
	
	/* Fill with test pattern (same as otperf) */
	memset(payload, 'n', length);
	
	/* Create UDP message */
	LOG_INF("=== HDR TEST: Creating UDP message (length=%u) ===", length);
	message = otUdpNewMessage(instance, NULL);
	if (message == NULL) {
		otCliOutputFormat("hdr: failed to allocate message\r\n");
		LOG_ERR("HDR TEST: Message allocation failed");
		error = OT_ERROR_NO_BUFS;
		goto exit;
	}
	LOG_INF("HDR TEST: Message allocated successfully");
	
	/* Append payload to message */
	error = otMessageAppend(message, payload, length);
	if (error != OT_ERROR_NONE) {
		otCliOutputFormat("hdr: failed to append payload (%d)\r\n", error);
		LOG_ERR("HDR TEST: Failed to append payload");
		goto exit;
	}
	LOG_INF("HDR TEST: Payload appended (%u bytes)", length);
	
	/* Set up message info - send to a destination address */
	memset(&messageInfo, 0, sizeof(messageInfo));
	
	LOG_INF("HDR TEST: Resolving destination address...");
	
	const otMeshLocalPrefix *meshLocalPrefix = otThreadGetMeshLocalPrefix(instance);
	if (meshLocalPrefix == NULL) {
		otCliOutputFormat("hdr: failed to get mesh local prefix\r\n");
		LOG_ERR("HDR TEST: Failed to get mesh local prefix");
		error = OT_ERROR_INVALID_STATE;
		goto exit;
	}
	
#ifdef CONFIG_OPENTHREAD_RADIO_2P4GHZ_HDR_ASSUME_NEIGHBOR_SUPPORT
	/* TEST MODE: Use a dummy RLOC16 that's different from our own */
	uint16_t myRloc16 = otThreadGetRloc16(instance);
	uint16_t testRloc16 = (myRloc16 == 0x8800) ? 0x8801 : 0x8800;  /* Use different RLOC */
	
	LOG_INF("HDR TEST: TEST MODE - Using dummy destination RLOC16=0x%04x (my RLOC16=0x%04x)", 
	        testRloc16, myRloc16);
	LOG_INF("HDR TEST: Neighbor capability check is BYPASSED");
	
	memcpy(&messageInfo.mPeerAddr, meshLocalPrefix, sizeof(otMeshLocalPrefix));
	messageInfo.mPeerAddr.mFields.m8[8] = 0x00;
	messageInfo.mPeerAddr.mFields.m8[9] = 0x00;
	messageInfo.mPeerAddr.mFields.m8[10] = 0x00;
	messageInfo.mPeerAddr.mFields.m8[11] = 0xff;
	messageInfo.mPeerAddr.mFields.m8[12] = 0xfe;
	messageInfo.mPeerAddr.mFields.m8[13] = 0x00;
	messageInfo.mPeerAddr.mFields.m8[14] = (uint8_t)(testRloc16 >> 8);
	messageInfo.mPeerAddr.mFields.m8[15] = (uint8_t)(testRloc16 & 0xFF);
#else
	/* NORMAL MODE: Try to get parent's RLOC address (works for Child and Router roles) */
	otRouterInfo parentInfo;
	error = otThreadGetParentInfo(instance, &parentInfo);
	
	if (error == OT_ERROR_NONE) {
		/* Build parent RLOC address from mesh local prefix and parent RLOC16 */
		memcpy(&messageInfo.mPeerAddr, meshLocalPrefix, sizeof(otMeshLocalPrefix));
		messageInfo.mPeerAddr.mFields.m8[8] = 0x00;
		messageInfo.mPeerAddr.mFields.m8[9] = 0x00;
		messageInfo.mPeerAddr.mFields.m8[10] = 0x00;
		messageInfo.mPeerAddr.mFields.m8[11] = 0xff;
		messageInfo.mPeerAddr.mFields.m8[12] = 0xfe;
		messageInfo.mPeerAddr.mFields.m8[13] = 0x00;
		messageInfo.mPeerAddr.mFields.m8[14] = (uint8_t)(parentInfo.mRloc16 >> 8);
		messageInfo.mPeerAddr.mFields.m8[15] = (uint8_t)(parentInfo.mRloc16 & 0xFF);
		LOG_INF("HDR TEST: Using parent RLOC16=0x%04x", parentInfo.mRloc16);
	} else if (otThreadGetDeviceRole(instance) == OT_DEVICE_ROLE_LEADER) {
		/* Leader has no parent, try to find a neighbor to send to */
		uint16_t myRloc16 = otThreadGetRloc16(instance);
		LOG_INF("HDR TEST: Device is leader with RLOC16=0x%04x", myRloc16);
		
		/* Get the first neighbor */
		otNeighborInfoIterator iterator = OT_NEIGHBOR_INFO_ITERATOR_INIT;
		otNeighborInfo neighborInfo;
		
		if (otThreadGetNextNeighborInfo(instance, &iterator, &neighborInfo) == OT_ERROR_NONE) {
			/* Found a neighbor, use its RLOC16 */
			memcpy(&messageInfo.mPeerAddr, meshLocalPrefix, sizeof(otMeshLocalPrefix));
			messageInfo.mPeerAddr.mFields.m8[8] = 0x00;
			messageInfo.mPeerAddr.mFields.m8[9] = 0x00;
			messageInfo.mPeerAddr.mFields.m8[10] = 0x00;
			messageInfo.mPeerAddr.mFields.m8[11] = 0xff;
			messageInfo.mPeerAddr.mFields.m8[12] = 0xfe;
			messageInfo.mPeerAddr.mFields.m8[13] = 0x00;
			messageInfo.mPeerAddr.mFields.m8[14] = (uint8_t)(neighborInfo.mRloc16 >> 8);
			messageInfo.mPeerAddr.mFields.m8[15] = (uint8_t)(neighborInfo.mRloc16 & 0xFF);
			LOG_INF("HDR TEST: Sending to neighbor RLOC16=0x%04x", neighborInfo.mRloc16);
		} else {
			/* No neighbors found - leader is alone in network */
			otCliOutputFormat("hdr: leader has no neighbors/children to send to\r\n");
			LOG_ERR("HDR TEST: Leader is alone, add another device to the network");
			error = OT_ERROR_INVALID_STATE;
			goto exit;
		}
	} else {
		otCliOutputFormat("hdr: not attached to network or no parent\r\n");
		LOG_ERR("HDR TEST: Not attached to network or no parent");
		error = OT_ERROR_INVALID_STATE;
		goto exit;
	}
#endif
	
	messageInfo.mPeerPort = 12345;  /* Test port */
	messageInfo.mSockPort = 12345;
	
	LOG_INF("HDR TEST: Destination configured - port %u", messageInfo.mPeerPort);
	LOG_INF("HDR TEST: Calling otUdpSendDatagram() -> enters IP6 stack");
	
	/* Send via UDP (this goes through IP6 -> mesh forwarder -> radio) */
	error = otUdpSendDatagram(instance, message, &messageInfo);
	message = NULL;  /* otUdpSendDatagram takes ownership */
	
	if (error != OT_ERROR_NONE) {
		otCliOutputFormat("hdr: send failed (%d)\r\n", error);
		LOG_ERR("HDR TEST: otUdpSendDatagram() failed with error %d", error);
	} else {
		LOG_INF("HDR TEST: otUdpSendDatagram() returned success");
		LOG_INF("HDR TEST: Message queued in IP6/Mesh Forwarder");
		otCliOutputFormat("hdr: sent %u bytes via OpenThread UDP/IP6 stack\r\n", length);
		otCliOutputFormat("       (dest: ");
		for (int i = 0; i < 16; i++) {
			otCliOutputFormat("%02x", messageInfo.mPeerAddr.mFields.m8[i]);
			if (i < 15 && (i % 2) == 1) {
				otCliOutputFormat(":");
			}
		}
		otCliOutputFormat(")\r\n");
		LOG_INF("=== HDR TEST: Command completed, watch for MeshForwarder logs ===");
	}
	
exit:
	if (message != NULL) {
		otMessageFree(message);
	}
	if (payload != NULL) {
		free(payload);
	}
	return OT_ERROR_NONE;
}

static const otCliCommand sCommands[] = {
	{"hdr", hdrCommand},
};

void otCliVendorSetUserCommands(void)
{
	otCliSetUserCommands(sCommands, 1, NULL);
}
