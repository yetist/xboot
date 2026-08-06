#!/bin/sh
#

# The qemu's root directory.
export QEMU_DIR=$(
  cd $(dirname $0)
  pwd
)

# Run qemu
exec qemu-system-loongarch64 -M virt -cpu la464 -m 512M -smp 1 -name "Loong64 Virtual Machine" -S -gdb tcp::1234,ipv4 -rtc base=localtime -serial stdio -kernel ${QEMU_DIR}/../../../output/xboot
