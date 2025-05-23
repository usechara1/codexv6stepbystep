.PHONY: kernel clean

CFLAGS = -Wall -Werror -O0 \
		-fno-omit-frame-pointer -ggdb \
		-MD -mcmodel=medany \
		-ffreestanding -fno-common -nostdlib \
		-mno-relax \
		-fno-stack-protector -fno-pie -no-pie \
		-z max-page-size=4096 \
		-Wl,--no-warn-rwx-segments

kernel:
	riscv64-unknown-elf-gcc $(CFLAGS) \
		kernel/entry.S \
		kernel/start.c \
		kernel/uart.c \
		kernel/main.c \
		kernel/kernelvec.S \
		kernel/trap.c \
		-T kernel/kernel.ld \
		-o kernel.elf

QEMU = qemu-system-riscv64 -machine virt \
		-bios none \
		-kernel kernel.elf \
		-m 128 \
		-smp 1 \
		-nographic

qemu: kernel
	$(QEMU)

gdb: kernel
	$(QEMU) -s -S & \
		gdb-multiarch -q -x gdbinit

clean:
	rm -rf kernel.elf \
		kernel.d

# 解决偶发调试时卡死问题
killall:
	killall qemu-system-riscv64
	killall gdb-multiarch
