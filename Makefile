CC = cc
CPPFLAGS ?=
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS = -lm

.PHONY: all test clean
all: radec

radec: radec.c radec.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) radec.c $(LDLIBS) -o $@

tests/test_radec: tests/test_radec.c radec.c radec.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -DRADEC_NO_MAIN -I. $(LDFLAGS) tests/test_radec.c radec.c $(LDLIBS) -o $@

test: radec tests/test_radec
	./tests/test_radec
	python3 tests/test_cli.py ./radec

clean:
	rm -f radec tests/test_radec
