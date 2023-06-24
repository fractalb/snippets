
#ifdef __cplusplus
extern "C" {
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define Q1(x) (((x) >> 24) & 0xff)
#define Q2(x) (((x) >> 16) & 0xff)
#define Q3(x) (((x) >>  8) & 0xff)
#define Q4(x) (((x) >>  0) & 0xff)

const char *parse_ipv4(const char *str, int64_t *ipaddr);
const char *parse_ipv6(const char *buf, uint16_t hextet[8], bool *valid);
int str2ipv4(const char *ipquad, uint32_t *ipaddr, int *prefix);
int str2ipv6(const char *ipstr, uint8_t bytes[16]);
void print_ipv4(uint32_t ip, int mask);
void print_ipv6(unsigned char buf[16], int prefix);

#ifdef __cplusplus
}
#endif
