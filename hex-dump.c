#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static inline char hex(char c)
{
	if (c >= 0 && c <= 9)
		return '0' + c;
	if (c >= 10 && c <= 15)
		return 'a' + c - 10;
	assert(0);
	return 0;
}

/** This is only a helper function. It should not be called directly
 * for the users.
 * This function consumes 16 bytes from `buf` and writes 78 bytes into `hexbuf`.
 * The caller must ensure that `buf` contains enough input data(16 bytes) and
 * `hexbuf` has atleast 78 bytes space.
 * NOTE: No NUL character appended at the end
 */
static inline int conv_16bytes(char *hexbuf, char *buf)
{
	int i, j;
	char str[16];

	j = sprintf(hexbuf, "%.16p : ", buf);
	assert(j == 21); // assuming 64 bit pointer addresses
	for (i=0; i < 16; i++) {
		hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
		hexbuf[j++] = hex(buf[i] & 0xf);
		if (i & 0x1 == 0x1)
			hexbuf[j++] = ' ';
		str[i] = isprint(buf[i]) ? buf[i] : '.';
	}
	memcpy(&hexbuf[j], str, 16);
	j += 16;
	hexbuf[j++] = '\n';
	assert(j == 78);
	return j;
}

static inline int conv_nbytes(char *hexbuf, char *buf, int size)
{
	int i, j;
	char str[16];

	assert(size < 16);

	j = sprintf(hexbuf, "%.16p : ", buf);
	assert(j == 21); // assuming 64 bit pointer addresses
	for (i=0; i < size; i++) {
		hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
		hexbuf[j++] = hex(buf[i] & 0xf);
		if (i & 0x1 == 0x1)
			hexbuf[j++] = ' ';
		str[i] = isprint(buf[i]) ? buf[i] : '.';
	}
	while (j < 61)
		hexbuf[j++] = ' ';
	memcpy(&hexbuf[j], str, size);
	j += size;
	hexbuf[j++] = '\n';
	assert(j + 16 - size == 78);
	return j;
}

static inline int conv_nbytes_2(char *hexbuf, char *buf, int size)
{
	int i, j;

	assert(size < 16);

	j = sprintf(hexbuf, "%.16p : ", buf);
	assert(j == 21); // assuming 64 bit pointer addresses
	for (i=0; i < size; i++) {
		hexbuf[j++] = hex((buf[i] & 0xf0) >> 4);
		hexbuf[j++] = hex(buf[i] & 0xf);
		if (i & 0x1 == 0x1)
			hexbuf[j++] = ' ';
	}
	hexbuf[j++] = '\n';
	return j;
}

int hex_dump(char *hexbuf, int hbsize, char *buf, int size)
{
	int i, j, rc = -1;

	if (size < 1 || hbsize < 1)
		return rc;

	while (hbsize >= 78 && size > 16) {
		j = conv_16bytes(hexbuf, buf);
		assert(j == 78);
		hexbuf += j;
		hbsize -= j;
		buf    += 16;
		size   -= 16;
	}


	if (hbsize >= 78) {
		j = conv_nbytes(hexbuf, buf, size);
		rc = 0;
	} else if (hbsize > 21 + (size + 1)/2 * 5) {
		j = conv_nbytes_2(hexbuf, buf, size);
	} else if (hbsize > 21) {
		j = sprintf(hexbuf, "%.16p : ", buf);
	}

	hexbuf += j;
	hbsize -= j;

	// Terminate the string properly
	if (hbsize == 0) {
		hexbuf[-1] = '\0'; //Overwrite the previous char.
		rc = -1;
	} else {
		hexbuf[0] = '\0';
	}

	return rc;
}

int main()
{
	char *str;
	char pc[200];
	char hexbuf[16 * 78];

	while (true) {
		str = fgets(pc, 200, stdin);
		if (str == NULL)
			break;
		hex_dump(hexbuf, sizeof(hexbuf), str, strlen(str));
		printf("%s", hexbuf);
	}
}
