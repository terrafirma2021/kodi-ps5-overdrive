/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "NetworkPS5.h"

#include <array>
#include <cstdio>
#include <cstring>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef AF_LINK
#include <net/if_dl.h>
#endif

extern "C" int sceNetCtlGetInfo(int selector, void* info);
extern "C" int sceNetCtlGetState(int* state);

namespace
{

// sceNetCtlGetInfo writes the selected value at offset zero of a 256-byte
// union. These selectors are the PS5's route and DNS entries. Keep the call
// read-only and validate the returned text before presenting it in the GUI.
constexpr int NET_CTL_IP_ADDRESS = 14;
constexpr int NET_CTL_NETMASK = 15;
constexpr int NET_CTL_DEFAULT_ROUTE = 16;
constexpr int NET_CTL_PRIMARY_DNS = 17;
constexpr int NET_CTL_SECONDARY_DNS = 18;
constexpr int NET_CTL_STATE_IP_OBTAINED = 3;
constexpr int NET_CTL_ETHER_ADDR = 2;

std::string NetCtlIpv4(int selector)
{
  alignas(4) std::array<char, 256> info{};
  if (sceNetCtlGetInfo(selector, info.data()) != 0)
    return {};
  const size_t length = strnlen(info.data(), info.size());
  if (length == 0 || length >= INET_ADDRSTRLEN)
    return {};
  in_addr address{};
  if (inet_pton(AF_INET, info.data(), &address) != 1 || address.s_addr == INADDR_ANY)
    return {};
  return std::string(info.data(), length);
}

bool IsUsable(const struct ifaddrs* ifa)
{
  return ifa->ifa_addr && (ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_LOOPBACK);
}

std::string Ipv4ToString(const struct sockaddr* sa)
{
  if (!sa || sa->sa_family != AF_INET)
    return {};
  char buf[INET_ADDRSTRLEN] = {};
  const auto* sin = reinterpret_cast<const struct sockaddr_in*>(sa);
  if (!inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf)))
    return {};
  return buf;
}

// First usable IPv4 address (and netmask) of a non-loopback interface.
bool FindIpv4(std::string& address, std::string& netmask)
{
  struct ifaddrs* list = nullptr;
  if (getifaddrs(&list) != 0 || !list)
    return false;

  bool found = false;
  for (const struct ifaddrs* ifa = list; ifa; ifa = ifa->ifa_next)
  {
    if (!IsUsable(ifa) || ifa->ifa_addr->sa_family != AF_INET)
      continue;
    address = Ipv4ToString(ifa->ifa_addr);
    netmask = Ipv4ToString(ifa->ifa_netmask);
    if (!address.empty())
    {
      found = true;
      break;
    }
  }
  freeifaddrs(list);
  return found;
}

} // namespace

std::unique_ptr<CNetworkBase> CNetworkBase::GetNetwork()
{
  return std::make_unique<CNetworkPS5>();
}

bool CNetworkInterfacePS5::IsConnected() const
{
  int state = 0;
  if (sceNetCtlGetState(&state) == 0 && state == NET_CTL_STATE_IP_OBTAINED)
    return true;
  if (!NetCtlIpv4(NET_CTL_IP_ADDRESS).empty())
    return true;
  std::string address, netmask;
  return FindIpv4(address, netmask);
}

std::string CNetworkInterfacePS5::GetMacAddress() const
{
  char raw[6] = {};
  GetMacAddressRaw(raw);
  const char empty[6] = {};
  if (std::memcmp(raw, empty, sizeof(raw)) == 0)
    return {};
  char buf[18];
  std::snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
                static_cast<unsigned char>(raw[0]), static_cast<unsigned char>(raw[1]),
                static_cast<unsigned char>(raw[2]), static_cast<unsigned char>(raw[3]),
                static_cast<unsigned char>(raw[4]), static_cast<unsigned char>(raw[5]));
  return buf;
}

void CNetworkInterfacePS5::GetMacAddressRaw(char rawMac[6]) const
{
  std::memset(rawMac, 0, 6);
  alignas(4) std::array<char, 256> info{};
  const char empty[6] = {};
  if (sceNetCtlGetInfo(NET_CTL_ETHER_ADDR, info.data()) == 0 &&
      std::memcmp(info.data(), empty, sizeof(empty)) != 0)
  {
    std::memcpy(rawMac, info.data(), 6);
    return;
  }
#ifdef AF_LINK
  struct ifaddrs* list = nullptr;
  if (getifaddrs(&list) != 0 || !list)
    return;
  for (const struct ifaddrs* ifa = list; ifa; ifa = ifa->ifa_next)
  {
    if (!IsUsable(ifa) || ifa->ifa_addr->sa_family != AF_LINK)
      continue;
    const auto* sdl = reinterpret_cast<const struct sockaddr_dl*>(ifa->ifa_addr);
    if (sdl->sdl_alen == 6)
    {
      std::memcpy(rawMac, LLADDR(sdl), 6);
      break;
    }
  }
  freeifaddrs(list);
#endif
}

std::string CNetworkInterfacePS5::GetCurrentIPAddress() const
{
  const std::string netCtlAddress = NetCtlIpv4(NET_CTL_IP_ADDRESS);
  if (!netCtlAddress.empty())
    return netCtlAddress;
  std::string address, netmask;
  if (FindIpv4(address, netmask))
    return address;
  return {};
}

std::string CNetworkInterfacePS5::GetCurrentNetmask() const
{
  const std::string netCtlMask = NetCtlIpv4(NET_CTL_NETMASK);
  if (!netCtlMask.empty())
    return netCtlMask;
  std::string address, netmask;
  if (FindIpv4(address, netmask) && !netmask.empty())
    return netmask;
  return {};
}

std::string CNetworkInterfacePS5::GetCurrentDefaultGateway() const
{
  return NetCtlIpv4(NET_CTL_DEFAULT_ROUTE);
}

CNetworkPS5::CNetworkPS5() : CNetworkBase()
{
  m_interfaces.push_back(&m_iface);
}

CNetworkPS5::~CNetworkPS5() = default;

std::vector<CNetworkInterface*>& CNetworkPS5::GetInterfaceList()
{
  return m_interfaces;
}

bool CNetworkPS5::PingHost(unsigned long host, unsigned int timeout_ms)
{
  // No raw ICMP sockets for an unprivileged title.
  return false;
}

std::vector<std::string> CNetworkPS5::GetNameServers()
{
  std::vector<std::string> servers;
  const std::string primary = NetCtlIpv4(NET_CTL_PRIMARY_DNS);
  const std::string secondary = NetCtlIpv4(NET_CTL_SECONDARY_DNS);
  if (!primary.empty())
    servers.push_back(primary);
  if (!secondary.empty() && secondary != primary)
    servers.push_back(secondary);
  return servers;
}
