# DEFINES := -DUSE_SIMPLE_UART
CFLAGS := -Wall -Wextra -Iinclude $(DEFINES)
CC := clang
TARGET := ./riscv-vm
FDT := ./dts/rv64_dt.dtb
KERNEL := kernel/fw_payload.bin

SRCS := \
	helpers/fifo.c  \
	helpers/file.c  \
	core/hart.c  \
	core/csr.c  \
	core/pmp.c  \
	core/trap.c  \
	core/mmu.c  \
	core/instructions.c \
	core/memory.c \
	devices/clint.c  \
	devices/plic.c  \
	devices/uart_8250.c  \
	soc/soc.c \
	main.c

OBJS := ${SRCS:.c=.o}

all: $(TARGET) $(FDT)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

%.dtb: %.dts
	m4 $(DEFINES) < $^ | dtc -O dtb -o $@ -

run: all
	$(TARGET) $(FDT) $(KERNEL)

clean:
	-@rm $(TARGET) $(FDT) $(OBJS) 2>/dev/null

