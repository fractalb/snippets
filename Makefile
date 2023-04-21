
all: hex2binary-test hex2binary-cmd hex-dump

hex-dump: hex-dump.c
	gcc -std=gnu11 -Wall -Wextra $< -o $@

hex2binary-test: hex2binary.c unittest_hex2binary.cc
	g++ -Wall unittest_hex2binary.cc -lgtest -lgtest_main -o $@

hex2binary-cmd: hex2binary-cmd.cc hex2binary.c
	g++ -std=c++17 -Wall -Wextra $< -o $@

.PHONY=clean
clean:
	rm -f *.o
	rm -f hex-dump hex2binary-test hex2binary-cmd
