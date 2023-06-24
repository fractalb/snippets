
#include "ip-parser.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define Q1(x) (((x) >> 24) & 0xff)
#define Q2(x) (((x) >> 16) & 0xff)
#define Q3(x) (((x) >> 8) & 0xff)
#define Q4(x) (((x) >> 0) & 0xff)

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

const char *parse_ipv4(const char *str, int64_t *ipaddr) {
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
  UNKNOWN = 0,
  SINGLE_COLON = 1,
  DOUBLE_COLON = 2,
  SINGLE_DOT = 3,
} sep_t;

typedef enum {
  INVALID = 0,
  VALID = 1,
  FINISH = 2,
} state_t;

typedef struct {
  uint16_t *hextet;
  uint8_t current_index;
  int8_t double_colon_index;
  state_t state;
  const char *buf_backtrack;
} ipv6_parser_ctx_t;

static inline const char *str_state(state_t s) {
  switch (s) {
    case INVALID:
      return "INVALID";
    case VALID:
      return "VALID";
    case FINISH:
      return "FINISH";
    default:
      return "ERROR STATE";
  }
}

[[maybe_unused]] static void print_parse_ctx(ipv6_parser_ctx_t *ctx) {
  printf("current_index: %d\n", (int)ctx->current_index);
  printf("double_colon_index: %d\n", (int)ctx->double_colon_index);
  printf("State: %s\n", str_state(ctx->state));
}

#define PRINT_PARSE_CTX(ctx)               \
  ({                                       \
    printf("%s:%d\n", __func__, __LINE__); \
    print_parse_ctx(ctx);                  \
  })

static inline const char *parse_sep(const char *buf, sep_t *separator) {
  *separator = UNKNOWN;
  if (*buf && *buf == ':') {
    if (*(buf + 1) && *(buf + 1) == ':') {
      *separator = DOUBLE_COLON;
      return buf + 2;
    } else {
      *separator = SINGLE_COLON;
      return buf + 1;
    }
  } else if (*buf == '.') {
    *separator = SINGLE_DOT;
    return buf + 1;
  }
  return buf;
}

static inline const char *parse_hexdigit(const char *buf, int *hex_val) {
  *hex_val = -1;
  int c = *buf;

  if (c >= '0' && c <= '9')
    *hex_val = c - '0';
  else if (c >= 'a' && c <= 'f')
    *hex_val = c - 'a' + 10;
  else if (c >= 'A' && c <= 'F')
    *hex_val = c - 'A' + 10;
  else
    return buf;

  return buf + 1;
}

static inline const char *parse_hextet(const char *buf, int *hextet_val) {
  *hextet_val = -1;

  const char *rbuf = buf;
  int hex_val = -1;

  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val = hex_val;  // At least, one valid hex digit

  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  return rbuf;
}

const char *parse_sep_and_hextet(const char *buf, ipv6_parser_ctx_t *ctx) {
  if (ctx->state == INVALID || ctx->state == FINISH) return buf;

  if (ctx->current_index == 8 && ctx->state == VALID) {
    ctx->state = FINISH;
    return buf;
  }

  if (ctx->current_index >= 8 ||
      ctx->double_colon_index >= ctx->current_index) {
    ctx->state = INVALID;
    return buf;
  }

  bool single_colon_parsed = false;
  uint8_t i = ctx->current_index;
  sep_t separator = UNKNOWN;
  const char *rbuf = buf;
  int hextet_val;

  // Parse separator
  rbuf = parse_sep(rbuf, &separator);
  // PRINT_PARSE_CTX(ctx);
  if (separator == UNKNOWN) {
    if (ctx->double_colon_index != -1)
      ctx->state = FINISH;
    else
      ctx->state = INVALID;
    goto end;
  }
  // PRINT_PARSE_CTX(ctx);
  switch (separator) {
    case DOUBLE_COLON:
      if (ctx->double_colon_index >= 0) {
        ctx->state = INVALID;
        rbuf -= 2;  // Go back to begining of ::
        goto end;
      }
      // zero hextet. Be ware the number of zeros can be more.
      ctx->double_colon_index = i++;
      if (i == 8) {
        ctx->state = FINISH;
        goto end;
      }
      break;
    case SINGLE_COLON:
      if (ctx->current_index == 0) {
        // Address should not start with a single colon
        ctx->state = INVALID;
        rbuf--;
        goto end;
      }
      single_colon_parsed = true;
      break;
    case SINGLE_DOT:
      // PRINT_PARSE_CTX(ctx);
      if (ctx->double_colon_index < 0 && i != 7) {
        ctx->state = INVALID;
        rbuf--;
        goto end;
      }
      rbuf = ctx->buf_backtrack;
      int64_t ipv4;
      rbuf = parse_ipv4(rbuf, &ipv4);
      if (ipv4 < 0 || ipv4 > UINT_MAX) {
        ctx->state = INVALID;
        goto end;
      }
      ctx->hextet[i - 1] = ((uint32_t)ipv4 >> 16) & 0xffff;
      ctx->hextet[i++] = ((uint32_t)ipv4) & 0xffff;
      ctx->state = FINISH;
      goto end;
    default:
      assert(0);
  }

  ctx->buf_backtrack = rbuf;
  // PRINT_PARSE_CTX(ctx);
  rbuf = parse_hextet(rbuf, &hextet_val);
  if (hextet_val == -1) {
    if (single_colon_parsed) {
      /* Check if it's a valid addres without the single colon
       * which has just been parsed. */
      if (ctx->double_colon_index >= 0) {
        ctx->state = FINISH;
        rbuf--;  // Valid without the single colon
      } else {
        // Should not end with a single colon ':'.
        ctx->state = INVALID;
      }
      goto end;
    }
    // Parsed double colon
    ctx->state = FINISH;
    goto end;
  }
  ctx->hextet[i++] = hextet_val;

end:
  ctx->current_index = i;
  // PRINT_PARSE_CTX(ctx);
  return rbuf;
}

static inline int expand_double_colon(ipv6_parser_ctx_t *ctx) {
  assert(ctx->current_index <= 8);
  assert(ctx->double_colon_index >= 0);
  assert(ctx->double_colon_index < 8);
  assert(ctx->double_colon_index < ctx->current_index);
  assert(ctx->state == FINISH);
  if (ctx->current_index == 8) return 0;
  int i, j;
  for (i = ctx->current_index - 1, j = 7; i < j && i != ctx->double_colon_index;
       i--, j--)
    ctx->hextet[j] = ctx->hextet[i];
  while (j > 0 && j != i) ctx->hextet[j--] = 0;
  return 0;
}

const char *parse_ipv6(const char *buf, uint16_t hextet[8], bool *valid) {
  memset(hextet, 0, 8 * sizeof(hextet[0]));
  *valid = false;

  ipv6_parser_ctx_t ctx = {
      .hextet = hextet,
      .current_index = 0,
      .double_colon_index = -1,
      .state = VALID,
      .buf_backtrack = NULL,
  };

  const char *rbuf = buf;
  int hextet_val = -1;

  rbuf = parse_hextet(rbuf, &hextet_val);
  if (hextet_val != -1) {
    hextet[0] = hextet_val;
    ctx.current_index++;
  }

  // PRINT_PARSE_CTX(&ctx);
  while (ctx.current_index < 8 && ctx.state == VALID)
    rbuf = parse_sep_and_hextet(rbuf, &ctx);

  if (ctx.state == INVALID) return rbuf;

  if (ctx.current_index < 8 && ctx.state == FINISH &&
      ctx.double_colon_index >= 0) {
    if (expand_double_colon(&ctx) != 0) return rbuf;
  }

  *valid = true;
  return rbuf;
}

int str2ipv6(const char *ipstr, uint8_t bytes[16]) {
  uint16_t hextet[8];
  bool valid;
  ipstr = parse_ipv6(ipstr, hextet, &valid);
  if (valid && *ipstr == '\0') {
    for (int i = 0; i < 8; i++) {
      bytes[2 * i] = (hextet[i] >> 8) & 0xff;
      bytes[2 * i + 1] = hextet[i] & 0xff;
    }
    return 0;
  }
  return -1;
}
