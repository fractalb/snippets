
#include <gtest/gtest.h>

#include <vector>

extern "C" {
#include "parse-inet6-address.c"
}

std::vector<const char*> zeroIp = {
    "::",       "0::",       "00::",       "000::",       "0000::",
    "0:0000::", "00:0000::", "000:0000::", "0000:0000::",
};
TEST(Ipv6_Parser, ValidZeroAddr) {
  uint16_t hextet[8];
  bool valid;
  for (auto addr : zeroIp) {
    const char* ret = parse_ipv6(addr, hextet, &valid);
    EXPECT_TRUE(valid);
    EXPECT_EQ(addr + strlen(addr), ret);
    for (auto a : hextet) EXPECT_EQ(a, 0);
  }
}
