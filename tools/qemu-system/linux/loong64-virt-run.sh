#!/bin/sh
#

# The qemu's root directory.
export QEMU_DIR=$(
  cd $(dirname $0)
  pwd
)

# Run qemu
exec qemu-system-loongarch64 -M virt -m 512M -smp 4 -name "Loong64 Virtual Machine" -rtc base=localtime -serial stdio -kernel ${QEMU_DIR}/../../../output/xboot
