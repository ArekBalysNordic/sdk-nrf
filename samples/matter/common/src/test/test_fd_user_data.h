/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <platform/nrfconnect/FactoryDataProvider.h>

namespace Nrf::Matter
{
void ParseUserDataForTestingPurposes(chip::DeviceLayer::FactoryDataProviderBase *provider);
} // namespace Nrf::Matter