#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace throwback {

// One entry of "All Programs": a program or a folder of them.
struct Program {
  std::wstring name;
  // What to launch, and the parsing name its icon is read from.
  std::wstring target;
  std::vector<std::unique_ptr<Program>> children;
  bool folder = false;
  // Newly installed and not started yet (for a folder: holds such a
  // program), which Windows 7 highlights.
  bool fresh = false;
};

// Everything the start menu can launch, read from the Start Menu folders
// (current user and all users, merged like Explorer does) and the packaged
// apps in shell:AppsFolder.
struct ProgramCatalog {
  Program root;
  // Every launchable program once, for search.
  std::vector<const Program*> all;
  // Whether any program is fresh, which lights up "All Programs".
  bool hasFresh = false;
  // App ID (as Windows' usage history records it) to display name.
  std::unordered_map<std::wstring, std::wstring> appNames;
};

std::unique_ptr<ProgramCatalog> LoadPrograms();

}  // namespace throwback
