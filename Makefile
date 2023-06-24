
CFLAGS := -std=c11 -Wall -Wextra -fsanitize=undefined
CXXFLAGS := -std=c++17 -Wall -Wextra -fsanitize=undefined

PROGS := hex2binary-test hex2binary-cmd hex-dump clib test-ip-parser

all: $(PROGS)

clib: clib.c
	$(CC) $(CFLAGS) $< -o $@

hex-dump: hex-dump.c
	$(CC) $(CFLAGS) $< -o $@

hex2binary-test: hex2binary.c unittest_hex2binary.cc
	$(CXX) $(CXXFLAGS) unittest_hex2binary.cc -lgtest -lgtest_main -lpthread -o $@

hex2binary-cmd: hex2binary-cmd.cc hex2binary.c
	$(CXX) $(CXXFLAGS) $< -o $@

ip-parser.o: ip-parser.c ip-parser.h
	$(CC) $(CFLAGS) -c $< -o $@

test-ip-parser: unittest_ip-parser.cc ip-parser.o
	$(CXX) $(CXXFLAGS) $^ -o $@ -lgtest -lgtest_main

.PHONY=clean
clean:
	rm -f *.o
	rm -f $(PROGS)
