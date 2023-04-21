#include <gtest/gtest.h>

extern "C" {
#include "hex2binary.c"
}

std::vector<std::tuple<const char *, const char *>> test_strings{
    {"", ""},
    {"f", "\xf"},
    {"ff", "\xff"},
    {"12", "\x12"},
    {"12034", "\x1\x20\x34"}};

TEST(TestHex2Binary, Test1) {
  const char *hex;
  const char *exp;
  char buf[80];
  for (auto [h, e] : test_strings) {
    hex = h;
    exp = e;
    int error;
    size_t s = decode_hex_to_binary(hex, strlen(hex), buf, 79, &error);
    buf[s] = '\0';
    ASSERT_STREQ(buf, exp);
    // EXPECT_EQ(error, 0);
  }
}
