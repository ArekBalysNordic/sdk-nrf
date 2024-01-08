/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_NCS_SAMPLE_STORAGE_STATS

#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CodeUtils.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

LOG_MODULE_DECLARE(storage_manager, CONFIG_CHIP_APP_LOG_LEVEL);

namespace Nordic
{

namespace Matter
{

	using namespace chip;
	using Shell::Engine;
	using Shell::shell_command_t;
	using Shell::streamer_get;
	using Shell::streamer_printf;

	static Engine sShellStorageCommands;
	static Engine sShellStorageSubCommands;

	static bool GetStorageFreeSpace(size_t &freeBytes)
	{
		void *storage = nullptr;
		int status = settings_storage_get(&storage);
		if (status != 0 || !storage) {
			LOG_ERR("CredentialsStorage: Cannot read NVS free space [error: %d]", status);
			return false;
		}
		freeBytes = nvs_calc_free_space(static_cast<nvs_fs *>(storage));
		return true;
	}

	static bool GetStorageUsedSpace(size_t &usedBytes)
	{
		return true;
	}

	CHIP_ERROR Help(int argc, char **argv)
	{
		sShellStorageSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR StorageCommand(int argc, char **argv)
	{
		if (argc == 0) {
			return Help(argc, argv);
		}
		return sShellStorageSubCommands.ExecCommand(argc, argv);
	}

	void RegisterStorageStatsCommands()
	{
		static const shell_command_t sStorageCommand = { &StorageCommand, "storage",
								 "Matter Storage CLI commands" };

		static const shell_command_t sStorageSubcommands[] = {
			{ [](int argc, char **argv) {
				 size_t storageFreeSpace = 0;
				 if (GetStorageFreeSpace(storageFreeSpace)) {
					 printk("%d\n", storageFreeSpace);
				 }
				 return CHIP_NO_ERROR;
			 },
			  "free", "Print free storage space" },
			{ [](int argc, char **argv) {
				 size_t storageUsedSpace = 0;
				 if (GetStorageUsedSpace(storageUsedSpace)) {
					 printk("%d\n", storageUsedSpace);
				 }
				 return CHIP_NO_ERROR;
			 },
			  "used", "Print free storage space" },
			{ [](int argc, char **argv) { return CHIP_NO_ERROR; }, "list",
			  "Print list of used storage entries" },
			{ [](int argc, char **argv) { return CHIP_NO_ERROR; }, "detailed",
			  "Print list of used storage entries" },
			{ [](int argc, char **argv) {
				 size_t storageFreeSpace = 0;
				 size_t storageUsedSpace = 0;
				 size_t storageTotalSpace = 0;
				 printk("Declared: %d Bytes\nUsed:     %d Bytes\nFree:     %d Bytes\n",
					 storageTotalSpace, storageUsedSpace, storageFreeSpace);
				 return CHIP_NO_ERROR;
			 },
			  "space", "Print declared, used, and free current storage space" },
		};

		sShellStorageSubCommands.RegisterCommands(sStorageSubcommands, ArraySize(sStorageSubcommands));
		Engine::Root().RegisterCommands(&sStorageCommand, 1);
	}

} // namespace Matter
} // namespace Nordic

#endif