#include "core/stable_order.h"

#include <gtest/gtest.h>

namespace throwback {
namespace {

using Order = std::vector<int>;

TEST(StableOrderTest, KeepsExistingOrderWhenInputIsReordered) {
  EXPECT_EQ(MergeStableOrder(Order{1, 2, 3}, Order{3, 1, 2}), (Order{1, 2, 3}));
}

TEST(StableOrderTest, DropsClosedItems) {
  EXPECT_EQ(MergeStableOrder(Order{1, 2, 3}, Order{3, 1}), (Order{1, 3}));
}

TEST(StableOrderTest, AppendsNewItemsInDiscoveryOrder) {
  EXPECT_EQ(MergeStableOrder(Order{1, 2}, Order{5, 2, 4, 1}), (Order{1, 2, 5, 4}));
}

TEST(StableOrderTest, IgnoresDuplicates) {
  EXPECT_EQ(MergeStableOrder(Order{1, 1}, Order{2, 1, 2}), (Order{1, 2}));
}

}  // namespace
}  // namespace throwback
