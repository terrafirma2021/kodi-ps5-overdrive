/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "OverdriveCatalog.h"

#include "filesystem/File.h"
#include "utils/JSONVariantParser.h"
#include "utils/Variant.h"
#include "utils/log.h"

#include <unordered_set>
#include <utility>
#include <vector>

namespace OVERDRIVE
{
bool Catalog::Load(const std::string& path)
{
  std::vector<uint8_t> bytes;
  XFILE::CFile file;
  if (file.LoadFile(path, bytes) < 0)
  {
    CLog::Log(LOGERROR, "Overdrive: cannot read catalog {}", path);
    return false;
  }

  CVariant document;
  if (!CJSONVariantParser::Parse(std::string(bytes.begin(), bytes.end()), document) ||
      !document.isObject() || document["schema"].asInteger() != 1 ||
      !document["titles"].isArray())
  {
    CLog::Log(LOGERROR, "Overdrive: invalid catalog {}", path);
    return false;
  }

  std::vector<Title> parsed;
  std::unordered_set<std::string> ids;
  const CVariant& entries = document["titles"];
  for (auto it = entries.begin_array(); it != entries.end_array(); ++it)
  {
    const CVariant& entry = *it;
    if (!entry.isObject())
      continue;
    Title title{entry["id"].asString(),
                entry["title"].asString(),
                entry["platform"].asString(),
                entry["contentKind"].asString(),
                entry["developer"].asString(),
                entry["summary"].asString(),
                entry["cover"].asString()};
    if (title.id.empty() || title.name.empty() || !ids.insert(title.id).second)
      continue;
    parsed.emplace_back(std::move(title));
  }

  m_titles = std::move(parsed);
  CLog::Log(LOGINFO, "Overdrive: loaded {} catalog titles", m_titles.size());
  return true;
}

const Title* Catalog::Find(const std::string& id) const
{
  for (const Title& title : m_titles)
  {
    if (title.id == id)
      return &title;
  }
  return nullptr;
}
} // namespace OVERDRIVE
