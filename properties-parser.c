#include <bsd/string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024
#define MAX_VALUE_SIZE 512

struct parse_ctx {
  const char *white_space;
  const char *comment_prefix;
  const char *key_val_separator;
  const char *value_wrapper;
};

/*
 * A line which starts with a '#' is treated as a comment
 * '=' is treated as the separator of key, value pairs.
 * white_space characters should not include '\n' since it's
 * only valid as a line-ending.
 */
static const struct parse_ctx PCTX = {
    .white_space = "\t \v",
    .comment_prefix = "#",
    .key_val_separator = "=",
    .value_wrapper = "'",
};

void init_parse_ctx(struct parse_ctx *ctx) { memcpy(ctx, &PCTX, sizeof(*ctx)); }

static inline bool have_comments(const struct parse_ctx *pctx) {
  return pctx->comment_prefix != NULL && strcmp(pctx->comment_prefix, "") != 0;
}

static inline bool starts_with(const char *prefix, const char *str) {
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

char *parse_key_value_from_line(char *line, ssize_t len, const char *key,
                                const struct parse_ctx *pctx) {
  if (pctx == NULL)
    pctx = &PCTX;

  ssize_t _len = strspn(line, pctx->white_space);
  if (_len + 1 >= len)
    return NULL; // Full line is whitespace. return.
  if (have_comments(pctx) && starts_with(pctx->comment_prefix, line + _len)) {
    // printf("Skip Comment.\n");
    return NULL;
  }

  char *next = line + _len;
  char *s = strsep(&next, pctx->key_val_separator);
  if (s == NULL || next == NULL)
    return NULL;
  s += strspn(s, pctx->white_space);
  ssize_t l2 = strcspn(s, pctx->white_space);
  if (strncmp(key, s, l2) != 0)
    return NULL;
  if (strspn(s + l2, pctx->white_space) != strlen(s + l2))
    return NULL;

  /* The value is assumed to be wrapped in a pair of single quotes */
  char *s1 = strsep(&next, pctx->value_wrapper);
  if (s1 == NULL || next == NULL)
    return NULL;
  if (strspn(s1, pctx->white_space) != strlen(s1))
    return NULL;
  char *s2 = strsep(&next, pctx->value_wrapper);
  if (s2 == NULL || next == NULL)
    return NULL;
  if (strspn(next, pctx->white_space) + 1 /*\n*/ != strlen(next))
    return NULL;
  return s2;
}

int parse_key_value_from_file(const char *file, const char *key,
                              char value[MAX_VALUE_SIZE],
                              const struct parse_ctx *pctx) {
  int err = -1;
  FILE *stream;
  char *line = malloc(MAX_LINE_SIZE);
  size_t len = MAX_LINE_SIZE;
  ssize_t nread;

  stream = fopen(file, "r");
  if (stream == NULL) {
    perror("fopen");
    return err;
  }

  while ((nread = getline(&line, &len, stream)) != -1) {
    if (len <= 2)
      continue;
    const char *_value = parse_key_value_from_line(line, len, key, pctx);
    if (_value == NULL)
      continue;
    if (strlcpy(value, _value, MAX_VALUE_SIZE) > MAX_VALUE_SIZE)
      fprintf(stderr, "The value of \"%s\" truncated!\n", key);
    err = 0;
    break;
  }

  free(line);
  fclose(stream);
  return err;
}

int main() {
  struct parse_ctx pctx;
  init_parse_ctx(&pctx);
  pctx.white_space = " \t";
  pctx.comment_prefix = "##";
  const char *key = "ENV_CERT_CHAIN";
  char value[MAX_VALUE_SIZE];
  if (parse_key_value_from_file("test.properties", key, value, &pctx) != 0) {
    fprintf(stderr, "Error parsing the file\n");
    return -1;
  }
  printf("%s = %s\n", key, value);
  return 0;
}
