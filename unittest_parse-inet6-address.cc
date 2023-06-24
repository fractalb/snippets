
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <vector>

extern "C" {
#include "parse-inet6-address.c"
}

std::vector<const char*> zeroIp = {
    "::",       "0::",       "00::",       "000::",       "0000::",
    "0:0000::", "00:0000::", "000:0000::", "0000:0000::", "0:0:0:0:0:0:0:0",
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

TEST(Ipv6Parser, ValidIps2) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "a::b:0:0:0:0:c";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(hextet[0], 0xa);
  EXPECT_EQ(hextet[1], 0);
  EXPECT_EQ(hextet[2], 0xb);
  EXPECT_EQ(hextet[3], 0);
  EXPECT_EQ(hextet[4], 0);
  EXPECT_EQ(hextet[5], 0);
  EXPECT_EQ(hextet[6], 0);
  EXPECT_EQ(hextet[7], 0xc);
}

TEST(Ipv6Parser, ValidIps3) {
  uint16_t hextet[8];
  bool valid;
  const char* addr = "a:b:c:d:e:f:c::0";
  const char* ret = parse_ipv6(addr, hextet, &valid);
  EXPECT_EQ(ret - 15, addr);
  EXPECT_TRUE(valid);
  EXPECT_EQ(hextet[0], 0xa);
  EXPECT_EQ(hextet[1], 0xb);
  EXPECT_EQ(hextet[2], 0xc);
  EXPECT_EQ(hextet[3], 0xd);
  EXPECT_EQ(hextet[4], 0xe);
  EXPECT_EQ(hextet[5], 0xf);
  EXPECT_EQ(hextet[6], 0xc);
  EXPECT_EQ(hextet[7], 0x0);
}

TEST(InetPtoN, ValidIps1) {
  uint8_t bytes[16];
  const char* addr = "a:b:c:d:e:f:c::";
  int ret = inet_pton(AF_INET6, addr, bytes);
  EXPECT_EQ(ret, 1);
  EXPECT_EQ(bytes[0], 0);
  EXPECT_EQ(bytes[1], 0xa);
  EXPECT_EQ(bytes[2], 0);
  EXPECT_EQ(bytes[3], 0xb);
  EXPECT_EQ(bytes[4], 0);
  EXPECT_EQ(bytes[5], 0xc);
  EXPECT_EQ(bytes[6], 0);
  EXPECT_EQ(bytes[7], 0xd);
  EXPECT_EQ(bytes[8], 0);
  EXPECT_EQ(bytes[9], 0xe);
  EXPECT_EQ(bytes[10], 0);
  EXPECT_EQ(bytes[11], 0xf);
  EXPECT_EQ(bytes[12], 0);
  EXPECT_EQ(bytes[13], 0xc);
  EXPECT_EQ(bytes[14], 0);
  EXPECT_EQ(bytes[15], 0);
}

TEST(InetPtoN, ValidIps2) {
  uint8_t bytes[16];
  const char* addr = "::192.168.2.3";
  int ret = inet_pton(AF_INET6, addr, bytes);
  EXPECT_EQ(ret, 1);
  for (int i = 0; i < 12; i++) EXPECT_EQ(bytes[i], 0);
  EXPECT_EQ(bytes[12], 0xc0);
  EXPECT_EQ(bytes[13], 0xa8);
  EXPECT_EQ(bytes[14], 0x2);
  EXPECT_EQ(bytes[15], 0x3);
}

std::vector<const char*> testIpv6 = {
    "::",
    "0::",
    "::0",
    "0::0",
    "a::b",
    "::ab",
    "ab::",
    "a:b:c:d:e:f:a:b",
    ":::",
    "::::",
    "::0::",
    ":0::",
    "::0:",
    "0123:4567:890a:bcde:f012:3456:7890:abcd",
    "::192.168.3.4",
    "::0192.168.3.4",
};

TEST(Ipv6ToBytes, MultiTest1) {
  uint8_t bytes1[16];
  uint8_t bytes2[16];
  for (auto addr : testIpv6) {
    int ret1 = inet_pton(AF_INET6, addr, bytes1);
    int ret2 = str2ipv6(addr, bytes2);
    // std::cout << "addr: " << addr << '\n';
    if (ret1 == 1) {
      EXPECT_EQ(ret2, 0);
      EXPECT_EQ(memcmp(bytes1, bytes2, sizeof(bytes1)), 0);
    } else if (ret1 == 0) {
      EXPECT_EQ(ret2, -1);
    }
  }
}
