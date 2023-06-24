
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define Q1(x) (((x) >> 24) & 0xff)
#define Q2(x) (((x) >> 16) & 0xff)
#define Q3(x) (((x) >> 8) & 0xff)
#define Q4(x) (((x) >> 0) & 0xff)

#define IPV4_MAX_STR_SIZE 16
void u32_to_ip_quad(uint32_t ip, char str[IPV4_MAX_STR_SIZE]) {
  int i;
  i = snprintf(str, IPV4_MAX_STR_SIZE, "%u.%u.%u.%u", Q1(ip), Q2(ip), Q3(ip),
               Q4(ip));
  ;
  assert(i < IPV4_MAX_STR_SIZE);
}

char *u32_ip_as_string(uint32_t ip) {
  char *ipstr = malloc(IPV4_MAX_STR_SIZE);
  if (!ipstr) return NULL;
  u32_to_ip_quad(ip, ipstr);
  return ipstr;
}

/** This function doesn't do full validation of
 * the `ipstr` string. For example the below
 * ip-addresses strings are all accepted as 0x1020304.
 *
 * 1.  "01.2.3.4"
 * 2.  "+1.2.3.4"
 * 3.  "1. 2. 3. 4"
 * 4.  "1.+2.3.4"
 *
 * Note: Use this function only if you know that
 * `ipstr` is a valid IP address. Use an IPv4
 * string parser if you want to validate the input
 * or if you don't trust the input.
 */
int64_t ip_quad_to_u32(const char *ipstr) {
  uint32_t q[4];
  uint32_t ip;
  int i;
  if (!ipstr) return -1;
  i = sscanf(ipstr, "%u.%u.%u.%u", &q[3], &q[2], &q[1], &q[0]);
  if (i != 4) return -1;
  if (q[3] > 255 || q[2] > 255 || q[1] > 255 || q[0] > 255) return -1;
  ip = (q[3] << 24) | (q[2] << 16) | (q[1] << 8) | q[0];
  return ip;
}

static char hex(int i) {
  if (i >= 0 && i <= 9) return '0' + i;
  if (i >= 10 && i <= 15) return 'a' + i - 10;
  return 0;
}

/* Write hex string into `str` from `buf`. `str` is expected
 * to have enough memory to hold the full hex-string.
 * i.e. 3*size */
static void hex_str(const char buf[], int size, char *str) {
  int j = 0;
  if (size < 1) return;

  const char byte_separator = ' ';
  str[j++] = hex((buf[0] & 0xf0) >> 4);
  str[j++] = hex(buf[0] & 0x0f);
  for (int i = 1; i < size; i++) {
    str[j++] = byte_separator;
    str[j++] = hex((buf[i] & 0xf0) >> 4);
    str[j++] = hex(buf[i] & 0xf);
  }
  str[j] = '\0';
}

int test_ip() {
  int ret = 0;
  uint32_t ip[] = {0, 1, 7, 8, 9, 16, 0x01010101, 0xffffffff};
  int n = ARR_SIZE(ip);
  char *ipstr;
  int64_t ip_new;
  for (int i = 0; i < n; i++) {
    ipstr = u32_ip_as_string(ip[i]);
    ip_new = ip_quad_to_u32(ipstr);
    if (ip[i] != ip_new) {
      printf("Failed for ip=%#x, str=%s, parsed_ip=%#lx\n", ip[i], ipstr,
             ip_new);
      ret--;
    }
    free(ipstr);
  }
  return ret;
}

void TEST_IP() {
  int i;
  i = test_ip(0);
  if (i == 0)
    printf("***All IP tests passed***\n");
  else
    printf("***IP Tests Failed(%d)***\n", -i);
}

void TEST_HEX() {
  const char buf[] = "Test buf";
  char hexbuf[600];
  const char *expected_output = "54 65 73 74 20 62 75 66";
  hex_str(buf, sizeof(buf) - 1, hexbuf);
  if (strcmp(hexbuf, expected_output) != 0)
    printf("***Hex conversion test failed***\n");
  else
    printf("***Hex conversion test passed***\n");
}

int main() {
  TEST_IP();
  TEST_HEX();
  return 0;
}
