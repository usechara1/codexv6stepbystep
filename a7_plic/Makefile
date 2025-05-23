.PHONY: kernel clean

K=kernel
U=user

OBJS= \
	$(K)/entry.o \
	$(K)/start.o \
	$(K)/main.o \
	$(K)/uart.o \
	$(K)/kernelvec.o \
	$(K)/trap.o \
	$(K)/kalloc.o \
	$(K)/string.o \
	$(K)/printf.o \
	$(K)/vm.o \
	$(K)/proc.o \
	$(K)/spinlock.o \
	$(K)/plic.o

TOOLPREFIX = riscv64-unknown-elf-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

QEMU = qemu-system-riscv64

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
# CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -fno-common -nostdlib
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free
CFLAGS += -fno-builtin-memcpy -Wno-main
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LDFLAGS = -z max-page-size=4096

$(K)/kernel: $(OBJS) $(K)/kernel.ld
	$(LD) $(LDFLAGS) -T $(K)/kernel.ld -o $(K)/kernel $(OBJS)
	$(OBJDUMP) -S $K/kernel > $K/kernel.asm
	$(OBJDUMP) -t $K/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $K/kernel.sym

CPUS := 1

QEMUOPTS = -machine virt \
		-bios none \
		-kernel $(K)/kernel \
		-m 128 \
		-smp $(CPUS) \
		-nographic

qemu: $(K)/kernel
	@echo "*** Build success ***"
	$(QEMU) $(QEMUOPTS)

gdb: $(K)/kernel
	@echo "*** Build success ***"
	$(QEMU) $(QEMUOPTS) -s -S & \
		gdb-multiarch -q -x gdbinit

qemu-gdb: $K/kernel
	@echo "*** Build gdb success ***"
	$(QEMU) $(QEMUOPTS) -s -S

qemu-debug:
	gdb-multiarch -q -x gdbinit

clean:
	rm -rf kernel/kernel
	rm -rf kernel.d
	rm -rf */*.d
	rm -rf */*.o */*.asm */*.sym
