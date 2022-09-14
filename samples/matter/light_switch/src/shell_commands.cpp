/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "shell_commands.h"
#include "binding_handler.h"
#include "light_switch.h"

#include <platform/CHIPDeviceLayer.h>

#ifdef CONFIG_CHIP_LIB_SHELL

using namespace chip;
using namespace chip::app;

namespace SwitchCommands
{
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

constexpr static EndpointId kLightSwitchEndpointId = 1;
constexpr static ClusterId kOnOffClusterId = 6;
constexpr static ClusterId kLevelControlClusterId = 8;

Engine sShellSwitchSubCommands;
Engine sShellSwitchBindSubCommands;
Engine sShellSwitchOnOffSubCommands;
Engine sShellSwitchGroupsSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;

static CHIP_ERROR SwitchHelpHandler(int argc, char **argv)
{
	sShellSwitchSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
	return CHIP_NO_ERROR;
}

static CHIP_ERROR FindDevicesHandler(int argc, char **argv)
{
	DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::LookForDevices, 0);

	return CHIP_NO_ERROR;
}

static CHIP_ERROR SwitchBindHelpHandler(int argc, char **argv)
{
	sShellSwitchBindSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
	return CHIP_NO_ERROR;
}

static CHIP_ERROR SwitchBindCommandHandler(int argc, char **argv)
{
	if (argc == 0) {
		return SwitchBindHelpHandler(argc, argv);
	}
	return sShellSwitchBindSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR SwitchCommandHandler(int argc, char **argv)
{
	if (argc == 0) {
		return SwitchHelpHandler(argc, argv);
	}
	return sShellSwitchSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR TableCommandHelper(int argc, char **argv)
{
	BindingHandler::GetInstance().PrintBindingTable();
	return CHIP_NO_ERROR;
}

namespace Unicast
{
	static CHIP_ERROR OnOffHelpHandler(int argc, char **argv)
	{
		sShellSwitchOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR OnOffCommandHandler(int argc, char **argv)
	{
		if (argc == 0) {
			return OnOffHelpHandler(argc, argv);
		}
		return sShellSwitchOnOffSubCommands.ExecCommand(argc, argv);
	}

	static CHIP_ERROR OnBindHandler(int argc, char **argv)
	{
		if(argc < 3){
			sShellSwitchBindSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
			return CHIP_ERROR_INVALID_ARGUMENT;
		}

		EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
		entry->type                    = EMBER_UNICAST_BINDING;
		entry->fabricIndex             = atoi(argv[0]);
		entry->nodeId                  = atoi(argv[1]);
		entry->local                   = kLightSwitchEndpointId;
		entry->remote                  = atoi(argv[2]);
		entry->clusterId.SetValue(kOnOffClusterId);
		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));

		entry = Platform::New<EmberBindingTableEntry>();
		entry->type                    = EMBER_UNICAST_BINDING;
		entry->fabricIndex             = atoi(argv[0]);
		entry->nodeId                  = atoi(argv[1]);
		entry->local                   = kLightSwitchEndpointId;
		entry->remote                  = atoi(argv[2]);
		entry->clusterId.SetValue(kLevelControlClusterId);
		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));

		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR OnCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::On::Id;
		data->ClusterId = Clusters::OnOff::Id;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR OffCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::Off::Id;
		data->ClusterId = Clusters::OnOff::Id;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR ToggleCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::Toggle::Id;
		data->ClusterId = Clusters::OnOff::Id;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}
} /* namespace Unicast */

namespace Group
{
	CHIP_ERROR SwitchHelpHandler(int argc, char **argv)
	{
		sShellSwitchGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
		return CHIP_NO_ERROR;
	}

	CHIP_ERROR SwitchCommandHandler(int argc, char **argv)
	{
		if (argc == 0) {
			return SwitchHelpHandler(argc, argv);
		}

		return sShellSwitchGroupsSubCommands.ExecCommand(argc, argv);
	}

	static CHIP_ERROR OnBindHandler(int argc, char **argv)
	{
		if(argc < 2){
			sShellSwitchBindSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
			return CHIP_ERROR_INVALID_ARGUMENT;
		}

		// set binding to onoff cluster
		EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
		entry->type                    = EMBER_MULTICAST_BINDING;
		entry->fabricIndex             = atoi(argv[0]);
		entry->groupId                 = atoi(argv[1]);
		entry->local                   = kLightSwitchEndpointId;
		entry->clusterId.SetValue(kOnOffClusterId);      
		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));

		// set binding to levelcontrol cluster
		entry = Platform::New<EmberBindingTableEntry>();
		entry->type                    = EMBER_MULTICAST_BINDING;
		entry->fabricIndex             = atoi(argv[0]);
		entry->groupId                 = atoi(argv[1]);
		entry->local                   = kLightSwitchEndpointId;
		entry->clusterId.SetValue(kLevelControlClusterId);      
		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));

		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR OnOffHelpHandler(int argc, char **argv)
	{
		sShellSwitchGroupsOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
		return CHIP_NO_ERROR;
	}

	static CHIP_ERROR OnOffCommandHandler(int argc, char **argv)
	{
		if (argc == 0) {
			return OnOffHelpHandler(argc, argv);
		}

		return sShellSwitchGroupsOnOffSubCommands.ExecCommand(argc, argv);
	}

	CHIP_ERROR OnCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::On::Id;
		data->ClusterId = Clusters::OnOff::Id;
		data->IsGroup = true;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}

	CHIP_ERROR OffCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::Off::Id;
		data->ClusterId = Clusters::OnOff::Id;
		data->IsGroup = true;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}

	CHIP_ERROR ToggleCommandHandler(int argc, char **argv)
	{
		BindingHandler::BindingData *data = Platform::New<BindingHandler::BindingData>();
		data->EndpointId = LightSwitch::GetInstance().GetLightSwitchEndpointId();
		data->CommandId = Clusters::OnOff::Commands::Toggle::Id;
		data->ClusterId = Clusters::OnOff::Id;
		data->IsGroup = true;

		DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler,
							reinterpret_cast<intptr_t>(data));
		return CHIP_NO_ERROR;
	}

} /* namespace Group */

void RegisterSwitchCommands()
{
	static const shell_command_t sSwitchSubCommands[] = {
		{ &SwitchHelpHandler, "help", "Switch commands" },
		{ &FindDevicesHandler, "find",
		  "Scan within a Fabric to find devices that can be bound with light-switch" },
		{ &Unicast::OnOffCommandHandler, "onoff", "Usage: switch onoff [on|off|toggle]" },
		{ &Group::SwitchCommandHandler, "groups", "Usage: switch groups onoff [on|off|toggle]" },
		{ &SwitchBindCommandHandler, "bind", "Bind to the device. Usage: switch bind [group, unicast]" },
		{ &TableCommandHelper, "table", "Print a binding table" }
	};

	static const shell_command_t sSwitchBindCommands[] {
		{&Unicast::OnBindHandler, "unicast", "Bind single device to make unicast connection. Usage: switch unicast [fabricId] [nodeId] [endpoint]"},
		{&Group::OnBindHandler, "group", "Bind the Light Switch to the given group. UsageL switch group [fabricId] [groupId]"}
	};

	static const shell_command_t sSwitchOnOffSubCommands[] = {
		{ &Unicast::OnOffHelpHandler, "help", "Usage : switch ononff [on|off|toggle]" },
		{ &Unicast::OnCommandHandler, "on", "Sends on command to bound lighting app" },
		{ &Unicast::OffCommandHandler, "off", "Sends off command to bound lighting app" },
		{ &Unicast::ToggleCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
	};

	static const shell_command_t sSwitchGroupsSubCommands[] = {
		{ &Group::SwitchHelpHandler, "help", "switch a group of bounded lightning apps" },
		{ &Group::OnOffCommandHandler, "onoff", "Usage: switch groups onoff [on|off|toggle]" }
	};

	static const shell_command_t sSwichGroupsOnOffSubCommands[] = {
		{ &Group::OnOffHelpHandler, "help", "Usage: switch groups onoff [on|off|toggle]" },
		{ &Group::OnCommandHandler, "on", "Sends on command to bound Group" },
		{ &Group::OffCommandHandler, "off", "Sends off command to bound Group" },
		{ &Group::ToggleCommandHandler, "toggle", "Sends toggle command to bound Group" }
	};

	static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
							"Light-switch commands. Usage: switch [onoff|groups|find|table|bind]" };

	sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwichGroupsOnOffSubCommands,
							    ArraySize(sSwichGroupsOnOffSubCommands));
	sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, ArraySize(sSwitchOnOffSubCommands));
	sShellSwitchBindSubCommands.RegisterCommands(sSwitchBindCommands, ArraySize(sSwitchBindCommands));
	sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, ArraySize(sSwitchGroupsSubCommands));
	sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, ArraySize(sSwitchSubCommands));

	Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}

} /* namespace SwitchCommands */
#endif
