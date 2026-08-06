#
# Machine makefile
#

ASFLAGS         += -fno-pie
CFLAGS          += -fno-pie
LDFLAGS         += -no-pie -Wl,--gc-sections
MCFLAGS         := -march=loongarch64 -mabi=lp64d

LIBDIRS         :=
LIBS            :=
INCDIRS         :=
SRCDIRS         :=
