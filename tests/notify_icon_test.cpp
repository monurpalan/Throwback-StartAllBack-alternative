#include "core/notify_icon.h"

#include <gtest/gtest.h>

#include <cstring>

namespace throwback {
namespace {

// Builds the bytes Shell_NotifyIcon sends, field by field.
class Packet {
 public:
  explicit Packet(std::uint32_t command, std::uint32_t size = 956) : bytes_(8 + 956) {
    Put(0, 0x34753423u);
    Put(4, command);
    Put(8, size);
  }
  Packet& Window(std::uint32_t value) { return Put(12, value); }
  Packet& Id(std::uint32_t value) { return Put(16, value); }
  Packet& Flags(std::uint32_t value) { return Put(20, value); }
  Packet& Callback(std::uint32_t value) { return Put(24, value); }
  Packet& Icon(std::uint32_t value) { return Put(28, value); }
  Packet& Tip(const std::wstring& text) {
    std::memcpy(bytes_.data() + 32, text.c_str(), (text.size() + 1) * 2);
    return *this;
  }
  Packet& State(std::uint32_t state, std::uint32_t mask) {
    Put(288, state);
    return Put(292, mask);
  }
  Packet& Version(std::uint32_t value) { return Put(808, value); }
  Packet& Guid(std::uint8_t first) {
    bytes_[944] = static_cast<std::byte>(first);
    return *this;
  }
  std::span<const std::byte> Bytes() const { return bytes_; }
  NotifyIconMessage Parse() const { return *ParseNotifyIconData(bytes_); }

 private:
  Packet& Put(size_t offset, std::uint32_t value) {
    std::memcpy(bytes_.data() + offset, &value, 4);
    return *this;
  }
  std::vector<std::byte> bytes_;
};

constexpr std::uint32_t kAdd = 0, kModify = 1, kDelete = 2, kSetVersion = 4;
constexpr std::uint32_t kMessage = 1, kIconFlag = 2, kTipFlag = 4, kStateFlag = 8, kGuidFlag = 0x20;

TEST(NotifyIconTest, ParsesFields) {
  const NotifyIconMessage message = Packet(kAdd)
                                        .Window(0x1234)
                                        .Id(7)
                                        .Flags(kMessage | kIconFlag | kTipFlag)
                                        .Callback(0x8001)
                                        .Icon(0xABCD)
                                        .Tip(L"Volume")
                                        .Parse();
  EXPECT_EQ(message.command, kAdd);
  EXPECT_EQ(message.window, 0x1234u);
  EXPECT_EQ(message.id, 7u);
  EXPECT_EQ(message.callbackMessage, 0x8001u);
  EXPECT_EQ(message.icon, 0xABCDu);
  EXPECT_EQ(message.tip, L"Volume");
}

TEST(NotifyIconTest, RejectsOtherData) {
  std::vector<std::byte> bytes(64);
  EXPECT_FALSE(ParseNotifyIconData(bytes).has_value());
  EXPECT_FALSE(ParseNotifyIconData({}).has_value());
}

TEST(NotifyIconTest, OldStructureHasShortTipAndNoState) {
  const NotifyIconMessage message = Packet(kAdd, 152).Tip(L"Old").State(1, 1).Parse();
  EXPECT_EQ(message.tip, L"Old");
  EXPECT_EQ(message.stateMask, 0u);
}

TEST(TrayIconListTest, AddModifyDelete) {
  TrayIconList list;
  EXPECT_TRUE(list.Apply(Packet(kAdd).Window(1).Id(1).Flags(kIconFlag).Icon(10).Parse()));
  EXPECT_FALSE(list.Apply(Packet(kAdd).Window(1).Id(1).Parse()));  // already there
  EXPECT_TRUE(list.Apply(Packet(kAdd).Window(2).Id(1).Parse()));
  ASSERT_EQ(list.Icons().size(), 2u);

  EXPECT_TRUE(list.Apply(Packet(kModify).Window(1).Id(1).Flags(kTipFlag).Tip(L"New").Parse()));
  EXPECT_EQ(list.Icons()[0].tip, L"New");
  EXPECT_EQ(list.Icons()[0].icon, 10u);  // untouched without NIF_ICON

  EXPECT_TRUE(list.Apply(Packet(kDelete).Window(1).Id(1).Parse()));
  EXPECT_FALSE(list.Apply(Packet(kModify).Window(1).Id(1).Parse()));
  ASSERT_EQ(list.Icons().size(), 1u);
  EXPECT_EQ(list.Icons()[0].window, 2u);
}

TEST(TrayIconListTest, HidingAndVersion) {
  TrayIconList list;
  list.Apply(Packet(kAdd).Window(1).Id(1).Parse());
  list.Apply(Packet(kModify).Window(1).Id(1).Flags(kStateFlag).State(1, 1).Parse());
  EXPECT_TRUE(list.Icons()[0].hidden);
  list.Apply(Packet(kModify).Window(1).Id(1).Flags(kStateFlag).State(0, 1).Parse());
  EXPECT_FALSE(list.Icons()[0].hidden);
  EXPECT_TRUE(list.Apply(Packet(kSetVersion).Window(1).Id(1).Version(4).Parse()));
  EXPECT_EQ(list.Icons()[0].version, 4u);
}

TEST(TrayIconListTest, GuidIconsMatchByGuid) {
  TrayIconList list;
  list.Apply(Packet(kAdd).Window(1).Id(1).Flags(kGuidFlag).Guid(9).Parse());
  // Same GUID from another window: the same icon, now owned by that window.
  EXPECT_TRUE(list.Apply(Packet(kModify).Window(5).Id(3).Flags(kGuidFlag).Guid(9).Parse()));
  ASSERT_EQ(list.Icons().size(), 1u);
  EXPECT_EQ(list.Icons()[0].window, 5u);
}

TEST(TrayIconListTest, RemovesIconsOfClosedWindows) {
  TrayIconList list;
  list.Apply(Packet(kAdd).Window(1).Id(1).Parse());
  list.Apply(Packet(kAdd).Window(1).Id(2).Parse());
  list.Apply(Packet(kAdd).Window(2).Id(1).Parse());
  list.RemoveWindow(1);
  ASSERT_EQ(list.Icons().size(), 1u);
  EXPECT_EQ(list.Icons()[0].window, 2u);
}

}  // namespace
}  // namespace throwback
