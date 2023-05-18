#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prints a given buf in hex to a given buffer, along with a NUL terminating
 * character. Offset + hex representation of 16 bytes + ascii signature of 16
 * bytes
 */

static inline char hex(char c) {
  if (c >= 0 && c <= 9) return '0' + c;
  if (c >= 10 && c <= 15) return 'a' + c - 10;
  assert(0);
  return 0;
}

/** This is only a helper function. This is not intended to be called by the
 * end-users directly. Prints 16 bytes at a time with offset and ascii signature
 * This function consumes 16 bytes from `buf` and writes 70 bytes into `hexbuf`.
 * The caller must ensure that `buf` contains enough input data(16 bytes) and
 * `hexbuf` has atleast 70 bytes space.
 * NOTE: No NUL character appended at the end
 */
static inline int conv_16bytes(char *hexbuf, char *buf, int offset) {
  int i, j;
  char str[16];

  j = sprintf(hexbuf, "0x%.8x : ", offset);
  assert(j == 13);  // assuming 64 bit pointer addresses
  for (i = 0; i < 16; i++) {
    hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
    hexbuf[j++] = hex(buf[i] & 0xf);
    if ((i & 0x1) == 0x1) hexbuf[j++] = ' ';
    str[i] = isprint(buf[i]) ? buf[i] : '.';
  }
  memcpy(&hexbuf[j], str, 16);
  j += 16;
  hexbuf[j++] = '\n';
  assert(j == 70);
  return j;
}

static inline int conv_nbytes(char *hexbuf, char *buf, int size, int offset) {
  int i, j;
  char str[16];

  assert(size < 16);

  j = sprintf(hexbuf, "0x%.8x : ", offset);
  assert(j == 13);  // assuming 64 bit pointer addresses
  for (i = 0; i < size; i++) {
    hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
    hexbuf[j++] = hex(buf[i] & 0xf);
    if ((i & 0x1) == 0x1) hexbuf[j++] = ' ';
    str[i] = isprint(buf[i]) ? buf[i] : '.';
  }
  while (j < 53) hexbuf[j++] = ' ';
  memcpy(&hexbuf[j], str, size);
  j += size;
  hexbuf[j++] = '\n';
  assert(j + 16 - size == 70);
  return j;
}

static inline int conv_nbytes_2(char *hexbuf, char *buf, int size, int offset) {
  int i, j;

  assert(size < 16);

  j = sprintf(hexbuf, "0x%.8x : ", offset);
  assert(j == 21);  // assuming 64 bit pointer addresses
  for (i = 0; i < size; i++) {
    hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
    hexbuf[j++] = hex(buf[i] & 0xf);
    if ((i & 0x1) == 0x1) hexbuf[j++] = ' ';
  }
  hexbuf[j++] = '\n';
  return j;
}

int hex_dump(char *hexbuf, int hbsize, char *buf, int size) {
  int j, rc = 0;
  int offset = 0;

  if (size < 1 || hbsize < 1) return rc;

  /* 10 byte offset + 16 bytes * 2+1 */
  while (hbsize >= 70 && size >= 16) {
    j = conv_16bytes(hexbuf, buf, offset);
    offset += 16;
    assert(j == 70);
    rc += j;
    hexbuf += j;
    hbsize -= j;
    buf += 16;
    size -= 16;
  }

  if (size > 0) {
    if (hbsize >= 70)
      j = conv_nbytes(hexbuf, buf, size, offset);
    else if (hbsize > 13 + (size + 1) / 2 * 5)
      j = conv_nbytes_2(hexbuf, buf, size, offset);
    else if (hbsize > 13)
      j = sprintf(hexbuf, "0x%.8x : ", offset);

    hbsize -= j;
    hexbuf += j;
    rc += j;
  }

  // Terminate the string properly
  if (hbsize == 0) {
    hexbuf[-1] = '\0';  // Overwrite the previous char.
  } else {
    hexbuf[0] = '\0';
    rc++;
  }

  return rc;
}

#ifdef DEBUG_TEST
int main() {
  char *str;
  char pc[80];
  char hexbuf[16 * 70];
  int rc;
  int len;

  while (true) {
    str = fgets(pc, sizeof(pc), stdin);
    if (!str) break;
    len = strlen(str);
    rc = hex_dump(hexbuf, sizeof(hexbuf), str, len);
    if (rc > 0) {
      printf("%s", hexbuf);
      printf("Read = %d, Printed = %d\n", len, rc);
    }
  }
  printf("\n");
}
#else
int main() {
  size_t buf_len = 4000;
  char *buf = malloc(buf_len);
  size_t hexbuf_len = 20000 + 1;  // 5 times input
  char *hexbuf = malloc(hexbuf_len);
  while (!feof(stdin)) {
    size_t size = fread(buf, 1, buf_len, stdin);
    int rc = hex_dump(hexbuf, hexbuf_len, buf, size);
    if (rc > 0) printf("%s\n", hexbuf);
  }
  return 0;
}
#endif
