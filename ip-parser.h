
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define INET_ADDRSTRLEN 16

const char *parse_ipv4(const char *str, int64_t *ipaddr);
const char *parse_ipv6(const char *buf, uint16_t hextet[8], bool *valid);
int str2ipv4(const char *ipquad, uint32_t *ipaddr, int *prefix);
int str2ipv6(const char *ipstr, uint8_t bytes[16]);
void print_ipv4(uint32_t ip, int mask);
void print_ipv6(unsigned char buf[16], int prefix);
const char *ipv4_string(char ipstr[INET_ADDRSTRLEN], uint32_t ipaddr);
void print_ipv4(uint32_t ip, int mask);

#ifdef __cplusplus
}
#endif
