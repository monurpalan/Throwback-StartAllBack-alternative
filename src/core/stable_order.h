#pragma once

#include <unordered_set>
#include <vector>

namespace throwback {

// Keeps items that still exist in their previous order, drops the ones that
// disappeared and appends new ones in discovery order. This stops taskbar
// buttons from jumping around when the window Z-order changes.
template <typename T>
std::vector<T> MergeStableOrder(const std::vector<T>& previous, const std::vector<T>& current) {
  const std::unordered_set<T> alive(current.begin(), current.end());
  std::unordered_set<T> placed;
  std::vector<T> result;
  result.reserve(current.size());
  for (const T& item : previous) {
    if (alive.contains(item) && placed.insert(item).second) result.push_back(item);
  }
  for (const T& item : current) {
    if (placed.insert(item).second) result.push_back(item);
  }
  return result;
}

}  // namespace throwback
