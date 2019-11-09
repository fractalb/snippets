
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static int convert_base_from_hex_to_decimal(uint16_t val)
{
	int new_val = 0;
	int decimal_digit;

	for (int i = 3; i >= 0; i--) {
		decimal_digit = (val >> (i * 4)) & 0xf;
		if (decimal_digit > 9)
			return -1;
		new_val *= 10;
		new_val += decimal_digit;
	}
	return new_val;
}

/*TODO:
 * ::192.168.43.248/34  Should it be considered invalid?
 * Currently prefix 34 (> 32) and anything <= 128 is taken
 * as valid prefix even though IPv4 dotted quad is in inside IPv6.
 */
int str2ipv6(const char *str, char *buf, int size)
{
	const char *s = str;
	int prefix = -1;
	int dcs_index = 0; /* double colon start index */
	int val = 0;
	int qi = 0;
	int extra = 0;
	int v4qi = -1;
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
				if (qi == 7 || dcs_index != 0)
					goto err;
				dcs_index = qi;
				*b++ = 0;
				*b++ = 0;
				qi++, s++;
			}
			if (*s == '.' || *s == ':') {
				goto err;
			}
		} else if (c == '.' && qi < 7) {
			val = convert_base_from_hex_to_decimal(val);
			if (val < 0)
				goto err;
			v4qi = 0;
			break;
		} else if (c == '/' && qi <= 7) {
			*b++ = val >> 8;
			*b++ = val & 0xff;
			qi++;
			val = 0;
			prefix = 128;
		} else {
			goto err;
		}
	}

	if (!dcs_index && qi < 7)
		goto err;

	qi *= 2;
	dcs_index *= 2;

	if (prefix >= 0) {
		val = convert_base_from_hex_to_decimal(val);
	}

	s--;
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
		*b++ = val;
		qi++;
	} else if (v4qi == 4 && val <= 128) {
		/* val is actually no. of bits of subnet mask */
		prefix = val;
	} else if (v4qi == -1 && qi <= 16) {
		if (prefix >= 0 && val <= 128) {
			prefix = val;
		} else if (prefix < 0 && qi <= 14) {
			buf[qi++] = val >> 8;
			buf[qi++] = val & 0xff;
		} else {
			goto err;
		}
	} else {
		goto err;
	}

	extra = 16 - qi;
	memmove(buf + dcs_index + extra, buf + dcs_index, qi - dcs_index);
	memset(buf + dcs_index, 0, extra);
	if (prefix >= 0)
		buf[16] = prefix;
	return 0;
err:
	return EINVAL;
}

/* IPv4 CIDR to subnet address */
int str2ipv4(const char *str, uint32_t *ipaddr)
{
	const char *s = str;
	uint32_t mask = -1;
	uint32_t ip = 0;
	int val = 0;
	int i4 = 0;
	char c;

	while ((c = *s++) != '\0') {
		if (val > 255)
			goto err;

		if ('0' <= c && c <= '9') {
			val *= 10;
			val += c - '0';
		} else if ((c == '.' && i4 < 3) || (c == '/' && i4 == 3)) {
			ip <<= 8;
			ip += val;
			i4++;
			val = 0;
		} else {
			goto err;
		}
	}

	if (i4 == 3 && val <= 255) {
		ip <<= 8;
		ip += val;
	} else if (i4 == 4 && val <= 32) {
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

	const char *ip6strs[] = {
		"ffff:0102::24/48",
		"::0a0b:28/68",
		":0a0b::28/68",
		":0a0b::28:/68",
		":0a0b:28/68",
		":0a0b:28:/68",
		"abcd:dcba:eeff:ffee:dead:beef:8496:1024",
		"abcd:dcba:eeff:ffee:dead:beef:8496:1024/98",
		"9232:0:48::23/122",
		"::",
		"::192.168.43.28/24",
		"::192.168.43.248",
		"::192.168.43.248/34",
		"::257.168.43.248/34",
		"::157.168.43.248.28/30",
		"::/132",
		"::/128",
		":::/128",
		"2004:::/128",
		"10df::45::98/12",
		"fffff:12:0/120",
	};

	unsigned char buf[18];

	for (int i = 0; i < ARRAY_SIZE(ipstrs); i++) {
		if (str2ipv4(ipstrs[i], NULL) != 0)
			printf("%s is an invalid address\n", ipstrs[i]);
	}

	for (int i = 0; i < ARRAY_SIZE(ip6strs); i++) {
		printf("%s -> ", ip6strs[i]);
		memset(buf, 0, sizeof(buf));
		if (str2ipv6(ip6strs[i], buf, 17) != 0)
			printf("is an invalid address\n");
		else {
			printf("%02x%02x", buf[0], buf[1]);
			for (int j = 1; j < 8; j++) {
				printf(":%02x%02x", buf[2 * j], buf[2 * j + 1]);
			}
			printf("/%d\n", buf[16]);
		}
	}

	return 0;
}
