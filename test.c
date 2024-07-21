#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static const char *skip_spaces(const char *str)
{
	while (*str && (*str == ' ' || *str == '\t'))
		str++;
	return str;
}

static inline bool is_ascii_digit(int x)
{
	return x >= '0' && x <= '9';
}

// Parse atmost 3 digits in an ip address. "192.168.4.5"
static inline const char *parse_quad(const char *str, int *value)
{
	*value = -1;
	// Make sure, no left padding of zeroes.
	// Rejects 00, 01, 001, but accepts 0.
	if (*str == '0' && is_ascii_digit(*(str + 1)))
		goto err;
	int val = 0;
	int i = 0;
	for (; i < 3 && is_ascii_digit(*str); i++, str++) {
		val *= 10;
		val += *str - '0';
	}
	// Reject no digit characters case
	if (i == 0)
		goto err;
	// Reject more than 3 digit characters
	if (i == 3 && is_ascii_digit(*str))
		goto err;
	*value = val;
err:
	return str;
}

static const char *parse_ipv4(const char *str, int64_t *ipaddr)
{
	*ipaddr = -1;
	unsigned quad1, quad2, quad3, quad4;
	const char *remainder = str;
	int value;
	remainder = parse_quad(remainder, &value);
	if (value < 0 || value > 255 || *remainder != '.')
		goto err;
	quad1 = value;
	remainder = parse_quad(++remainder, &value);
	if (value < 0 || value > 255 || *remainder != '.')
		goto err;
	quad2 = value;
	remainder = parse_quad(++remainder, &value);
	if (value < 0 || value > 255 || *remainder != '.')
		goto err;
	quad3 = value;
	remainder = parse_quad(++remainder, &value);
	if (value < 0 || value > 255)
		goto err;
	quad4 = value;
	*ipaddr = (quad1 << 24) + (quad2 << 16) + (quad3 << 8) + quad4;
err:
	return remainder;
}

int util_parse_ip4_str(const char *ipstr, uint32_t *startp, uint32_t *endp)
{
	const char *remainder;
	int64_t value;
	remainder = skip_spaces(ipstr);
	remainder = parse_ipv4(remainder, &value);
	if (value < 0 || value > UINT_MAX)
		goto err;
	*startp = (uint32_t)value;
	if (*remainder == '/') { // Subnet
		int mask;
		remainder = parse_quad(++remainder, &mask);
		if (mask < 0 || mask > 32)
			goto err;
		*endp = *startp | (1U << (32 - mask)) - 1;
	} else {
		remainder = skip_spaces(remainder);
		if (*remainder == '-') { // Maybe, a range
			remainder = skip_spaces(++remainder);
			remainder = parse_ipv4(remainder, &value);
			if (value < 0 || value > UINT_MAX)
				goto err;
			*endp = (uint32_t)value;
		}
	}
	remainder = skip_spaces(remainder);
	if (*remainder != '\0') {
		goto err;
	}
	return 0;
err:
	return -1;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
int main()
{
	const char *ipstr[] = { "192.168.2.3",
				"0.1.2.3",
				" 0.1.2.032 ",
				" 0.1.2.320 ",
				" 00.1.2.32 ",
				"192.168.3.4/30",
				"192.168.3.6  - 	192.168.4.6",
				"192.168.3.6  - 	192.168.4.7" };
	uint32_t startp, endp;
	for (int i = 0; i < ARRAY_SIZE(ipstr); i++) {
		int ret = util_parse_ip4_str(ipstr[i], &startp, &endp);
		printf("ret=%d input=%s startp=%#x endp=%#x\n", ret, ipstr[i],
		       startp, endp);
	}
}
