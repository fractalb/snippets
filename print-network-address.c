
#include <stdio.h>
#include <stdint.h>
#include <errno.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

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
	const char *ipstrs[] = { "10.1.2.3/22",      "192.168.32.2/29",
				 "172.16.129.34/21", "192.1.164.58/26",
				 "0.0.0.0/0",	"11.12.13.14/4",
				 "14.6.8.1/32",      "10.2.3.29" };

	for (int i = 0; i < ARRAY_SIZE(ipstrs); i++) {
		if (ipstr2ipaddr(ipstrs[i], NULL) != 0)
			printf("%s is an invalid address\n", ipstrs[i]);
	}
}
