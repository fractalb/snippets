
all: hex2binary-test

hex2binary-test: hex2binary.c unittest_hex2binary.cc
	g++ unittest_hex2binary.cc -lgtest -lgtest_main -o $@
