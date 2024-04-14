#include <arpa/inet.h>
#include <benchmark/benchmark.h>

#include <iostream>

#include "ip-parser.h"

#if 0
void print_ipv6(unsigned char buf[16]) {
  char b[40];
  int i;
  i = snprintf(b, sizeof(b),
           "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
           buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
  b[i] = '\0';
  printf("%s\n", b);
  return;
}
#endif

const char* ipv6str = "ab:01:02:03:04:abcd:ffff::";
static void BM_Str2ipv6(benchmark::State& state) {
  uint8_t bytes[16];
  for (auto _ : state) str2ipv6(ipv6str, bytes);
  // std::cout << "str2ipv6: ";
  // print_ipv6(bytes);
}

// Register benchmark
BENCHMARK(BM_Str2ipv6);

static void BM_Inetpton6(benchmark::State& state) {
  uint8_t bytes[16];
  for (auto _ : state) inet_pton(AF_INET6, ipv6str, bytes);
  // std::cout << "inet_pton6: ";
  // print_ipv6(bytes);
}

// Register benchmark
BENCHMARK(BM_Inetpton6);

// const char *ipquad = "192.168.23.1";
// const char *ipquad = "213.94.76.58";
// const char *ipquad = "124.192.178.154";
const char* ipquad = "10.11.12.13";
static void BM_Str2ipv4(benchmark::State& state) {
  uint32_t ipaddr;
  for (auto _ : state) str2ipv4(ipquad, &ipaddr, NULL);
  // std::cout << "str2ipv4: " << ipaddr << '\n';
}
// Register benchmark
BENCHMARK(BM_Str2ipv4);

static void BM_Inetpton(benchmark::State& state) {
  uint32_t ipaddr;
  for (auto _ : state) inet_pton(AF_INET, ipquad, &ipaddr);
  // std::cout << "inet_pton: " << ipaddr << '\n';
}
// Register the function as a benchmark
BENCHMARK(BM_Inetpton);

static void BM_InetNework(benchmark::State& state) {
  [[maybe_unused]]uint32_t ipaddr;
  for (auto _ : state) ipaddr = inet_network(ipquad);
  // std::cout << "inet_network: " << ipaddr << '\n';
}
// Register the function as a benchmark
BENCHMARK(BM_InetNework);
BENCHMARK_MAIN();
