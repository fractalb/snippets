
#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <vector>

#include "ip-parser.h"

std::vector<const char*> zeroIp = {
    "::",       "0::",       "00::",       "000::",       "0000::",
    "0:0000::", "00:0000::", "000:0000::", "0000:0000::", "0:0:0:0:0:0:0:0",
};

TEST(Ipv6Parser, ValidZeroAddr) {
  uint8_t bytes[16];
  bool valid;
  for (auto addr : zeroIp) {
    const char* ret = parse_ipv6(addr, bytes, &valid);
    EXPECT_TRUE(valid);
    EXPECT_EQ(addr + strlen(addr), ret);
    for (auto a : bytes) EXPECT_EQ(a, 0);
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
  uint8_t bytes[16];
  bool valid;
  const char* addr = ":0::";
  // std::cout << "addr = " << addr << '\n';
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_EQ(ret, addr);
  EXPECT_FALSE(valid);
}

TEST(Ipv6Parser, InvalidIpAddr2) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "0:00000:";
  // std::cout << "addr = " << addr << '\n';
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_EQ(ret - 6, addr);
  EXPECT_FALSE(valid);
}

TEST(Ipv6Parser, ValidInvalid1) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = ":::";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : bytes) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid2) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::::";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 2);
  for (auto a : bytes) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid3) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "0::::";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : bytes) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid4) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "0:0:::";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 5);
  for (auto a : bytes) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid5) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::0:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 3);
  for (auto a : bytes) EXPECT_EQ(a, 0);
}

TEST(Ipv6Parser, ValidInvalid6) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::0a:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 4);
  for (int i = 0; i < 15; ++i) {
    EXPECT_EQ(bytes[i], 0);
  }
  EXPECT_EQ(bytes[15], 0xa);
}

TEST(Ipv6Parser, ValidInvalid7) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::0ab:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 5);
  for (int i = 0; i < 15; ++i) {
    EXPECT_EQ(bytes[i], 0);
  }
  EXPECT_EQ(bytes[15], 0xab);
}

TEST(Ipv6Parser, ValidInvalid8) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::0abc:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 6);
  for (int i = 0; i < 14; ++i) {
    EXPECT_EQ(bytes[i], 0);
  }
  EXPECT_EQ(bytes[14], 0xa);
  EXPECT_EQ(bytes[15], 0xbc);
}

TEST(Ipv6Parser, ValidInvalid9) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "::0abcd:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 6);
  for (int i = 0; i < 14; ++i) {
    EXPECT_EQ(bytes[i], 0);
  }
  EXPECT_EQ(bytes[14], 0xa);
  EXPECT_EQ(bytes[15], 0xbc);
}

TEST(Ipv6Parser, ValidIps1) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "0a::bcd:";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_TRUE(valid);
  EXPECT_EQ(ret - addr, 7);
  EXPECT_EQ(bytes[0], 0);
  EXPECT_EQ(bytes[1], 0xa);
  for (int i = 2; i < 14; ++i) {
    EXPECT_EQ(bytes[i], 0);
  }
  EXPECT_EQ(bytes[14], 0xb);
  EXPECT_EQ(bytes[15], 0xcd);
}

TEST(Ipv6Parser, ValidIps2) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "a::b:0:0:0:0:c";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_EQ(ret - strlen(addr), addr);
  EXPECT_TRUE(valid);
  EXPECT_EQ(bytes[0], 0x0);
  EXPECT_EQ(bytes[1], 0xa);
  EXPECT_EQ(bytes[2], 0);
  EXPECT_EQ(bytes[3], 0);
  EXPECT_EQ(bytes[4], 0x0);
  EXPECT_EQ(bytes[5], 0xb);
  EXPECT_EQ(bytes[6], 0);
  EXPECT_EQ(bytes[7], 0);
  EXPECT_EQ(bytes[8], 0);
  EXPECT_EQ(bytes[9], 0);
  EXPECT_EQ(bytes[10], 0);
  EXPECT_EQ(bytes[11], 0);
  EXPECT_EQ(bytes[12], 0);
  EXPECT_EQ(bytes[13], 0);
  EXPECT_EQ(bytes[14], 0x0);
  EXPECT_EQ(bytes[15], 0xc);
}

TEST(Ipv6Parser, ValidIps3) {
  uint8_t bytes[16];
  bool valid;
  const char* addr = "a:b:c:d:e:f:c::0";
  const char* ret = parse_ipv6(addr, bytes, &valid);
  EXPECT_EQ(ret - 15, addr);
  EXPECT_TRUE(valid);
  EXPECT_EQ(bytes[0], 0x0);
  EXPECT_EQ(bytes[1], 0xa);
  EXPECT_EQ(bytes[2], 0x0);
  EXPECT_EQ(bytes[3], 0xb);
  EXPECT_EQ(bytes[4], 0x0);
  EXPECT_EQ(bytes[5], 0xc);
  EXPECT_EQ(bytes[6], 0x0);
  EXPECT_EQ(bytes[7], 0xd);
  EXPECT_EQ(bytes[8], 0x0);
  EXPECT_EQ(bytes[9], 0xe);
  EXPECT_EQ(bytes[10], 0x0);
  EXPECT_EQ(bytes[11], 0xf);
  EXPECT_EQ(bytes[12], 0x0);
  EXPECT_EQ(bytes[13], 0xc);
  EXPECT_EQ(bytes[14], 0x0);
  EXPECT_EQ(bytes[15], 0x0);
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
    "ab:01:02:03:04:abcd:ffff::",
    "::192.168.3.4",
    "::192.168.3.4.5",
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

/******************* IPv4 Unittests ***************************/
std::vector<const char*> testIpv4 = {
    "",        ".",       "1.2.3..4",        ".1.2.3.4",  "192.168.1.2",
    "1.2.3.4", "0.0.0.0", "256.255.255.255", "1.2.3.4.5", "01.02.03.04",
    "1.2",
};

TEST(Ipv4ToUint32, MultiTest1) {
  uint32_t ip1, ip2;
  int mask;
  for (auto addr : testIpv4) {
    int ret1 = inet_pton(AF_INET, addr, &ip1);
    int ret2 = str2ipv4(addr, &ip2, &mask);
    // std::cout << "addr: " << addr << '\n';
    if (ret1 == 1) {
      EXPECT_EQ(ret2, 0);
      EXPECT_EQ(mask, 32);
      EXPECT_EQ(ip2, ntohl(ip1));
    } else if (ret1 == 0) {
      EXPECT_EQ(ret2, -1);
    }
  }
}
