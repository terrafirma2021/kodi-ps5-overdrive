/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "GUIWindowOverdrive.h"

#include "FileItem.h"
#include "FileItemList.h"
#include "dialogs/GUIDialogOK.h"
#include "guilib/WindowIDs.h"
#include "utils/Variant.h"

#include <memory>
#include <string>

namespace
{
constexpr const char* ROOT = "overdrive://root/";
constexpr const char* GAMES = "overdrive://games/";
constexpr const char* HOMEBREW = "overdrive://homebrew/";

void AddCategory(CFileItemList& items, const char* path, const char* label)
{
  auto item = std::make_shared<CFileItem>(path, true);
  item->SetLabel(label);
  item->SetLabelPreformatted(true);
  items.Add(std::move(item));
}
} // namespace

CGUIWindowOverdrive::CGUIWindowOverdrive() : CGUIMediaWindow(WINDOW_OVERDRIVE, "Overdrive.xml")
{
}

void CGUIWindowOverdrive::OnInitWindow()
{
  // A catalog dropped into the profile takes effect the next time this window opens.
  m_loaded = false;
  CGUIMediaWindow::OnInitWindow();
}

bool CGUIWindowOverdrive::GetDirectory(const std::string& path, CFileItemList& items)
{
  if (!m_loaded)
  {
    m_loaded = true;
    if (!m_catalog.Load("special://home/overdrive/catalog.json"))
      m_catalog.Load("special://xbmc/system/overdrive/catalog.json");
  }

  const std::string directory = path.empty() ? ROOT : path;
  items.SetPath(directory);
  if (directory == ROOT)
  {
    items.SetLabel("Overdrive");
    AddCategory(items, GAMES, "Games");
    AddCategory(items, HOMEBREW, "Homebrew");
    return true;
  }

  if (directory != GAMES && directory != HOMEBREW)
    return false;

  items.SetLabel(directory == GAMES ? "Games" : "Homebrew");
  items.SetContent("games");
  for (const OVERDRIVE::Title& title : m_catalog.Titles())
  {
    const bool homebrew = title.kind == "homebrew";
    if (homebrew != (directory == HOMEBREW))
      continue;

    auto item = std::make_shared<CFileItem>("overdrive://title/" + title.id, false);
    item->SetLabel(title.name);
    item->SetLabel2(title.developer);
    item->SetLabelPreformatted(true);
    item->SetProperty("overdrive.id", title.id);
    item->SetProperty("overdrive.platform", title.platform);
    if (!title.cover.empty())
      item->SetArt("thumb", title.cover);
    items.Add(std::move(item));
  }
  return true;
}

bool CGUIWindowOverdrive::OnSelect(int index)
{
  if (index < 0 || index >= m_vecItems->Size())
    return false;

  const CFileItemPtr item = m_vecItems->Get(index);
  if (item->IsFolder())
    return CGUIMediaWindow::OnSelect(index);

  const OVERDRIVE::Title* title = m_catalog.Find(item->GetProperty("overdrive.id").asString());
  if (title == nullptr)
    return false;

  const std::string details = title->summary.empty()
                                  ? title->platform + " | " + title->developer
                                  : title->summary;
  CGUIDialogOK::ShowAndGetInput(CVariant{title->name}, CVariant{details});
  return true;
}
