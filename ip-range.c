#include "ip-parser.h"
#include <stdio.h>

unsigned ipmask(int n) {
	uint64_t mask64 = UINT64_MAX;
	unsigned mask = UINT32_MAX;
	return ((mask64 >> (32 - n)) << (32 - n)) & mask;
}

static void test_ipmask() {
	for (int i=0; i<= 32; i++) {
		printf("i=%d, mask=%#x\n", i, ipmask(i));
	}
}

static void print_range(unsigned ipaddr, int prefix) {
	unsigned mask = ipmask(prefix);
	unsigned range_start = ipaddr & mask;
	unsigned range_end = range_start ^ (~mask);
	if (range_start != ipaddr) {
		printf("Incorrect range. ");
	}
	print_ipv4(ipaddr, prefix);
	printf("%#x - %#x\n", range_start, range_end);
}

static void parse_and_print_range(const char *ipstring) {
	uint32_t ipaddr;
	int prefix;
	int ret = str2ipv4(ipstring, &ipaddr, &prefix);
	if (ret != 0) {
		printf ("Incorrect IPv4(CIDR): %s\n", ipstring);
	}
	print_range(ipaddr, prefix);
}
int main(int argc, char *argv[]) {
#if 0
	test_ipmask();
#else
	if (argc == 1) {
		char ipstring[20] = {0};
		while (scanf("%19s", ipstring) != EOF)
			parse_and_print_range(ipstring);

		return 0;
	}
	for (int i = 1; i < argc; i++) {
		parse_and_print_range(argv[i]);
	}
#endif
	return 0;
}
