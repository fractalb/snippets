
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static int convert_from_hex_to_dec(int val)
{
	int ret = 0;
	int tmp;
	for(int i = 3; i >= 0; i--) {
		ret *= 10;
		if ((tmp = val>>(i*8) & 0xff) > 9)
			return -1;
		ret += tmp;
	}
	return ret;
}

int str2ipv6(const char *str, char *buf, int size)
{
	const char *s = str;
	uint32_t prefix = 128;
	int dbl_colon_start = 0;
	int val = 0;
	int qi = 0;
	int extra = 0;
	int v4qi = 0;
	char *b = buf;
	char c;

	while ((c = *s++) != '\0') {
		if (val > 65535)
			goto err;

		if ('0' <= c && c <= '9') {
			val *= 16;
			val += c - '0';
		} else if ('a' <= c && c <= 'f') {
			val *= 16;
			val += c - 'a' + 10;
		} else if ('A' <= c && c <= 'F') {
			val *= 16;
			val += c - 'A' + 10;
		} else if (c == ':' && qi < 7) {
			*b++ = val >> 8;
			*b++ = val & 0xff;
			qi++;
			val = 0;
			if (*s == ':') {
				if (qi == 7 || dbl_colon_start != 0)
					goto err;
				dbl_colon_start = qi;
				*b++ = 0;
				*b++ = 0;
				qi++, s++;
			} else if(*s == '.') {
				goto err;
			}
		} else if (c == '.' && qi < 7) {
			val = convert_from_hex_to_dec(val);
			if (val < 0)
				goto err;
			v4qi = 0;
			s--;
			break;
		} else if (c == '/' && qi <= 7) {
			*b++ = val >> 8;
			*b++ = val & 0xff;
			qi++;
			val = 0;
		} else {
			goto err;
		}
	}

	qi *= 2;
	dbl_colon_start *= 2;

	while ((c = *s++) != '\0') {
		if (val > 255)
			goto err;

		if ('0' <= c && c <= '9') {
			val *= 10;
			val += c - '0';
		} else if ((c == '.' && v4qi < 3) || (c == '/' && v4qi == 3)) {
			*b++ = val;
			qi++;
			v4qi++;
			val = 0;
		} else {
			goto err;
		}
	}

	if (v4qi == 3 && val <= 255) {
		*b++ =  val;
		qi++;
	} else if (v4qi == 4 && val <= 128) {
		/* val is actually no. of bits of subnet mask */
		prefix = val;
	} else if (v4qi == 0 && qi <= 16 && val <= 128) {
		prefix = val;
	} else {
		goto err;
	}

	extra = 16 - qi;
	memmove(buf+dbl_colon_start+extra, buf+dbl_colon_start, qi - dbl_colon_start);
	memset(buf+dbl_colon_start, 0, extra);
	return 0;
err:
	return EINVAL;
}
/* IPv4 CIDR to subnet address */
int ipstr2ipaddr(const char *str, uint32_t *ipaddr)
{
	const char *s = str;
	uint32_t mask = -1;
	uint32_t ip = 0;
	int val = 0;
	int qi = 0;
	char c;

	while ((c = *s++) != '\0') {
		if (val > 255)
			goto err;

		if ('0' <= c && c <= '9') {
			val *= 10;
			val += c - '0';
		} else if ((c == '.' && qi < 3) || (c == '/' && qi == 3)) {
			ip <<= 8;
			ip += val;
			qi++;
			val = 0;
		} else {
			goto err;
		}
	}

	if (qi == 3 && val <= 255) {
		ip <<= 8;
		ip += val;
	} else if (qi == 4 && val <= 32) {
		/* val is actually no. of bits of subnet mask */
		mask >>= 32 - val;
		mask <<= 32 - val;
	} else {
		goto err;
	}

	ip &= mask;
	if (ipaddr)
		*ipaddr = ip;
	else
		printf("%s = %d.%d.%d.%d\n", str, (ip >> 24) & 0xff,
		       (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
	return 0;
err:
	return EINVAL;
}

int main()
{
	const char *ipstrs[] = { "10.1.2.3/22",
				 "192.168.32.2/29",
				 "172.16.129.34/21",
				 "192.1.164.58/26",
				 "0.0.0.0/0",
				 "11.12.13.14/4",
				 "255.255.255.255/32",
				 "255.255.255.255/31",
				 "254.254.255.255/30",
				 "14.6.8.1/32",
				 "10.2.3.29",
				 "10.2.2.2.32",
				 "49.58.64.256",
				 "59.32.4",
				 "59.32..4",
				 "....",
				 "...",
				 "83.213.79/65" };

	for (int i = 0; i < ARRAY_SIZE(ipstrs); i++) {
		if (ipstr2ipaddr(ipstrs[i], NULL) != 0)
			printf("%s is an invalid address\n", ipstrs[i]);
	}

	return 0;
}
