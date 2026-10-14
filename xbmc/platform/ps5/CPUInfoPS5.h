/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "platform/posix/CPUInfoPosix.h"

class CCPUInfoPS5 : public CCPUInfoPosix
{
public:
  CCPUInfoPS5();
  ~CCPUInfoPS5() = default;

  bool SupportsCPUUsage() const override { return false; }
  int GetUsedPercentage() override { return 0; }
  // The PS5 clock is dynamic. No title-safe live clock query is available.
  float GetCPUFrequency() override { return 0.0f; }
};
