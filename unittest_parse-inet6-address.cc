
#include <gtest/gtest.h>
#include <arpa/inet.h>

#include <vector>

extern "C" {
#include "parse-inet6-address.c"
}

std::vector<const char*> zeroIp = {
    "::",       "0::",       "00::",       "000::",       "0000::",
    "0:0000::", "00:0000::", "000:0000::", "0000:0000::",
    "0:0:0:0:0:0:0:0",
};

TEST(Ipv6Parser, ValidZeroAddr) {
  uint16_t hextet[8];
  bool valid;
  for (auto addr : zeroIp) {
    const char* ret = parse_ipv6(addr, hextet, &valid);
    EXPECT_TRUE(valid);
    EXPECT_EQ(addr + strlen(addr), ret);
    for (auto a : hextet) EXPECT_EQ(a, 0);
  }
}

// Sample invalid IP's
// ":::", "::::",
// ":0:",
// "0::0:",
// "::0:",
// ":0::",
// "0:00000::",
// "0:::", "0::0::", "0:0::0:",
// "0:0::",

TEST(Ipv6Parser, InvalidIpAddr1) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = ":0::";
  // std::cout << "addr = " << addr << '\n';
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_EQ(ret, addr);
  EXPECT_FALSE(valid);
}

TEST(Ipv6Parser, InvalidIpAddr2) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0:00000:";
  // std::cout << "addr = " << addr << '\n';
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_EQ(ret - 6, addr);
  EXPECT_FALSE(valid);
}

TEST(Ipv6Parser, ValidInvalid1) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = ":::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid2) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid3) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0::::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid4) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "0:0:::";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 5);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid5) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "::0:";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : hextet) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid6) {
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

TEST(Ipv6Parser, ValidInvalid7) {
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

TEST(Ipv6Parser, ValidInvalid8) {
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

TEST(Ipv6Parser, ValidInvalid9) {
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

TEST(Ipv6Parser, ValidIps1) {
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

TEST(InetPtoN, ValidIps1) {
  uint8_t bytes[16];
  const char* addr = "0a::00bcd";
  int ret = inet_pton(AF_INET6, addr, bytes);
  EXPECT_EQ(ret, 1);
  EXPECT_EQ(bytes[0], 0);
  EXPECT_EQ(bytes[1], 0xa);
  EXPECT_EQ(bytes[14], 0xb);
  EXPECT_EQ(bytes[15], 0xcd);
}
