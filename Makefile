# 工具定义
CC = gcc
LD = ld
ASM = nasm
OBJCOPY = objcopy
QEMU = qemu-system-x86_64

# 目录结构
BOOT_DIR = boot
KERNEL_DIR = kernel
LIBS_DIR = libs
DRIVERS_DIR = drivers
SCRIPT_DIR = scripts

# 内存配置 - 可覆盖的默认值
QEMU_MEMORY ?= 64
KERNEL_MEMORY_MB ?= 64

DEBUG ?= 1
ENABLE_SERIAL ?= 1

# 编译和链接标志 - 传递内存大小给内核
CFLAGS = -m32 -nostdlib -ffreestanding -Wall -Wextra \
         -I$(KERNEL_DIR) -I$(DRIVERS_DIR) -I$(KERNEL_DIR)/memory -I$(LIBS_DIR) \
         -DKERNEL_MEMORY_MB=$(KERNEL_MEMORY_MB)

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
endif

ifeq ($(ENABLE_SERIAL),1)
	CFLAGS += -DENABLE_SERIAL
endif

LDFLAGS = -m elf_i386 -T $(SCRIPT_DIR)/linker.ld -nostdlib
ASFLAGS = -f elf32

# 自动查找源文件
KERNEL_C_SRCS = $(shell find $(KERNEL_DIR) -name "*.c" -not -name ".*")
DRIVER_C_SRCS = $(shell find $(DRIVERS_DIR) -name "*.c" -not -name ".*")
KERNEL_ASM_SRCS = $(shell find $(KERNEL_DIR) -name "*.asm" -not -name ".*")
LIBS_C_SRCS = $(shell find $(LIBS_DIR) -name "*.c" -not -name ".*")

# 推导目标文件
KERNEL_C_OBJS = $(KERNEL_C_SRCS:.c=.c.o)
DRIVER_C_OBJS = $(DRIVER_C_SRCS:.c=.c.o)
LIBS_C_OBJS = $(LIBS_C_SRCS:.c=.c.o)
KERNEL_ASM_OBJS = $(KERNEL_ASM_SRCS:.asm=.asm.o)

# 正确的链接顺序
ALL_OBJS = $(KERNEL_ASM_OBJS) $(KERNEL_C_OBJS) $(DRIVER_C_OBJS) $(LIBS_C_OBJS)

# 最终目标
KERNEL_ELF = $(KERNEL_DIR)/kernel.elf
KERNEL_BIN = $(KERNEL_DIR)/kernel.bin
OS_IMAGE = myos.img

# 默认目标
all: $(OS_IMAGE)

# 生成操作系统镜像
$(OS_IMAGE): $(BOOT_DIR)/boot.bin $(KERNEL_BIN)
	@echo "Creating OS image..."
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BOOT_DIR)/boot.bin of=$@ conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc
	@echo "OS image created: $@"

# 编译引导程序
$(BOOT_DIR)/boot.bin: $(BOOT_DIR)/boot.asm
	@echo "Building bootloader..."
	$(ASM) -f bin $< -o $@

# 生成内核二进制文件
$(KERNEL_BIN): $(KERNEL_ELF)
	@echo "Creating kernel binary..."
	$(OBJCOPY) -O binary $< $@

# 链接内核
$(KERNEL_ELF): $(ALL_OBJS)
	@echo "Linking kernel..."
	@echo "Object files: $(words $(ALL_OBJS)) files"
	@echo "Configured memory: $(KERNEL_MEMORY_MB) MB"
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJS)
	@echo "Kernel linked: $@"

# 编译规则
%.c.o: %.c
	@echo "Compiling C: $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

%.asm.o: %.asm
	@echo "Assembling: $< -> $@"
	$(ASM) $(ASFLAGS) $< -o $@

# 清理构建产物
clean:
	@echo "Cleaning build files..."
	rm -f $(OS_IMAGE) $(BOOT_DIR)/boot.bin $(KERNEL_BIN) $(KERNEL_ELF)
	find $(KERNEL_DIR) $(DRIVERS_DIR) $(LIBS_DIR) -name "*.c.o" -delete
	find $(KERNEL_DIR) $(DRIVERS_DIR) -name "*.asm.o" -delete


# 运行目标 - 支持不同内存配置
run: $(OS_IMAGE)
	@echo "Starting QEMU with $(QEMU_MEMORY)MB RAM..."
	$(QEMU) -m $(QEMU_MEMORY) -drive format=raw,file=$(OS_IMAGE)

# 串口调试模式 (VSCode终端输出)
debug: $(OS_IMAGE)
	@echo "=== Debug Mode (Serial Output) ==="
	@echo "Press Ctrl+A then X to exit"
	@echo "Build config: DEBUG=$(DEBUG), ENABLE_SERIAL=$(ENABLE_SERIAL)"
	$(QEMU) -m $(QEMU_MEMORY) \
	        -drive format=raw,file=$(OS_IMAGE) \
	        -nographic \
			-serial tcp::5555,server,nowait \
			-monitor stdio

# 图形模式 (VGA输出)
graphic: $(OS_IMAGE)
	@echo "=== Graphic Mode (VGA Output) ==="
	$(QEMU) -m $(QEMU_MEMORY) \
	        -drive format=raw,file=$(OS_IMAGE) \
	        -vga std

# 开发模式 (构建带串口支持并运行)
dev: clean
	@make ENABLE_SERIAL=1 DEBUG=1
	@make debug

# 发布模式 (构建不带串口的镜像)
release: clean
	@make ENABLE_SERIAL=0 DEBUG=0
	@make graphic

# 串口测试模式
serial-test: clean
	@echo "=== Serial Test Mode ==="
	@make ENABLE_SERIAL=1 DEBUG=1
	@echo "Testing serial output..."
	$(QEMU) -m $(QEMU_MEMORY) \
	        -drive format=raw,file=$(OS_IMAGE) \
	        -nographic \
	        -serial mon:stdio \
	        -d guest_errors 2>&1 | tee serial_test.log

# GDB调试模式
gdb: $(OS_IMAGE)
	@echo "Starting QEMU in GDB debug mode..."
	$(QEMU) -m $(QEMU_MEMORY) \
	        -drive format=raw,file=$(OS_IMAGE) \
	        -s -S \
	        -nographic \
	        -serial stdio

.PHONY: all clean run run-16 run-32 run-64 run-128 build-16 build-64 build-128 debug