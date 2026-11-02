#include "core/monitor_diff.h"

#include <algorithm>

namespace throwback {

MonitorDiff DiffMonitors(const std::vector<std::wstring>& current,
                         const std::vector<std::wstring>& wanted) {
  MonitorDiff diff;
  for (const auto& name : wanted) {
    if (std::ranges::find(current, name) == current.end()) diff.added.push_back(name);
  }
  for (const auto& name : current) {
    if (std::ranges::find(wanted, name) == wanted.end()) diff.removed.push_back(name);
  }
  return diff;
}

}  // namespace throwback
