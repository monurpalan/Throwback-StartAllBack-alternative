#pragma once

#include <string>
#include <vector>

namespace throwback {

// Which monitors gained or lost a bar, by device name.
struct MonitorDiff {
  std::vector<std::wstring> added;
  std::vector<std::wstring> removed;
};

// Compares the monitors that have a bar with the ones that should.
MonitorDiff DiffMonitors(const std::vector<std::wstring>& current,
                         const std::vector<std::wstring>& wanted);

}  // namespace throwback
