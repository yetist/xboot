# README

```
$ make CROSS_COMPILE=loongarch64-linux-gnu- PLATFORM=loongarch64-virt
$ qemu-system-loongarch64 -m 512M -smp 4 -vga none -nographic -M virt -cpu la464 -kernel output/xboot
```
