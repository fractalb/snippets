
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/** This function will try to interpret the hex representation
 * of the given value as a base10 value and returns that value.
 *
 * If the hex reprsentation of the given value contains
 * alphabets (a - f) then it can't be interpreted as a
 * base10 value and this function will return -1.
 *
 * Examples:
 *
 *1. input : 146
 *
 *   146 => 0x92
 *   hex2base10(146) -> 92 (base10 value)
 *
 *2. input : 161
 *
 *   161 => 0xa1
 *   hex2base10(161) -> -1 ('a' can't be treated as a decimal number)
 */
static int hex2base10(uint16_t val) {
  int base10_val = 0;
  int decimal_digit;

  for (int i = 3; i >= 0; i--) {
    decimal_digit = (val >> (i * 4)) & 0xf;
    if (decimal_digit > 9) return -1;
    base10_val *= 10;
    base10_val += decimal_digit;
  }
  return base10_val;
}

/*TODO:
 * ::192.168.43.248/34  Should it be considered invalid?
 * Currently prefix 34 (> 32) and anything <= 128 is taken
 * as valid prefix even though IPv4 dotted quad is in inside IPv6.
 */
int str2ipv6(const char *str, char buf[16], int *prefix) {
  const char *s = str;
  int px = -1;
  int i_dc = 0; /* index of dc (double colon) */
  int val = 0;
  int i6 = 0;
  int i4 = -1;
  int nz = 0; /* no. of zero bytes */
  char c;
  char buf2[16];

  if (buf == NULL) buf = buf2;

  while ((c = *s++) != '\0') {
    if (val > 65535) goto err;

    if ('0' <= c && c <= '9') {
      val *= 16;
      val += c - '0';
    } else if ('a' <= c && c <= 'f') {
      val *= 16;
      val += c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
      val *= 16;
      val += c - 'A' + 10;
    } else if (px < 0) {
      if (c == ':' && i6 < 13) {
        buf[i6++] = val >> 8;
        buf[i6++] = val & 0xff;
        val = 0;
        if (*s == ':') {
          if (i6 == 14 || i_dc != 0) goto err;
          i_dc = i6;
          buf[i6++] = 0;
          buf[i6++] = 0;
          s++;
        }
        if (*s == '.' || *s == ':') {
          goto err;
        }
      } else if (c == '.' && i6 < 13) {
        i4 = 0;
        break; /* IPv4 parsing now */
      } else if (c == '/' && i6 < 15) {
        buf[i6++] = val >> 8;
        buf[i6++] = val & 0xff;
        val = 0;
        px = 128;
      } else {
        goto err;
      }
    } else {
      goto err;
    }
  }

  if (!i_dc) {
    if ((px < 0 && i6 != 14) || (px >= 0 && i6 != 16)) goto err;
  }

  if (px >= 0 || i4 == 0) {
    val = hex2base10(val);
    if (val < 0) goto err;
  }

  for (s--; (c = *s++) != '\0';) {
    if (val > 255) goto err;

    if ('0' <= c && c <= '9') {
      val *= 10;
      val += c - '0';
    } else if ((c == '.' && i4 < 3) || (c == '/' && i4 == 3)) {
      if (*s == '.' || *s == '/')  // Two consecutive dots
        goto err;
      buf[i6++] = val;
      i4++;
      val = 0;
    } else {
      goto err;
    }
  }

  if (i4 == 3 && val <= 255) {
    buf[i6++] = val;
  } else if (i4 == 4 && val <= 128) {
    /* val is actually no. of bits of subnet mask */
    px = val;
  } else if (i4 == -1 && i6 <= 16) {
    if (px >= 0 && val <= 128) {
      px = val;
    } else if (px < 0 && i6 <= 14) {
      buf[i6++] = val >> 8;
      buf[i6++] = val & 0xff;
    } else {
      goto err;
    }
  } else {
    goto err;
  }

  nz = 16 - i6;
  memmove(&buf[i_dc + nz], &buf[i_dc], i6 - i_dc);
  memset(&buf[i_dc], 0, nz);

  if (px >= 0 && prefix) *prefix = px;

  return 0;
err:
  return -1;
}

static inline bool is_ascii_digit(int x) { return x >= '0' && x <= '9'; }

/** Tries to parse a number (atmost 3 digits)
 * in an IPv4 quad. Stops at first non-digit
 * character or after the first three digits.
 * `value` will be set to -1 if there are zero
 * digits or more than 3 digits.
 *
 *    ┌─► str argument
 *    │┌───┐
 *   "└─192│.168.2.1"
 *     └───┘│
 *          └─► str returned
 */
static inline const char *parse_quad(const char *str, int *value) {
  *value = -1;
  // Make sure, no left padding of zeroes.
  // Rejects 00, 01, 001, but accepts 0.
  if (*str == '0' && is_ascii_digit(*(str + 1))) goto err;
  int val = 0;
  int i = 0;
  for (; i < 3 && is_ascii_digit(*str); i++, str++) {
    val *= 10;
    val += *str - '0';
  }
  // Reject no digit characters case
  if (i == 0) goto err;
  // Reject more than 3 digit characters
  if (i == 3 && is_ascii_digit(*str)) goto err;
  *value = val;
err:
  return str;
}

static const char *parse_ipv4(const char *str, int64_t *ipaddr) {
  *ipaddr = -1;
  unsigned quad1, quad2, quad3, quad4;
  const char *remainder = str;
  int value;
  remainder = parse_quad(remainder, &value);
  if (value < 0 || value > 255 || *remainder != '.') goto err;
  quad1 = value;
  remainder = parse_quad(++remainder, &value);
  if (value < 0 || value > 255 || *remainder != '.') goto err;
  quad2 = value;
  remainder = parse_quad(++remainder, &value);
  if (value < 0 || value > 255 || *remainder != '.') goto err;
  quad3 = value;
  remainder = parse_quad(++remainder, &value);
  if (value < 0 || value > 255) goto err;
  quad4 = value;
  *ipaddr = (quad1 << 24) + (quad2 << 16) + (quad3 << 8) + quad4;
err:
  return remainder;
}

int str2ipv4(const char *ipquad, uint32_t *ipaddr, int *prefix) {
  const char *remainder = ipquad;
  int64_t value;
  remainder = parse_ipv4(remainder, &value);
  if (value < 0 || value > UINT_MAX) goto err;
  *ipaddr = (uint32_t)value;
  if (prefix) {
    int subnet_mask = 32;
    if (*remainder == '/') {
      remainder = parse_quad(++remainder, &subnet_mask);
      if (subnet_mask < 0 || subnet_mask > 32) goto err;
    }
    *prefix = subnet_mask;
  }
  if (*remainder != '\0') goto err;
  return 0;
err:
  return -1;
}

#define Q1(x) (((x) >> 24) & 0xff)
#define Q2(x) (((x) >> 16) & 0xff)
#define Q3(x) (((x) >>  8) & 0xff)
#define Q4(x) (((x) >>  0) & 0xff)

void print_ipv4(uint32_t ip, int mask) {
  printf("%d.%d.%d.%d/%d\n", Q1(ip), Q2(ip), Q3(ip), Q4(ip), mask);
  return;
}

void print_ipv6(unsigned char buf[16], int prefix) {
  char b[44];
  snprintf(b, sizeof(b),
           "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%"
           "02x%02x/%-3d",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
           buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15],
           prefix);
  if (prefix < 0) b[39] = '\0';
  printf("%s\n", b);
  return;
}

typedef enum {
  VALID = 1,
  INVALID = 2,
} result_t;

const char *result_str(result_t t) {
  const char *s = NULL;
  switch (t) {
    case VALID:
      s = "VALID";
      break;
    case INVALID:
      s = "INVALID";
      break;
    default:
      s = "UNKNOWN";
      break;
  }

  return s;
}

typedef struct {
  const char *s;
  result_t expected;
} test_case_t;

test_case_t LAST_TEST_CASE = {0, 0};

typedef enum { IPv4, IPv6, UNKNOWN_TEST } TEST_NAME;

result_t run_test(TEST_NAME t, const void *test_args) {
  uint32_t ipaddr;
  int prefix;
  if (t == IPv4) {
    if (str2ipv4((const char *)test_args, &ipaddr, &prefix) == 0)
      return VALID;
    else
      return INVALID;
  } else if (t == IPv6) {
    if (str2ipv6((const char *)test_args, NULL, NULL) == 0)
      return VALID;
    else
      return INVALID;
  }
  return INVALID;
}

int main() {
  test_case_t ipv4_arr[] = {{"10.1.2.3/22", VALID},
                            {"192.168.32.2/29", VALID},
                            {" 192.168.32.2/29", INVALID},
                            {"192.168.32.2/29 ", INVALID},
                            {"192.168.b.2/29", INVALID},
                            {"192.168.2.3 ", INVALID},
                            {"192.168.2.3", VALID},
                            {"01.000.0.0/0", INVALID},
                            {"1.000.0.0/0", INVALID},
                            {"1.0.0.0/02", INVALID},
                            {"+1.0.0.0/0", INVALID},
                            {"1. 10.0.0", INVALID},
                            {"192.168.32.2a", INVALID},
                            {"192.168.32.2/", INVALID},
                            {"192.168.32.2/33", INVALID},
                            {"192.168.2/29", INVALID},
                            {"192.168.2", INVALID},
                            {"192...", INVALID},
                            {".192.12.34", INVALID},
                            {"00.192.12.34", INVALID},
                            {"0.192.12.34", VALID},
                            {"172.16.129.34/21", VALID},
                            {"192.1.164.58/26", VALID},
                            {"0.0.0.0/0", VALID},
                            {"11.12.13.14/4", VALID},
                            {"255.255.255.255/32", VALID},
                            {"255.255.255.255/31", VALID},
                            {"254.254.255.255/30", VALID},
                            {"14.6.8.1/32", VALID},
                            {"14.6.8./32", INVALID},
                            {"10.2.3.29", VALID},
                            {"10.2.2.2.32", INVALID},
                            {"49.58.64.256", INVALID},
                            {"59.32.4", INVALID},
                            {"59.32..4", INVALID},
                            {"....", INVALID},
                            {"...", INVALID},
                            {".../30", INVALID},
                            {"..", INVALID},
                            {"../20.", INVALID},
                            {"83.213.79/65", INVALID},
                            LAST_TEST_CASE};
  test_case_t ipv6_arr[] = {
      {"ffff:0102::24/48", VALID},
      {"::0a0b:28/68", VALID},
      {":0a0b::28/68", VALID},
      {":0a0b::28:/68", VALID},
      {":0a0b:28/68", INVALID},
      {":0a0b:28:/68", INVALID},
      {"abcd:dcba:eeff:ffee:dead:beef:8496:1024", VALID},
      {"abcd:dcba:eeff:ffee:dead:beef:8496::", INVALID},
      {"abcd:dcba:eeff:ffee:dead:beef:8496:1024/98", VALID},
      {"9232:0:48g::23/122", INVALID},
      {"9232:0:48::23/122", VALID},
      {"::", VALID},
      {"::192.168.43.28/24", VALID},
      {"::192.168.43.248", VALID},
      {"::192.168.43.248/34", VALID},
      {"::257.168.43.248/34", INVALID},
      {"::/192.168.43.28", INVALID},
      {"::192..43.28", INVALID},
      {"::192..43.28/30", INVALID},
      {"::/192.43..28/30", INVALID},
      {"::157.168.43.248.28/30", INVALID},
      {"::/132", INVALID},
      {"::/128", VALID},
      {":://128", INVALID},
      {":::/128", INVALID},
      {"2004:::/128", INVALID},
      {"10df::45::98/12", INVALID},
      {"fffff::12:0/120", INVALID},
      {"ffff::12:0/120", VALID},
      LAST_TEST_CASE,
  };

  printf("*****IPv4 Tests*****\n");
  int passed_tests = 0;
  int failed_tests = 0;
  result_t r;
  for (int i = 0; ipv4_arr[i].s != LAST_TEST_CASE.s; i++) {
    if ((r = run_test(IPv4, ipv4_arr[i].s)) != ipv4_arr[i].expected) {
      failed_tests++;
      printf("%s -> expected: %s, parsed: %s\n", ipv4_arr[i].s,
             result_str(ipv4_arr[i].expected), result_str(r));
      ;
    } else {
      passed_tests++;
    }
  }

  printf("Tests Passed: %d\nTests Failed: %d\n", passed_tests, failed_tests);

  /*****************************************************************************/

  printf("*****IPv6 Tests*****\n");
  passed_tests = 0;
  failed_tests = 0;
  for (int i = 0; ipv6_arr[i].s != LAST_TEST_CASE.s; i++) {
    if ((r = run_test(IPv6, ipv6_arr[i].s)) != ipv6_arr[i].expected) {
      failed_tests++;
      printf("%s -> expected: %s, parsed: %s\n", ipv6_arr[i].s,
             result_str(ipv6_arr[i].expected), result_str(r));
      ;
    } else {
      passed_tests++;
    }
  }

  printf("Tests Passed: %d\nTests Failed: %d\n", passed_tests, failed_tests);

  return 0;
}
