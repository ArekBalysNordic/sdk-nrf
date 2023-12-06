/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>

enum class EventSource : uint8_t { Undefined, Application, System };

namespace TaskExecutor
{
using Task = std::function<void()>;

void PostTask(const Task& task);
void DispatchNextTask();
}
