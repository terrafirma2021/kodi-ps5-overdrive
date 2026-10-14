/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <string>
#include <vector>

namespace OVERDRIVE
{
struct Title
{
  std::string id;
  std::string name;
  std::string platform;
  std::string kind;
  std::string developer;
  std::string summary;
  std::string cover;
};

class Catalog
{
public:
  bool Load(const std::string& path);
  const std::vector<Title>& Titles() const { return m_titles; }
  const Title* Find(const std::string& id) const;

private:
  std::vector<Title> m_titles;
};
} // namespace OVERDRIVE
