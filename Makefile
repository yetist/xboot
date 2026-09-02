#
# Top Makefile
#

TOPDIR = $(shell dirname $(shell pwd)/$(lastword $(MAKEFILE_LIST)))

.PHONY: all clean

all:
	@TOPDIR="$(TOPDIR)" $(MAKE) -s -C src all

clean:
	@TOPDIR="$(TOPDIR)" $(MAKE) -s -C src clean

dump:
	loongarch64-linux-gnu-objdump -d output/xboot > output/xboot.objdump

run: dump
	qemu-system-loongarch64 -m 2G -smp 4 -vga none -nographic -M virt -cpu max -rtc base=localtime -kernel output/xboot

debug: dump
	qemu-system-loongarch64 -m 2G -smp 4 -vga none -nographic -M virt -cpu max -rtc base=localtime -S -gdb tcp::1234,ipv4 -kernel output/xboot
