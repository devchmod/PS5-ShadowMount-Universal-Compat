PS5_PAYLOAD_SDK ?= /opt/ps5-payload-sdk
include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk

CFLAGS := -O2 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes \
          -Werror=strict-prototypes -Werror=missing-prototypes -std=gnu11
LDFLAGS := -Wl,--gc-sections

.PHONY: all clean

all: universal-compat.elf universal-compat-restore.elf

universal-compat.elf: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(PS5_PAYLOAD_SDK)/bin/prospero-strip --strip-all $@

universal-compat-restore.elf: restore.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	$(PS5_PAYLOAD_SDK)/bin/prospero-strip --strip-all $@

main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

restore.o: restore.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.elf
