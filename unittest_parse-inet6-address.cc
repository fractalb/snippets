
#include <gtest/gtest.h>

#include <vector>

extern "C" {
#include "parse-inet6-address.c"
}

std::vector<const char*> zeroIp = {
    "::",       "0::",       "00::",       "000::",       "0000::",
    "0:0000::", "00:0000::", "000:0000::", "0000:0000::",
    "0:0:0:0:0:0:0:0",
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

std::vector<const char*> invalidIps = {
    // ":::", "::::",
    //  ":0:",
    //  "0::0:",
    //  "::0:",
    ":0::",
    // "0:::", "0::0::", "0:0::0:",
    // "0:0::",
};

TEST(Ipv6_Parser, InvalidIpAddr) {
  uint16_t hextet[8];
  bool valid;
  for (auto addr : invalidIps) {
    std::cout << "addr = " << addr << '\n';
    const char* ret = parse_ipv6(addr, hextet, &valid);
    EXPECT_FALSE(valid);
  }
}

TEST(Ipv6_parser, ValidInvalid1) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = ":::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6_parser, ValidInvalid2) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6_parser, ValidInvalid3) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0::::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6_parser, ValidInvalid4) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0:0:::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 5);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6_parser, ValidInvalid5) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6_parser, ValidInvalid6) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0a:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 4);
  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(hextet[i], 0);
  }
  EXPECT_EQ(hextet[7], 0xa);
}

TEST(Ipv6_parser, ValidInvalid7) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0ab:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 5);
  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(hextet[i], 0);
  }
  EXPECT_EQ(hextet[7], 0xab);
}

TEST(Ipv6_parser, ValidInvalid8) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0abc:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 6);
  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(hextet[i], 0);
  }
  EXPECT_EQ(hextet[7], 0xabc);
}

TEST(Ipv6_parser, ValidInvalid9) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0abcd:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 6);
  for (int i = 0; i < 7; ++i) {
    EXPECT_EQ(hextet[i], 0);
  }
  EXPECT_EQ(hextet[7], 0xabc);
}

TEST(Ipv6_parser, ValidIps1) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0a::bcd:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 7);
  EXPECT_EQ(hextet[0], 0xa);
  for (int i = 1; i < 7; ++i) {
    EXPECT_EQ(hextet[i], 0);
  }
  EXPECT_EQ(hextet[7], 0xbcd);
}
