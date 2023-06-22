#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  UNKNOWN = 0,
  SINGLE_COLON = 1,
  DOUBLE_COLON = 2,
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
} parse_ctx_t;

static const char *parse_sep(const char *buf, sep_t *separator) {
  *separator = UNKNOWN;
  if (*buf && *buf == ':') {
    if (*(buf + 1) && *(buf + 1) == ':') {
      *separator = DOUBLE_COLON;
      return buf + 2;
    } else {
      *separator = SINGLE_COLON;
      return buf + 1;
    }
  }
  // printf("\t\tchar: %c\n", *buf);
  return buf;
}

static const char *parse_hexdigit(const char *buf, int *hex_val) {
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

static const char *parse_hextet(const char *buf, int *hextet_val) {
  *hextet_val = -1;

  const char *rbuf = buf;
  int hex_val = -1;

  // printf("%d\t\tchar: %c\n", __LINE__, *rbuf);
  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val = hex_val;  // At least, one valid hex digit

  // printf("%d\t\tchar: %c\n", __LINE__, *rbuf);
  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  // printf("%d\t\tchar: %c\n", __LINE__, *rbuf);
  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  // printf("%d\t\tchar: %c\n", __LINE__, *rbuf);
  rbuf = parse_hexdigit(rbuf, &hex_val);
  if (hex_val == -1) return rbuf;
  *hextet_val *= 16;
  *hextet_val += hex_val;

  return rbuf;
}

static const char *str_state(state_t s) {
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

static void print_parse_ctx(parse_ctx_t *ctx) {
  printf("current_index: %d\n", (int)ctx->current_index);
  printf("double_colon_index: %d\n", (int)ctx->double_colon_index);
  printf("State: %s\n", str_state(ctx->state));
}

#define PRINT_PARSE_CTX(ctx)               \
  ({                                       \
    printf("%s:%d\n", __func__, __LINE__); \
    print_parse_ctx(ctx);                  \
  })

const char *parse_sep_and_hextet(const char *buf, parse_ctx_t *ctx) {
  if (ctx->state == INVALID || ctx->state == FINISH) return buf;

  if (ctx->current_index > 8 || ctx->double_colon_index >= ctx->current_index) {
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
  if (separator == DOUBLE_COLON) {
    if (ctx->double_colon_index >= 0 || i + 2 >= 8) {
      ctx->state = INVALID;
      rbuf -= 2;  // Go back to begining of ::
      goto end;
    }
    ctx->double_colon_index = i;
    i += 2;  // Minimum two zero hextets. Be ware it can be more.
  } else {   // SINGLE_COLON
    if (ctx->current_index == 0) {
      // Address should not start with a single colon
      ctx->state = INVALID;
      rbuf--;
      goto end;
    }
    single_colon_parsed = true;
  }

  // PRINT_PARSE_CTX(ctx);

  rbuf = parse_hextet(rbuf, &hextet_val);
  if (hextet_val == -1) {
    if (single_colon_parsed) {
      if (ctx->double_colon_index >= 0) {
        ctx->state = FINISH;
        rbuf--; // Valid without the single colon
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

static inline int expand_double_colon(parse_ctx_t *ctx) {
  assert(ctx->current_index <= 8);
  assert(ctx->double_colon_index >= 0);
  assert(ctx->double_colon_index + 2 <= 8);
  assert(ctx->double_colon_index < ctx->current_index);
  assert(ctx->state == FINISH);
  if (ctx->current_index == 8) return 0;
  int i, j;
  for (i = ctx->current_index, j = 8; i != ctx->double_colon_index + 1;
       i--, j--)
    ctx->hextet[j] = ctx->hextet[i];
  while (j != i) ctx->hextet[j--] = 0;
  return 0;
}

const char *parse_ipv6(const char *buf, uint16_t hextet[8], bool *valid) {
  memset(hextet, 0, 8 * sizeof(hextet[0]));
  *valid = false;

  parse_ctx_t ctx = {
      .hextet = hextet,
      .current_index = 0,
      .double_colon_index = -1,
      .state = VALID,
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
