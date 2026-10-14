/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "OverdriveCatalog.h"
#include "windows/GUIMediaWindow.h"

class CGUIWindowOverdrive final : public CGUIMediaWindow
{
public:
  CGUIWindowOverdrive();
  std::string GetRootPath() override { return "overdrive://root/"; }

protected:
  void OnInitWindow() override;
  bool GetDirectory(const std::string& path, CFileItemList& items) override;
  bool OnSelect(int item) override;

private:
  OVERDRIVE::Catalog m_catalog;
  bool m_loaded{false};
};
