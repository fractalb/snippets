
CFLAGS := -std=c11 -Wall -Wextra

PROGS := hex2binary-test hex2binary-cmd hex-dump parse-inet-address

all: $(PROGS)

hex-dump: hex-dump.c
	$(CC) $(CFLAGS) $< -o $@

hex2binary-test: hex2binary.c unittest_hex2binary.cc
	g++ -std=c++17 -Wall unittest_hex2binary.cc -lgtest -lgtest_main -lpthread -o $@

hex2binary-cmd: hex2binary-cmd.cc hex2binary.c
	g++ -std=c++17 -Wall -Wextra $< -o $@

parse-inet-address: parse-inet-address.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY=clean
clean:
	rm -f *.o
	rm -f $(PROGS)
