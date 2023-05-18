
#include <stdio.h>
#include <string.h>

static inline unsigned char int_val(char hex) {
  if ('0' <= hex && hex <= '9') {
    return hex - '0';
  }
  if ('a' <= hex && hex <= 'f') {
    return hex - 'a' + 10;
  }
  if ('A' <= hex && hex <= 'F') {
    return hex - 'A' + 10;
  }
  return 0xff;
}

/**
 * This program converts a hex-string into
 * its actual bytes represented in binary
 */
size_t decode_hex_to_binary(const char *hex, size_t hexlen, char *bin,
                            size_t binlen, int *error) {
  *error = -1;
  if (hexlen == 0 || binlen == 0) return 0;
  size_t buflen = (hexlen + 1) / 2;
  if (binlen < buflen) buflen = binlen;

  size_t i = 0;  // hex buffer
  size_t j = 0;  // bin buffer
  if (hexlen % 2 != 0) {
    unsigned char b = int_val(hex[i++]);
    if (b == 0xff) {
      fprintf(stderr, "Invalid hex string. offending char = '%c'\n",
              hex[i - 1]);
      return j;
    }
    bin[j++] = b;
  }
  for (; i < hexlen && j < buflen;) {
    unsigned char b1 = int_val(hex[i++]);
    unsigned char b2 = int_val(hex[i++]);
    if (b1 == 0xff || b2 == 0xff) {
      fprintf(stderr, "Invalid hex string. offending char = %c%c\n", hex[i - 2],
              hex[i - 1]);
      return j;
    }
    bin[j++] = (b1 << 4) | b2;
  }
  *error = 0;
  return j;
}
