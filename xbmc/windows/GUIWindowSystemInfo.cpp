/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIWindowSystemInfo.h"

#include "GUIInfoManager.h"
#include "ServiceBroker.h"
#include "guilib/GUIComponent.h"
#include "guilib/GUIMessage.h"
#include "guilib/WindowIDs.h"
#include "guilib/guiinfo/GUIInfoLabels.h"
#include "pvr/PVRManager.h"
#include "rendering/RenderSystem.h"
#include "resources/LocalizeStrings.h"
#include "resources/ResourcesComponent.h"
#include "storage/MediaManager.h"
#include "utils/CPUInfo.h"
#include "utils/StringUtils.h"
#include "utils/SystemInfo.h"
#include "windowing/WinSystem.h"

#if defined(TARGET_PS5)
#include "network/Network.h"
#include "platform/ps5/VideoOutInfo.h"
#endif

constexpr int CONTROL_TEXT_START = 2;
constexpr int CONTROL_TEXT_END = 13; // 12 lines

#define CONTROL_TB_POLICY   30
#define CONTROL_BT_STORAGE  94
#define CONTROL_BT_DEFAULT  95
#define CONTROL_BT_NETWORK  96
#define CONTROL_BT_VIDEO    97
#define CONTROL_BT_HARDWARE 98
#define CONTROL_BT_PVR      99
#define CONTROL_BT_POLICY   100

#if defined(TARGET_PS5)
namespace
{
std::string Ps5DisplayValue(std::string value)
{
  const std::string busy = CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(503);
  return value.empty() || value == busy ? "-" : value;
}
}
#endif

constexpr int CONTROL_BT_DONATE = 101;
constexpr int CONTROL_GROUP_DONATE = 102;
constexpr int CONTROL_MULTI_IMAGE_DONATE = 103;

constexpr int CONTROL_GROUP_SYSTEM_BAR = 104;

constexpr int CONTROL_START = CONTROL_BT_STORAGE;
constexpr int CONTROL_END = CONTROL_BT_DONATE;

CGUIWindowSystemInfo::CGUIWindowSystemInfo(void) :
    CGUIWindow(WINDOW_SYSTEM_INFORMATION, "SettingsSystemInfo.xml")
{
  m_section = CONTROL_BT_DEFAULT;
  m_loadType = KEEP_IN_MEMORY;
}

CGUIWindowSystemInfo::~CGUIWindowSystemInfo(void) = default;

bool CGUIWindowSystemInfo::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_WINDOW_INIT:
    {
      CGUIWindow::OnMessage(message);
      SET_CONTROL_LABEL(52, CSysInfo::GetAppName() + " " + CSysInfo::GetVersion());
      SET_CONTROL_LABEL(53, CSysInfo::GetBuildDate());
      CONTROL_ENABLE_ON_CONDITION(CONTROL_BT_PVR, CServiceBroker::GetPVRManager().IsStarted());
      return true;
    }
    break;

    case GUI_MSG_WINDOW_DEINIT:
    {
      CGUIWindow::OnMessage(message);
      m_diskUsage.clear();
      m_privacyPolicyLoaded = false;
      return true;
    }
    break;

    case GUI_MSG_FOCUSED:
    {
      CGUIWindow::OnMessage(message);
      int focusedControl = GetFocusedControlID();
      if (m_section != focusedControl && focusedControl >= CONTROL_START && focusedControl <= CONTROL_END)
      {
        ResetLabels();
        m_section = focusedControl;
#if defined(TARGET_PS5)
        if (m_section == CONTROL_BT_STORAGE)
          m_diskUsage.clear();
#endif
      }
      if (m_section >= CONTROL_BT_STORAGE && m_section <= CONTROL_BT_PVR)
      {
        SET_CONTROL_HIDDEN(CONTROL_TB_POLICY);
        SET_CONTROL_HIDDEN(CONTROL_GROUP_DONATE);
        SET_CONTROL_VISIBLE(CONTROL_GROUP_SYSTEM_BAR);
      }
      else if (m_section == CONTROL_BT_POLICY)
      {
        LoadPrivacyPolicy();
        SET_CONTROL_VISIBLE(CONTROL_TB_POLICY);
        SET_CONTROL_HIDDEN(CONTROL_GROUP_DONATE);
        SET_CONTROL_VISIBLE(CONTROL_GROUP_SYSTEM_BAR);
      }
      else if (m_section == CONTROL_BT_DONATE)
      {
        SET_CONTROL_HIDDEN(CONTROL_TB_POLICY);
        SET_CONTROL_VISIBLE(CONTROL_GROUP_DONATE);
        SET_CONTROL_HIDDEN(CONTROL_GROUP_SYSTEM_BAR);
      }
      return true;
    }
    break;
  }
  return CGUIWindow::OnMessage(message);
}

void CGUIWindowSystemInfo::FrameMove()
{
  int i = CONTROL_TEXT_START;
  if (m_section == CONTROL_BT_DEFAULT)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(20154));
#if defined(TARGET_PS5)
    SET_CONTROL_LABEL(i++, "Platform: PlayStation 5");
    SET_CONTROL_LABEL(i++, "Memory: Kodi title heap");
#endif
    SetControlLabel(i++, "{}: {}", 158, SYSTEM_FREE_MEMORY);
#if defined(TARGET_PS5)
    CNetworkInterface* summaryIface = CServiceBroker::GetNetwork().GetFirstConnectedInterface();
    SET_CONTROL_LABEL(i++, StringUtils::Format("IP address: {}",
                      summaryIface ? Ps5DisplayValue(summaryIface->GetCurrentIPAddress()) : "-"));
#else
    SetControlLabel(i++, "{}: {}", 150, NETWORK_IP_ADDRESS);
#endif
    SetControlLabel(i++, "{} {}", 13287, SYSTEM_SCREEN_RESOLUTION);
    SetControlLabel(i++, "{} {}", 13283, SYSTEM_OS_VERSION_INFO);
    SetControlLabel(i++, "{}: {}", 12390, SYSTEM_UPTIME);
    SetControlLabel(i++, "{}: {}", 12394, SYSTEM_TOTALUPTIME);
#if defined(TARGET_PS5)
    SET_CONTROL_LABEL(i++, "Controller battery: -");
#else
    SetControlLabel(i++, "{}: {}", 12395, SYSTEM_BATTERY_LEVEL);
#endif
  }

  else if (m_section == CONTROL_BT_STORAGE)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(20155));
    if (m_diskUsage.empty())
      m_diskUsage = CServiceBroker::GetMediaManager().GetDiskUsage();

#if defined(TARGET_PS5)
    if (m_diskUsage.empty())
      m_diskUsage.emplace_back("Storage: -");
#endif

    for (size_t d = 0; d < m_diskUsage.size() && d <= CONTROL_TEXT_END - CONTROL_TEXT_START; ++d)
    {
      SET_CONTROL_LABEL(i++, m_diskUsage[d]);
    }
  }

  else if (m_section == CONTROL_BT_NETWORK)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(20158));
#if defined(TARGET_PS5)
    auto& network = CServiceBroker::GetNetwork();
    CNetworkInterface* iface = network.GetFirstConnectedInterface();
    const std::vector<std::string> dns = network.GetNameServers();
    SET_CONTROL_LABEL(i++, StringUtils::Format("Local network: {}", iface ? "IP obtained" : "Disconnected"));
    SET_CONTROL_LABEL(i++, StringUtils::Format("MAC address: {}", iface ? Ps5DisplayValue(iface->GetMacAddress()) : "-"));
    SET_CONTROL_LABEL(i++, StringUtils::Format("IP address: {}", iface ? Ps5DisplayValue(iface->GetCurrentIPAddress()) : "-"));
    SET_CONTROL_LABEL(i++, StringUtils::Format("Subnet mask: {}", iface ? Ps5DisplayValue(iface->GetCurrentNetmask()) : "-"));
    SET_CONTROL_LABEL(i++, StringUtils::Format("Gateway: {}", iface ? Ps5DisplayValue(iface->GetCurrentDefaultGateway()) : "-"));
    SET_CONTROL_LABEL(i++, StringUtils::Format("DNS 1: {}", dns.empty() ? "-" : dns[0]));
    SET_CONTROL_LABEL(i++, StringUtils::Format("DNS 2: {}", dns.size() < 2 ? "-" : dns[1]));
    SET_CONTROL_LABEL(i++, "Internet: not tested");
#else
    SET_CONTROL_LABEL(i++, CServiceBroker::GetGUI()->GetInfoManager().GetLabel(
                               NETWORK_LINK_STATE, INFO::DEFAULT_CONTEXT));
    SetControlLabel(i++, "{}: {}", 149, NETWORK_MAC_ADDRESS);
    SetControlLabel(i++, "{}: {}", 150, NETWORK_IP_ADDRESS);
    SetControlLabel(i++, "{}: {}", 13159, NETWORK_SUBNET_MASK);
    SetControlLabel(i++, "{}: {}", 13160, NETWORK_GATEWAY_ADDRESS);
    SetControlLabel(i++, "{}: {}", 13161, NETWORK_DNS1_ADDRESS);
    SetControlLabel(i++, "{}: {}", 20307, NETWORK_DNS2_ADDRESS);
    SetControlLabel(i++, "{} {}", 13295, SYSTEM_INTERNET_STATE);
#endif
  }

  else if (m_section == CONTROL_BT_VIDEO)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(20159));
#if defined(TARGET_PS5)
    unsigned outputWidth = 0;
    unsigned outputHeight = 0;
    if (KODI::PLATFORM::PS5::QuerySystemResolution(outputWidth, outputHeight))
      SET_CONTROL_LABEL(i++, StringUtils::Format("PS5 output: {} x {}", outputWidth, outputHeight));
    else
      SET_CONTROL_LABEL(i++, "PS5 output: -");
    const float refresh = KODI::PLATFORM::PS5::QueryRefreshRate();
    if (refresh > 0.0f)
      SET_CONTROL_LABEL(i++, StringUtils::Format("Output refresh: {:.2f} Hz", refresh));
    else
      SET_CONTROL_LABEL(i++, "Output refresh: -");
    if (KODI::PLATFORM::PS5::VideoOutHandle() >= 0)
    {
      SET_CONTROL_LABEL(i++, StringUtils::Format("Output range: {}",
                                                KODI::PLATFORM::PS5::IsDisplayHdr() ? "HDR" : "SDR"));
      SET_CONTROL_LABEL(i++, StringUtils::Format("High refresh preset: {}",
                           KODI::PLATFORM::PS5::IsHighRefreshSupported() ? "available" : "unavailable"));
    }
    else
    {
      SET_CONTROL_LABEL(i++, "Output range: -");
      SET_CONTROL_LABEL(i++, "High refresh preset: -");
    }
#endif
    SET_CONTROL_LABEL(i++, CServiceBroker::GetGUI()->GetInfoManager().GetLabel(
                               SYSTEM_VIDEO_ENCODER_INFO, INFO::DEFAULT_CONTEXT));
    SetControlLabel(i++, "{} {}", 13287, SYSTEM_SCREEN_RESOLUTION);

    auto renderingSystem = CServiceBroker::GetRenderSystem();
    if (renderingSystem)
    {
      static std::string vendor = renderingSystem->GetRenderVendor();
      if (!vendor.empty())
        SET_CONTROL_LABEL(
            i++,
            StringUtils::Format(
                "{} {}", CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(22007),
                vendor));

#if defined(HAS_DX)
      int renderVersionLabel = 22024;
#else
      int renderVersionLabel = 22009;
#endif
      static std::string version = renderingSystem->GetRenderVersionString();
      if (!version.empty())
        SET_CONTROL_LABEL(i++, StringUtils::Format(
                                   "{} {}",
                                   CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(
                                       renderVersionLabel),
                                   version));
    }

    auto windowSystem = CServiceBroker::GetWinSystem();
    if (windowSystem)
    {
      const std::string platform = windowSystem->GetName();
      if (platform != "platform default")
        SET_CONTROL_LABEL(
            i++,
            StringUtils::Format(
                "{} {}", CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(39153),
                platform));
    }

#if !defined(TARGET_PS5)
    SetControlLabel(i++, "{} {}", 22010, SYSTEM_GPU_TEMPERATURE);
#endif

    const std::string hdrTypes = CServiceBroker::GetGUI()->GetInfoManager().GetLabel(
        SYSTEM_SUPPORTED_HDR_TYPES, INFO::DEFAULT_CONTEXT);
    SET_CONTROL_LABEL(
        i++,
        StringUtils::Format(
            "{}: {}", CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(39174),
            hdrTypes.empty() ? CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(231)
                             : hdrTypes));
  }

  else if (m_section == CONTROL_BT_HARDWARE)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(20160));

    auto cpuInfo = CServiceBroker::GetCPUInfo();
    if (cpuInfo)
    {
      static std::string model = cpuInfo->GetCPUModel();
      if (!model.empty())
        SET_CONTROL_LABEL(i++, "CPU: " + model);
#if defined(TARGET_PS5)
      else
        SET_CONTROL_LABEL(i++, "CPU: -");
#endif

#if defined(TARGET_PS5)
      SET_CONTROL_LABEL(i++, StringUtils::Format("Visible CPU cores: {}", cpuInfo->GetCPUCount()));
#endif

      static std::string mips = cpuInfo->GetCPUBogoMips();
      if (!mips.empty())
        SET_CONTROL_LABEL(i++, "BogoMips: " + mips);

      static std::string soc = cpuInfo->GetCPUSoC();
      if (!soc.empty())
        SET_CONTROL_LABEL(i++, "SoC: " + soc);

      static std::string hardware = cpuInfo->GetCPUHardware();
      if (!hardware.empty())
        SET_CONTROL_LABEL(i++, "Hardware: " + hardware);

      static std::string revision = cpuInfo->GetCPURevision();
      if (!revision.empty())
        SET_CONTROL_LABEL(i++, "Revision: " + revision);

      static std::string serial = cpuInfo->GetCPUSerial();
      if (!serial.empty())
        SET_CONTROL_LABEL(i++, "Serial: " + serial);

      // temperature can't really be conditional because of localization units
#if !defined(TARGET_PS5)
      SetControlLabel(i++, "{} {}", 22011, SYSTEM_CPU_TEMPERATURE);
#endif

      // we can check if the cpufrequency is not 0 (default if not implemented)
      // but we have to call through CGUIInfoManager -> CSystemGUIInfo -> CSysInfo
      // to limit the frequency of updates
      static float cpuFreq = cpuInfo->GetCPUFrequency();
      if (cpuFreq > 0)
        SetControlLabel(i++, "{} {}", 13284, SYSTEM_CPUFREQUENCY);
    }
  }

  else if (m_section == CONTROL_BT_PVR)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(19166));
    int i = CONTROL_TEXT_START;

    SetControlLabel(i++, "{}: {}", 19120, PVR_BACKEND_NUMBER);
    i++;  // empty line
    SetControlLabel(i++, "{}: {}", 19012, PVR_BACKEND_NAME);
    SetControlLabel(i++, "{}: {}", 19114, PVR_BACKEND_VERSION);
    SetControlLabel(i++, "{}: {}", 19115, PVR_BACKEND_HOST);
    SetControlLabel(i++, "{}: {}", 19116, PVR_BACKEND_DISKSPACE);
    SetControlLabel(i++, "{}: {}", 19334, PVR_BACKEND_PROVIDERS);
    SetControlLabel(i++, "{}: {}", 19042, PVR_BACKEND_CHANNEL_GROUPS);
    SetControlLabel(i++, "{}: {}", 19019, PVR_BACKEND_CHANNELS);
    SetControlLabel(i++, "{}: {}", 19163, PVR_BACKEND_RECORDINGS);
    SetControlLabel(i++, "{}: {}", 19168,
                    PVR_BACKEND_DELETED_RECORDINGS); // Deleted and recoverable recordings
    SetControlLabel(i++, "{}: {}", 19025, PVR_BACKEND_TIMERS);
  }

  else if (m_section == CONTROL_BT_POLICY)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(12389));
  }
  else if (m_section == CONTROL_BT_DONATE)
  {
    SET_CONTROL_LABEL(40, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(12396));
  }
  CGUIWindow::FrameMove();
}

void CGUIWindowSystemInfo::ResetLabels()
{
  for (int i = CONTROL_TEXT_START; i <= CONTROL_TEXT_END; ++i)
  {
    SET_CONTROL_LABEL(i, "");
  }

  // Reset the multiimage to the beginning
  CGUIMessage msg{GUI_MSG_RESET_MULTI_IMAGE, GetID(), CONTROL_MULTI_IMAGE_DONATE};
  OnMessage(msg);
}

void CGUIWindowSystemInfo::SetControlLabel(int id, const char *format, int label, int info)
{
  std::string value = CServiceBroker::GetGUI()->GetInfoManager().GetLabel(info, INFO::DEFAULT_CONTEXT);
#if defined(TARGET_PS5)
  value = Ps5DisplayValue(value);
#endif
  std::string tmpStr = StringUtils::Format(
      format, CServiceBroker::GetResourcesComponent().GetLocalizeStrings().Get(label),
      value);
  SET_CONTROL_LABEL(id, tmpStr);
}

void CGUIWindowSystemInfo::LoadPrivacyPolicy()
{
  if (!m_privacyPolicyLoaded)
  {
    m_privacyPolicyLoaded = true;
    SET_CONTROL_LABEL(CONTROL_TB_POLICY, CServiceBroker::GetGUI()->GetInfoManager().GetLabel(
                                             SYSTEM_PRIVACY_POLICY, INFO::DEFAULT_CONTEXT));
  }
}
