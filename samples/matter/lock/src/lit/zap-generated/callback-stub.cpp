/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// THIS FILE IS GENERATED BY ZAP

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/Constants.h>

using namespace chip;

// Cluster Init Functions
void emberAfClusterInitCallback(EndpointId endpoint, ClusterId clusterId)
{
	switch (clusterId) {
	case app::Clusters::AccessControl::Id:
		emberAfAccessControlClusterInitCallback(endpoint);
		break;
	case app::Clusters::AdministratorCommissioning::Id:
		emberAfAdministratorCommissioningClusterInitCallback(endpoint);
		break;
	case app::Clusters::BasicInformation::Id:
		emberAfBasicInformationClusterInitCallback(endpoint);
		break;
	case app::Clusters::Descriptor::Id:
		emberAfDescriptorClusterInitCallback(endpoint);
		break;
	case app::Clusters::DiagnosticLogs::Id:
		emberAfDiagnosticLogsClusterInitCallback(endpoint);
		break;
	case app::Clusters::DoorLock::Id:
		emberAfDoorLockClusterInitCallback(endpoint);
		break;
	case app::Clusters::GeneralCommissioning::Id:
		emberAfGeneralCommissioningClusterInitCallback(endpoint);
		break;
	case app::Clusters::GeneralDiagnostics::Id:
		emberAfGeneralDiagnosticsClusterInitCallback(endpoint);
		break;
	case app::Clusters::GroupKeyManagement::Id:
		emberAfGroupKeyManagementClusterInitCallback(endpoint);
		break;
	case app::Clusters::IcdManagement::Id:
		emberAfIcdManagementClusterInitCallback(endpoint);
		break;
	case app::Clusters::Identify::Id:
		emberAfIdentifyClusterInitCallback(endpoint);
		break;
	case app::Clusters::NetworkCommissioning::Id:
		emberAfNetworkCommissioningClusterInitCallback(endpoint);
		break;
	case app::Clusters::OtaSoftwareUpdateProvider::Id:
		emberAfOtaSoftwareUpdateProviderClusterInitCallback(endpoint);
		break;
	case app::Clusters::OtaSoftwareUpdateRequestor::Id:
		emberAfOtaSoftwareUpdateRequestorClusterInitCallback(endpoint);
		break;
	case app::Clusters::OperationalCredentials::Id:
		emberAfOperationalCredentialsClusterInitCallback(endpoint);
		break;
	case app::Clusters::SoftwareDiagnostics::Id:
		emberAfSoftwareDiagnosticsClusterInitCallback(endpoint);
		break;
	case app::Clusters::ThreadNetworkDiagnostics::Id:
		emberAfThreadNetworkDiagnosticsClusterInitCallback(endpoint);
		break;
	default:
		// Unrecognized cluster ID
		break;
	}
}

void __attribute__((weak)) emberAfAccessControlClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfAdministratorCommissioningClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfBasicInformationClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfDescriptorClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfDiagnosticLogsClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfGeneralCommissioningClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfGeneralDiagnosticsClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfGroupKeyManagementClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfIcdManagementClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfIdentifyClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfNetworkCommissioningClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfOtaSoftwareUpdateProviderClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfOtaSoftwareUpdateRequestorClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfOperationalCredentialsClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfSoftwareDiagnosticsClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
void __attribute__((weak)) emberAfThreadNetworkDiagnosticsClusterInitCallback(EndpointId endpoint)
{
	// To prevent warning
	(void)endpoint;
}
