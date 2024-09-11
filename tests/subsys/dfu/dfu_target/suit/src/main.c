/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/ztest.h>
#include <dfu/dfu_target.h>
#include <dfu/dfu_target_suit.h>
#include <zephyr/devicetree.h>

#define TEST_IMAGE_SIZE		1024
#define TEST_IMAGE_ENVELOPE_NUM 0
#define TEST_IMAGE_APP_NUM	2

static uint8_t dfu_target_buffer[TEST_IMAGE_SIZE];

/* The SUIT envelopes are defined inside the respective manfest_*.c files. */
extern uint8_t manifest[];
extern const size_t manifest_len;
extern uint8_t dfu_cache_partition_1[];
extern const size_t dfu_cache_partition_1_len;

ZTEST(dfu_target_suit, test_image_init)
{
	int rc;

	rc = dfu_target_suit_reset();
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_ENVELOPE_NUM, TEST_IMAGE_SIZE,
			     NULL);
	zassert_equal(rc, -ENODEV,
		      "dfu_target should fail because the buffer has not been initialized. %d", rc);

	rc = dfu_target_suit_set_buf(dfu_target_buffer, sizeof(dfu_target_buffer));
	zassert_equal(rc, 0, "dfu_target should initialize the data buffer %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_ENVELOPE_NUM,
			     TEST_IMAGE_SIZE * 1000, NULL);
	zassert_equal(rc, -EFBIG, "dfu_target should not allow too big files. %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_ENVELOPE_NUM, TEST_IMAGE_SIZE,
			     NULL);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_ENVELOPE_NUM, TEST_IMAGE_SIZE,
			     NULL);
	zassert_equal(rc, -EBUSY,
		      "dfu_target should not initialize the same image twice until the stream is "
		      "not ended or reset. %d",
		      rc);

	rc = dfu_target_suit_reset();
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_APP_NUM, TEST_IMAGE_SIZE, NULL);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);
}

ZTEST(dfu_target_suit, test_image_upload)
{
	int rc;
	size_t offset = 0;

	rc = dfu_target_suit_reset();
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_write(manifest, TEST_IMAGE_SIZE);
	zassert_equal(rc, -EFAULT, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_schedule_update(0);
	zassert_equal(rc, -ENOTSUP,
		      "scheduling update should file because there is no manifest stored yet: %d",
		      rc);

	rc = dfu_target_suit_offset_get(&offset);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);
	zassert_equal(offset, 0, "DFU target offset should be equal to 0");

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_ENVELOPE_NUM, manifest_len,
			     NULL);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_write(manifest, manifest_len);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_offset_get(&offset);
	zassert_equal(rc, 0, "DFU target offset get should pass for image 0 %d", rc);
	zassert_equal(offset, manifest_len, "DFU target offset should be equal to image size");

	rc = dfu_target_done(true);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_init(DFU_TARGET_IMAGE_TYPE_SUIT, TEST_IMAGE_APP_NUM,
			     dfu_cache_partition_1_len, NULL);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_write(dfu_cache_partition_1, dfu_cache_partition_1_len);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_offset_get(&offset);
	zassert_equal(rc, 0, "DFU target offset get should pass for image 0 %d", rc);
	zassert_equal(offset, dfu_cache_partition_1_len,
		      "DFU target offset should be equal to image size");

	rc = dfu_target_done(true);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);

	rc = dfu_target_suit_schedule_update(0);
	zassert_equal(rc, 0, "Unexpected failure: %d", rc);
}

ZTEST_SUITE(dfu_target_suit, NULL, NULL, NULL, NULL, NULL);
