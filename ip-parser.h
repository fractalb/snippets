
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

const char *parse_ipv4(const char *str, int64_t *ipaddr);
const char *parse_ipv6(const char *buf, uint8_t bytes[16], bool *valid);
int str2ipv4(const char *ipquad, uint32_t *ipaddr, int *prefix);
int str2ipv6(const char *ipstr, uint8_t bytes[16]);
void print_ipv4(uint32_t ip, int mask);
void print_ipv6(unsigned char buf[16], int prefix);

#ifdef __cplusplus
}
#endif
