CFLAGS := -Wall -Wextra -Iinclude $(DEFINES)
CC := clang
TARGET := ./riscv-vm
FDT := ./dts/rv64_dt.dtb
KERNEL := kernel/fw_payload.bin

SRCS := \
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
	devices/ns16550.c  \
	soc/soc.c \
	main.c

OBJS := ${SRCS:.c=.o}

all: $(TARGET) $(FDT) $(KERNEL)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -lpthread -o $@ $^

$(KERNEL):
	../kernel/build

%.dtb: %.dts
	m4 $(DEFINES) < $^ | dtc -O dtb -o $@ -

run: all
	$(TARGET) $(FDT) $(KERNEL)

clean:
	-@rm $(TARGET) $(FDT) $(KERNEL) $(OBJS) 2>/dev/null

