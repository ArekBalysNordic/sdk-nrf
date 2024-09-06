/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file dfu_target_suit.h
 *
 * @defgroup dfu_target_suit SUIT DFU Target
 * @{
 * @brief DFU Target for upgrades performed by SUIT
 */

#ifndef DFU_TARGET_SUIT_H__
#define DFU_TARGET_SUIT_H__

#include <stddef.h>
#include <dfu/dfu_target.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set buffer to use for flash write operations.
 *
 * @retval Non-negative value if successful, -EINVAL negative errno otherwise.
 */
int dfu_target_suit_set_buf(uint8_t *buf, size_t len);

/**
 * @brief Initialize dfu target for specific image, perform steps necessary to receive firmware.
 *
 * If you call this function, you must call dfu_target_suit_done() to finalize the firmware upgrade
 * before initializing any other images.
 *
 * @param[in] file_size Size of the current file being downloaded.
 * @param[in] img_num Image index.
 * @param[in] cb Callback for signaling events(unused).
 *
 * @retval 0 If successful
 * @retval -ENODEV errno code if the buffer has not been initialized.
 * @retval -ENXIO errno code if the partition dedicated for provided image is not found.
 * @retval -ENOMEM errno code if the buffer is not large enough.
 * @retval -EFAULT errno code if flash device assigned to the image is not available on the device.
 * @retval -EBUSY errno code if the any image is already initialized and stream flash is in use.
 * @retval other negative errno code if the initialization failed.
 */
int dfu_target_suit_init(size_t file_size, int img_num, dfu_target_callback_t cb);

/**
 * @brief Get offset of firmware
 *
 * @param[out] offset Returns the offset of the firmware upgrade.
 *
 * @retval 0 if success, otherwise negative value if unable to get the offset
 */
int dfu_target_suit_offset_get(size_t *offset);

/**
 * @brief Write firmware data.
 *
 * @param[in] buf Pointer to data that should be written.
 * @param[in] len Length of data to write.
 *
 * @retval 0 on success
 * @retval -EFAULT errno code if the stream flash has not been initialized for any dfu image.
 * @retval other negative errno code if the initialization failed.
 */
int dfu_target_suit_write(const void *const buf, size_t len);

/**
 * @brief Deinitialize resources and finalize firmware upgrade if successful.

 * @param[in] successful Indicate whether the firmware was successfully recived.
 *
 * @retval 0 on success, negative errno otherwise.
 */
int dfu_target_suit_done(bool successful);

/**
 * @brief Schedule update and reset the device.
 *
 * This call requests images update and immediately starts it
 * by resetting the device.
 *
 * @param[in] img_num Given image pair index or -1 for all
 *		      of image pair indexes.
 *
 * @retval 0 for a successful request or a negative error
 *	   code identicating reason of failure.
 **/
int dfu_target_suit_schedule_update(int img_num);

/**
 * @brief Release resources and erase the download area.
 *
 * Cancels any ongoing updates.
 *
 * @retval 0 on success, negative errno otherwise.
 */
int dfu_target_suit_reset(void);

/**
 * @brief Reboot the device, and apply new image.
 *
 * @retval 0 on success, negative errno otherwise.
 */
int dfu_target_suit_reboot(void);

#ifdef __cplusplus
}
#endif

#endif /* DFU_TARGET_SUIT_H__ */

/**@} */
