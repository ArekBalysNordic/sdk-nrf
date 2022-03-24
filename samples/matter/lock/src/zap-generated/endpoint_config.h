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

// Prevent multiple inclusion
#pragma once

#include <lib/core/CHIPConfig.h>


// Default values for the attributes longer than a pointer,
// in a form of a binary blob
// Separate block is generated for big-endian and little-endian cases.
#if BIGENDIAN_CPU
#define GENERATED_DEFAULTS { \
\
  /* Endpoint: 0, Cluster: Descriptor (server), big-endian */\
\
  /* 0 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Basic (server), big-endian */\
\
  /* 4 - SoftwareVersion, */\
  0x00, 0x00, 0x00, 0x00, \
\
  /* 8 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: OTA Software Update Requestor (server), big-endian */\
\
  /* 12 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: General Commissioning (server), big-endian */\
\
  /* 16 - Breadcrumb, */\
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
\
  /* 24 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Network Commissioning (server), big-endian */\
\
  /* 28 - LastConnectErrorValue, */\
  0x00, 0x00, 0x00, 0x00, \
\
  /* 32 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: General Diagnostics (server), big-endian */\
\
  /* 36 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Software Diagnostics (server), big-endian */\
\
  /* 40 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: AdministratorCommissioning (server), big-endian */\
\
  /* 44 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Operational Credentials (server), big-endian */\
\
  /* 48 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 1, Cluster: On/Off (server), big-endian */\
\
  /* 52 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 1, Cluster: Descriptor (server), big-endian */\
\
  /* 56 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
}


#else // !BIGENDIAN_CPU
#define GENERATED_DEFAULTS { \
\
  /* Endpoint: 0, Cluster: Descriptor (server), little-endian */\
\
  /* 0 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Basic (server), little-endian */\
\
  /* 4 - SoftwareVersion, */\
  0x00, 0x00, 0x00, 0x00, \
\
  /* 8 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: OTA Software Update Requestor (server), little-endian */\
\
  /* 12 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: General Commissioning (server), little-endian */\
\
  /* 16 - Breadcrumb, */\
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
\
  /* 24 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Network Commissioning (server), little-endian */\
\
  /* 28 - LastConnectErrorValue, */\
  0x00, 0x00, 0x00, 0x00, \
\
  /* 32 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: General Diagnostics (server), little-endian */\
\
  /* 36 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Software Diagnostics (server), little-endian */\
\
  /* 40 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: AdministratorCommissioning (server), little-endian */\
\
  /* 44 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 0, Cluster: Operational Credentials (server), little-endian */\
\
  /* 48 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 1, Cluster: On/Off (server), little-endian */\
\
  /* 52 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
\
  /* Endpoint: 1, Cluster: Descriptor (server), little-endian */\
\
  /* 56 - FeatureMap, */\
  0x00, 0x00, 0x00, 0x00, \
\
}

#endif // BIGENDIAN_CPU

#define GENERATED_DEFAULTS_COUNT (14)

#define ZAP_TYPE(type) ZCL_ ## type ## _ATTRIBUTE_TYPE
#define ZAP_LONG_DEFAULTS_INDEX(index) { &generatedDefaults[index] }
#define ZAP_MIN_MAX_DEFAULTS_INDEX(index) { &minMaxDefaults[index] }
#define ZAP_EMPTY_DEFAULT() {(uint16_t) 0}
#define ZAP_SIMPLE_DEFAULT(x) {(uint16_t) x}

// This is an array of EmberAfAttributeMinMaxValue structures.
#define GENERATED_MIN_MAX_DEFAULT_COUNT 0
#define GENERATED_MIN_MAX_DEFAULTS { \
}


#define ZAP_ATTRIBUTE_MASK(mask) ATTRIBUTE_MASK_ ## mask
// This is an array of EmberAfAttributeMetadata structures.
#define GENERATED_ATTRIBUTE_COUNT 139
#define GENERATED_ATTRIBUTES { \
\
  /* Endpoint: 0, Cluster: Descriptor (server) */ \
  { 0x00000000, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* device list */  \
  { 0x00000001, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* server list */  \
  { 0x00000002, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* client list */  \
  { 0x00000003, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* parts list */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(0) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(1) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: Basic (server) */ \
  { 0x00000000, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* DataModelRevision */  \
  { 0x00000001, ZAP_TYPE(CHAR_STRING), 33, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* VendorName */  \
  { 0x00000002, ZAP_TYPE(VENDOR_ID), 2, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* VendorID */  \
  { 0x00000003, ZAP_TYPE(CHAR_STRING), 33, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* ProductName */  \
  { 0x00000004, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* ProductID */  \
  { 0x00000005, ZAP_TYPE(CHAR_STRING), 33, ZAP_ATTRIBUTE_MASK(SINGLETON) | ZAP_ATTRIBUTE_MASK(WRITABLE), ZAP_EMPTY_DEFAULT() }, /* NodeLabel */  \
  { 0x00000006, ZAP_TYPE(CHAR_STRING), 3, ZAP_ATTRIBUTE_MASK(SINGLETON) | ZAP_ATTRIBUTE_MASK(WRITABLE), ZAP_EMPTY_DEFAULT() }, /* Location */  \
  { 0x00000007, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_SIMPLE_DEFAULT(0x00) }, /* HardwareVersion */  \
  { 0x00000008, ZAP_TYPE(CHAR_STRING), 65, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* HardwareVersionString */  \
  { 0x00000009, ZAP_TYPE(INT32U), 4, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_LONG_DEFAULTS_INDEX(4) }, /* SoftwareVersion */  \
  { 0x0000000A, ZAP_TYPE(CHAR_STRING), 65, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_EMPTY_DEFAULT() }, /* SoftwareVersionString */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_LONG_DEFAULTS_INDEX(8) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, ZAP_ATTRIBUTE_MASK(SINGLETON), ZAP_SIMPLE_DEFAULT(3) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: OTA Software Update Requestor (server) */ \
  { 0x00000000, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(WRITABLE), ZAP_EMPTY_DEFAULT() }, /* DefaultOtaProviders */  \
  { 0x00000001, ZAP_TYPE(BOOLEAN), 1, 0, ZAP_SIMPLE_DEFAULT(1) }, /* UpdatePossible */  \
  { 0x00000002, ZAP_TYPE(ENUM8), 1, 0, ZAP_SIMPLE_DEFAULT(0) }, /* UpdateState */  \
  { 0x00000003, ZAP_TYPE(INT8U), 1, ZAP_ATTRIBUTE_MASK(NULLABLE), ZAP_EMPTY_DEFAULT() }, /* UpdateStateProgress */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(12) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(1) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: General Commissioning (server) */ \
  { 0x00000000, ZAP_TYPE(INT64U), 8, ZAP_ATTRIBUTE_MASK(WRITABLE), ZAP_LONG_DEFAULTS_INDEX(16) }, /* Breadcrumb */  \
  { 0x00000001, ZAP_TYPE(STRUCT), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* BasicCommissioningInfo */  \
  { 0x00000002, ZAP_TYPE(ENUM8), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RegulatoryConfig */  \
  { 0x00000003, ZAP_TYPE(ENUM8), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* LocationCapability */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(24) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: Network Commissioning (server) */ \
  { 0x00000000, ZAP_TYPE(INT8U), 1, 0, ZAP_EMPTY_DEFAULT() }, /* MaxNetworks */  \
  { 0x00000001, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* Networks */  \
  { 0x00000002, ZAP_TYPE(INT8U), 1, 0, ZAP_EMPTY_DEFAULT() }, /* ScanMaxTimeSeconds */  \
  { 0x00000003, ZAP_TYPE(INT8U), 1, 0, ZAP_EMPTY_DEFAULT() }, /* ConnectMaxTimeSeconds */  \
  { 0x00000004, ZAP_TYPE(BOOLEAN), 1, ZAP_ATTRIBUTE_MASK(WRITABLE), ZAP_EMPTY_DEFAULT() }, /* InterfaceEnabled */  \
  { 0x00000005, ZAP_TYPE(ENUM8), 1, ZAP_ATTRIBUTE_MASK(NULLABLE), ZAP_EMPTY_DEFAULT() }, /* LastNetworkingStatus */  \
  { 0x00000006, ZAP_TYPE(OCTET_STRING), 33, ZAP_ATTRIBUTE_MASK(NULLABLE), ZAP_EMPTY_DEFAULT() }, /* LastNetworkID */  \
  { 0x00000007, ZAP_TYPE(INT32S), 4, ZAP_ATTRIBUTE_MASK(NULLABLE), ZAP_LONG_DEFAULTS_INDEX(28) }, /* LastConnectErrorValue */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(32) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: General Diagnostics (server) */ \
  { 0x00000000, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* NetworkInterfaces */  \
  { 0x00000001, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RebootCount */  \
  { 0x00000002, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* UpTime */  \
  { 0x00000003, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TotalOperationalHours */  \
  { 0x00000004, ZAP_TYPE(ENUM8), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* BootReasons */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(36) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: Software Diagnostics (server) */ \
  { 0x00000001, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* CurrentHeapFree */  \
  { 0x00000002, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* CurrentHeapUsed */  \
  { 0x00000003, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* CurrentHeapHighWatermark */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(40) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: Thread Network Diagnostics (server) */ \
  { 0x00000000, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* channel */  \
  { 0x00000001, ZAP_TYPE(ENUM8), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RoutingRole */  \
  { 0x00000002, ZAP_TYPE(CHAR_STRING), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* NetworkName */  \
  { 0x00000003, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* PanId */  \
  { 0x00000004, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ExtendedPanId */  \
  { 0x00000005, ZAP_TYPE(OCTET_STRING), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* MeshLocalPrefix */  \
  { 0x00000006, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* OverrunCount */  \
  { 0x00000007, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* NeighborTableList */  \
  { 0x00000008, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RouteTableList */  \
  { 0x00000009, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* PartitionId */  \
  { 0x0000000A, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* weighting */  \
  { 0x0000000B, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* DataVersion */  \
  { 0x0000000C, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* StableDataVersion */  \
  { 0x0000000D, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* LeaderRouterId */  \
  { 0x0000000E, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* DetachedRoleCount */  \
  { 0x0000000F, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ChildRoleCount */  \
  { 0x00000010, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RouterRoleCount */  \
  { 0x00000011, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* LeaderRoleCount */  \
  { 0x00000012, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* AttachAttemptCount */  \
  { 0x00000013, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* PartitionIdChangeCount */  \
  { 0x00000014, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* BetterPartitionAttachAttemptCount */  \
  { 0x00000015, ZAP_TYPE(INT16U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ParentChangeCount */  \
  { 0x00000016, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxTotalCount */  \
  { 0x00000017, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxUnicastCount */  \
  { 0x00000018, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxBroadcastCount */  \
  { 0x00000019, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxAckRequestedCount */  \
  { 0x0000001A, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxAckedCount */  \
  { 0x0000001B, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxNoAckRequestedCount */  \
  { 0x0000001C, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxDataCount */  \
  { 0x0000001D, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxDataPollCount */  \
  { 0x0000001E, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxBeaconCount */  \
  { 0x0000001F, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxBeaconRequestCount */  \
  { 0x00000020, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxOtherCount */  \
  { 0x00000021, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxRetryCount */  \
  { 0x00000022, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxDirectMaxRetryExpiryCount */  \
  { 0x00000023, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxIndirectMaxRetryExpiryCount */  \
  { 0x00000024, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxErrCcaCount */  \
  { 0x00000025, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxErrAbortCount */  \
  { 0x00000026, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TxErrBusyChannelCount */  \
  { 0x00000027, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxTotalCount */  \
  { 0x00000028, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxUnicastCount */  \
  { 0x00000029, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxBroadcastCount */  \
  { 0x0000002A, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxDataCount */  \
  { 0x0000002B, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxDataPollCount */  \
  { 0x0000002C, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxBeaconCount */  \
  { 0x0000002D, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxBeaconRequestCount */  \
  { 0x0000002E, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxOtherCount */  \
  { 0x0000002F, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxAddressFilteredCount */  \
  { 0x00000030, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxDestAddrFilteredCount */  \
  { 0x00000031, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxDuplicatedCount */  \
  { 0x00000032, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrNoFrameCount */  \
  { 0x00000033, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrUnknownNeighborCount */  \
  { 0x00000034, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrInvalidSrcAddrCount */  \
  { 0x00000035, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrSecCount */  \
  { 0x00000036, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrFcsCount */  \
  { 0x00000037, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* RxErrOtherCount */  \
  { 0x00000038, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ActiveTimestamp */  \
  { 0x00000039, ZAP_TYPE(INT64U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* PendingTimestamp */  \
  { 0x0000003A, ZAP_TYPE(INT32U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* delay */  \
  { 0x0000003B, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* SecurityPolicy */  \
  { 0x0000003C, ZAP_TYPE(OCTET_STRING), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ChannelMask */  \
  { 0x0000003D, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* OperationalDatasetComponents */  \
  { 0x0000003E, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* ActiveNetworkFaultsList */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: AdministratorCommissioning (server) */ \
  { 0x00000000, ZAP_TYPE(INT8U), 1, 0, ZAP_EMPTY_DEFAULT() }, /* WindowStatus */  \
  { 0x00000001, ZAP_TYPE(FABRIC_IDX), 1, 0, ZAP_EMPTY_DEFAULT() }, /* AdminFabricIndex */  \
  { 0x00000002, ZAP_TYPE(INT16U), 2, 0, ZAP_EMPTY_DEFAULT() }, /* AdminVendorId */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(44) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(1) }, /* ClusterRevision */  \
\
  /* Endpoint: 0, Cluster: Operational Credentials (server) */ \
  { 0x00000000, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* NOCs */  \
  { 0x00000001, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* Fabrics */  \
  { 0x00000002, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* SupportedFabrics */  \
  { 0x00000003, ZAP_TYPE(INT8U), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* CommissionedFabrics */  \
  { 0x00000004, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* TrustedRootCertificates */  \
  { 0x00000005, ZAP_TYPE(FABRIC_IDX), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* CurrentFabricIndex */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(48) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(0x0001) }, /* ClusterRevision */  \
\
  /* Endpoint: 1, Cluster: On/Off (server) */ \
  { 0x00000000, ZAP_TYPE(BOOLEAN), 1, 0, ZAP_SIMPLE_DEFAULT(0x00) }, /* OnOff */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(52) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(2) }, /* ClusterRevision */  \
\
  /* Endpoint: 1, Cluster: Descriptor (server) */ \
  { 0x00000000, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* device list */  \
  { 0x00000001, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* server list */  \
  { 0x00000002, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* client list */  \
  { 0x00000003, ZAP_TYPE(ARRAY), 0, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() }, /* parts list */  \
  { 0x0000FFFC, ZAP_TYPE(BITMAP32), 4, 0, ZAP_LONG_DEFAULTS_INDEX(56) }, /* FeatureMap */  \
  { 0x0000FFFD, ZAP_TYPE(INT16U), 2, 0, ZAP_SIMPLE_DEFAULT(1) }, /* ClusterRevision */  \
}


// This is an array of EmberAfCluster structures.
#define ZAP_ATTRIBUTE_INDEX(index) (&generatedAttributes[index])

#define ZAP_GENERATED_COMMANDS_INDEX(index) ((chip::CommandId *) (&generatedCommands[index]))

// Cluster function static arrays
#define GENERATED_FUNCTION_ARRAYS   \
const EmberAfGenericClusterFunction chipFuncArrayBasicServer[] = {\
  (EmberAfGenericClusterFunction) emberAfBasicClusterServerInitCallback,\
};\
const EmberAfGenericClusterFunction chipFuncArrayOnOffServer[] = {\
  (EmberAfGenericClusterFunction) emberAfOnOffClusterServerInitCallback,\
};\



// clang-format off
#define GENERATED_COMMANDS { \
  /* Endpoint: 0, Cluster: OTA Software Update Requestor (server) */\
  /*   client_generated */ \
  0x00000000 /* AnnounceOtaProvider */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: General Commissioning (server) */\
  /*   client_generated */ \
  0x00000000 /* ArmFailSafe */, \
  0x00000002 /* SetRegulatoryConfig */, \
  0x00000004 /* CommissioningComplete */, \
  chip::kInvalidCommandId /* end of list */, \
  /*   server_generated */ \
  0x00000001 /* ArmFailSafeResponse */, \
  0x00000005 /* CommissioningCompleteResponse */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: Network Commissioning (server) */\
  /*   client_generated */ \
  0x00000000 /* ScanNetworks */, \
  0x00000003 /* AddOrUpdateThreadNetwork */, \
  0x00000004 /* RemoveNetwork */, \
  0x00000006 /* ConnectNetwork */, \
  0x00000008 /* ReorderNetwork */, \
  chip::kInvalidCommandId /* end of list */, \
  /*   server_generated */ \
  0x00000001 /* ScanNetworksResponse */, \
  0x00000007 /* ConnectNetworkResponse */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: Software Diagnostics (server) */\
  /*   client_generated */ \
  0x00000000 /* ResetWatermarks */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: Thread Network Diagnostics (server) */\
  /*   client_generated */ \
  0x00000000 /* ResetCounts */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: AdministratorCommissioning (server) */\
  /*   client_generated */ \
  0x00000000 /* OpenCommissioningWindow */, \
  0x00000001 /* OpenBasicCommissioningWindow */, \
  0x00000002 /* RevokeCommissioning */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 0, Cluster: Operational Credentials (server) */\
  /*   client_generated */ \
  0x00000000 /* AttestationRequest */, \
  0x00000002 /* CertificateChainRequest */, \
  0x00000004 /* CSRRequest */, \
  0x00000006 /* AddNOC */, \
  0x00000007 /* UpdateNOC */, \
  0x00000009 /* UpdateFabricLabel */, \
  0x0000000A /* RemoveFabric */, \
  0x0000000B /* AddTrustedRootCertificate */, \
  0x0000000C /* RemoveTrustedRootCertificate */, \
  chip::kInvalidCommandId /* end of list */, \
  /*   server_generated */ \
  0x00000001 /* AttestationResponse */, \
  0x00000003 /* CertificateChainResponse */, \
  0x00000005 /* CSRResponse */, \
  0x00000008 /* NOCResponse */, \
  chip::kInvalidCommandId /* end of list */, \
  /* Endpoint: 1, Cluster: On/Off (server) */\
  /*   client_generated */ \
  0x00000000 /* Off */, \
  0x00000001 /* On */, \
  0x00000002 /* Toggle */, \
  chip::kInvalidCommandId /* end of list */, \
}

// clang-format on

#define ZAP_CLUSTER_MASK(mask) CLUSTER_MASK_ ## mask
#define GENERATED_CLUSTER_COUNT 13


// clang-format off
#define GENERATED_CLUSTERS { \
  { \
      /* Endpoint: 0, Cluster: Descriptor (server) */ \
      .clusterId = 0x0000001D,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(0), \
      .attributeCount = 6, \
      .clusterSize = 6, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = nullptr ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: Basic (server) */ \
      .clusterId = 0x00000028,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(6), \
      .attributeCount = 13, \
      .clusterSize = 250, \
      .mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION), \
      .functions = chipFuncArrayBasicServer, \
      .acceptedCommandList = nullptr ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: OTA Software Update Provider (client) */ \
      .clusterId = 0x00000029,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(19), \
      .attributeCount = 0, \
      .clusterSize = 0, \
      .mask = ZAP_CLUSTER_MASK(CLIENT), \
      .functions = NULL, \
      .acceptedCommandList = nullptr ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: OTA Software Update Requestor (server) */ \
      .clusterId = 0x0000002A,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(19), \
      .attributeCount = 6, \
      .clusterSize = 9, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 0 ) ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: General Commissioning (server) */ \
      .clusterId = 0x00000030,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(25), \
      .attributeCount = 6, \
      .clusterSize = 14, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 2 ) ,\
      .generatedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 6 ) ,\
    },\
  { \
      /* Endpoint: 0, Cluster: Network Commissioning (server) */ \
      .clusterId = 0x00000031,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(31), \
      .attributeCount = 10, \
      .clusterSize = 48, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 9 ) ,\
      .generatedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 15 ) ,\
    },\
  { \
      /* Endpoint: 0, Cluster: General Diagnostics (server) */ \
      .clusterId = 0x00000033,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(41), \
      .attributeCount = 7, \
      .clusterSize = 6, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = nullptr ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: Software Diagnostics (server) */ \
      .clusterId = 0x00000034,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(48), \
      .attributeCount = 5, \
      .clusterSize = 6, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 18 ) ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: Thread Network Diagnostics (server) */ \
      .clusterId = 0x00000035,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(53), \
      .attributeCount = 64, \
      .clusterSize = 2, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 20 ) ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: AdministratorCommissioning (server) */ \
      .clusterId = 0x0000003C,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(117), \
      .attributeCount = 5, \
      .clusterSize = 10, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 22 ) ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 0, Cluster: Operational Credentials (server) */ \
      .clusterId = 0x0000003E,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(122), \
      .attributeCount = 8, \
      .clusterSize = 6, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 26 ) ,\
      .generatedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 36 ) ,\
    },\
  { \
      /* Endpoint: 1, Cluster: On/Off (server) */ \
      .clusterId = 0x00000006,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(130), \
      .attributeCount = 3, \
      .clusterSize = 7, \
      .mask = ZAP_CLUSTER_MASK(SERVER) | ZAP_CLUSTER_MASK(INIT_FUNCTION), \
      .functions = chipFuncArrayOnOffServer, \
      .acceptedCommandList = ZAP_GENERATED_COMMANDS_INDEX( 41 ) ,\
      .generatedCommandList = nullptr ,\
    },\
  { \
      /* Endpoint: 1, Cluster: Descriptor (server) */ \
      .clusterId = 0x0000001D,  \
      .attributes = ZAP_ATTRIBUTE_INDEX(133), \
      .attributeCount = 6, \
      .clusterSize = 6, \
      .mask = ZAP_CLUSTER_MASK(SERVER), \
      .functions = NULL, \
      .acceptedCommandList = nullptr ,\
      .generatedCommandList = nullptr ,\
    },\
}

// clang-format on

#define ZAP_CLUSTER_INDEX(index) (&generatedClusters[index])

#define ZAP_FIXED_ENDPOINT_DATA_VERSION_COUNT 12

// This is an array of EmberAfEndpointType structures.
#define GENERATED_ENDPOINT_TYPES { \
  { ZAP_CLUSTER_INDEX(0), 11, 357 }, \
  { ZAP_CLUSTER_INDEX(11), 2, 13 }, \
}



// Largest attribute size is needed for various buffers
#define ATTRIBUTE_LARGEST (255)

static_assert(ATTRIBUTE_LARGEST <= CHIP_CONFIG_MAX_ATTRIBUTE_STORE_ELEMENT_SIZE,
              "ATTRIBUTE_LARGEST larger than expected");

// Total size of singleton attributes
#define ATTRIBUTE_SINGLETONS_SIZE (250)

// Total size of attribute storage
#define ATTRIBUTE_MAX_SIZE (370)

// Number of fixed endpoints
#define FIXED_ENDPOINT_COUNT (2)

// Array of endpoints that are supported, the data inside
// the array is the endpoint number.
#define FIXED_ENDPOINT_ARRAY { 0x0000, 0x0001 }

// Array of profile ids
#define FIXED_PROFILE_IDS { 0x0103, 0x0103 }

// Array of device ids
#define FIXED_DEVICE_IDS { 22, 10 }

// Array of device versions
#define FIXED_DEVICE_VERSIONS { 1, 0 }

// Array of endpoint types supported on each endpoint
#define FIXED_ENDPOINT_TYPES { 0, 1 }

// Array of networks supported on each endpoint
#define FIXED_NETWORKS { 0, 0 }

