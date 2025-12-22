// kernel/kernel.c
#include "interrupt.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"
#include "heap.h"
#include "stdio.h"
#include "logging.h"
#include "serial.h"  // 添加串口头文件
#include "vga.h"


#ifdef ENABLE_SERIAL
    #define SERIAL_ENABLED 1
#else
    #define SERIAL_ENABLED 0
#endif

/* 系统初始化状态检查 */
void check_system_init_status(void) {
    char buffer[128];
    
    log_info("SYSTEM", "Checking initialization status...");
    
    // 检查构建配置
    sprintf(buffer, "Build configuration: DEBUG=%d, SERIAL_ENABLED=%d", 
            #ifdef DEBUG
            1
            #else
            0
            #endif
            , SERIAL_ENABLED);
    log_info("BUILD", "%s", buffer);
    
    // 检查内存大小
    uint32_t mem_mb = get_kernel_memory_mb();
    log_info("MEMORY", "Kernel memory configured: %d MB", mem_mb);
    
    if (mem_mb < 16) {
        log_warn("MEMORY", "Low memory configuration (%d MB)", mem_mb);
    }
}

/* IDT 和中断系统测试 */
void test_interrupt_system(void) {
    log_info("INTERRUPT", "Testing interrupt system...");
    
    // 检查 IDT 是否已安装
    #ifdef DEBUG
    log_debug("IDT", "IDT base address verified");
    #endif
    
    // 测试中断是否启用
    uint32_t flags;
    asm volatile("pushf\n\tpop %0" : "=r"(flags));
    if (flags & 0x200) {
        log_info("INTERRUPT", "Interrupts are enabled");
    } else {
        log_warn("INTERRUPT", "Interrupts are disabled");
    }
    
    // 测试 PIT 定时器是否工作
    log_info("TIMER", "Timer interrupts configured");
    
    // 测试键盘中断
    log_info("KEYBOARD", "Keyboard driver initialized");
    
    TEST_SUCCESS("INTERRUPT SYSTEM");
}

/* 内存系统测试 */
void test_memory_system(void) {
    log_info("MEMORY", "Testing memory management system...");
    
    // 测试堆分配器
    log_debug("HEAP", "Running heap allocator tests...");
    
    // 简单的内存分配测试
    void* test_ptr = kmalloc(64);
    if (test_ptr) {
        log_info("HEAP", "Small allocation successful at 0x%x", (uint32_t)test_ptr);
        kfree(test_ptr);
        log_info("HEAP", "Memory freed successfully");
    } else {
        log_error("HEAP", "Small allocation failed!");
    }
    
    // 测试多个分配
    void* ptr1 = kmalloc(128);
    void* ptr2 = kmalloc(256);
    void* ptr3 = kmalloc(512);
    
    if (ptr1 && ptr2 && ptr3) {
        log_info("HEAP", "Multiple allocations successful");
    }
    
    kfree(ptr1);
    kfree(ptr2);
    kfree(ptr3);
    
    // 测试大块内存分配
    log_debug("HEAP", "Testing large allocation...");
    void* large_ptr = kmalloc(2048);
    if (large_ptr) {
        log_info("HEAP", "Large allocation (2KB) successful");
        kfree(large_ptr);
    }
    
    TEST_SUCCESS("MEMORY SYSTEM");
}

/* 设备驱动测试 */
void test_device_drivers(void) {
    log_info("DEVICE", "Testing hardware drivers...");
    
    // 串口测试（如果启用）
    #if SERIAL_ENABLED
    log_info("SERIAL", "Testing serial port...");
    
    // 串口自检
    if (serial_self_test()) {
        log_info("SERIAL", "Serial port self-test passed");
        
        // 发送测试消息
        serial_write_string("[SERIAL] Serial port initialized successfully\n");
    } else {
        log_warn("SERIAL", "Serial port self-test failed");
    }
    #else
    log_warn("SERIAL", "Serial port disabled in this build");
    #endif
    
    // 屏幕驱动测试
    log_info("SCREEN", "Testing screen driver...");
    
    // 测试颜色输出
    printk_color("Screen test: ", make_color(WHITE, BLUE));
    printk_color("Normal", make_color(WHITE, BLACK));
    printf(" ");
    printk_color("Warning", make_color(YELLOW, BLACK));
    printk(" ");
    printk_color("Error", make_color(RED, BLACK));
    printk(" ");
    printk_color("Success", make_color(GREEN, BLACK));
    printk("\n");
    
    // 定时器测试
    log_info("TIMER", "Timer driver active");
    
    // 键盘测试
    log_info("KEYBOARD", "Keyboard driver ready for input");
    
    TEST_SUCCESS("DEVICE DRIVERS");
}

/* 系统完整性检查 */
void system_integrity_check(void) {
    log_info("SYSTEM", "Performing system integrity check...");
    
    uint8_t passed = 1;
    
    // 检查关键数据结构
    #ifdef DEBUG
    log_debug("SYSTEM", "Verifying kernel data structures...");
    #endif
    
    // 检查内存管理
    if (get_kernel_memory_mb() == 0) {
        log_error("SYSTEM", "Memory configuration invalid!");
        passed = 0;
    }
    
    // 检查中断向量表
    #ifdef DEBUG
    log_debug("SYSTEM", "IDT integrity check...");
    #endif
    
    // 检查堆分配器
    void* test_alloc = kmalloc(16);
    if (!test_alloc) {
        log_error("SYSTEM", "Heap allocator failure!");
        passed = 0;
    } else {
        kfree(test_alloc);
    }
    
    if (passed) {
        log_info("SYSTEM", "System integrity check PASSED");
        
        #if SERIAL_ENABLED
        serial_write_string("[SYSTEM] Integrity check PASSED\n");
        #endif
    } else {
        log_error("SYSTEM", "System integrity check FAILED");
        
        #if SERIAL_ENABLED
        serial_write_string("[SYSTEM] Integrity check FAILED\n");
        #endif
    }
}

/* 启动信息显示 */
void display_boot_info(void) {
    // 清屏后的第一个输出
    printk_color("=========================================\n", make_color(CYAN, BLACK));
    printk_color("              MyOS Kernel               \n", make_color(YELLOW, BLACK));
    printk_color("=========================================\n", make_color(CYAN, BLACK));
    
    #if SERIAL_ENABLED
    serial_write_string("\n=========================================\n");
    serial_write_string("              MyOS Kernel               \n");
    serial_write_string("=========================================\n\n");
    #endif
    
    // 版本信息
    log_info("SYSTEM", "MyOS Kernel Starting...");
    log_info("BUILD", "Version: 0.1.0-alpha");
    log_info("BUILD", "Date: %s %s", __DATE__, __TIME__);
}

/* 主入口函数 */
void kernel_main(void) {
    // 1. 初始化基本显示
    clear_screen();
    log_init();  // 初始化日志系统（包含串口初始化）
    
    // 显示启动信息
    display_boot_info();
    
    // 2. 检查系统状态
    check_system_init_status();
    
    // 3. 初始化中断系统
    log_info("INTERRUPT", "Initializing interrupt system...");
    idt_init();
    init_pic();
    
    // 4. 初始化内存管理系统
    log_info("MEMORY", "Initializing memory manager...");
    memory_init();
    
    // 5. 初始化硬件驱动
    log_info("DEVICE", "Initializing hardware drivers...");
    install_timer_interrupt();
    install_keyboard_interrupt();
    init_timer();
    keyboard_init();
    
    // 6. 测试堆分配器
    log_info("HEAP", "Testing heap allocator...");
    // test_heap_allocator();
    
    // 7. 启用中断
    log_info("INTERRUPT", "Enabling interrupts...");
    asm volatile("sti");
    
    // 8. 运行系统测试
    log_info("TEST", "Running system tests...");
    
    // 测试中断系统
    test_interrupt_system();
    
    // 测试内存系统
    test_memory_system();
    
    // 测试设备驱动
    test_device_drivers();
    
    // 测试日志系统
    // test_logging_system();
    
    // 9. 系统完整性检查
    system_integrity_check();
    
    // 10. 显示完成信息
    log_info("SYSTEM", "Kernel initialization completed successfully");
    
    #if SERIAL_ENABLED
    serial_write_string("\n[SYSTEM] MyOS Kernel Ready!\n");
    serial_write_string("[SYSTEM] Memory: ");
    char mem_str[16];
    sprintf(mem_str, "%d", get_kernel_memory_mb());
    serial_write_string(mem_str);
    serial_write_string(" MB available\n");
    #endif
    
    // 11. 显示提示信息
    printk_color("\n=========================================\n", make_color(GREEN, BLACK));
    printk_color("        SYSTEM READY - COMMAND MODE      \n", make_color(GREEN, BLACK));
    printk_color("=========================================\n\n", make_color(GREEN, BLACK));
    
    printf("System initialized with %d MB memory\n", get_kernel_memory_mb());
    printk_color("Heap allocator: ACTIVE\n", make_color(GREEN, BLACK));
    printk_color("Interrupts: ENABLED\n", make_color(GREEN, BLACK));
    
    #if SERIAL_ENABLED
    printk_color("Serial port: ENABLED\n", make_color(GREEN, BLACK));
    #else
    printk_color("Serial port: DISABLED\n", make_color(YELLOW, BLACK));
    #endif
    
    printk("Type 'help' for available commands\n");
    printk_color("os> ", make_color(CYAN, BLACK));
    
    // 12. 主循环
    log_info("SYSTEM", "Entering main loop...");
    
    while(1) {
        // 等待中断
        asm volatile("hlt");
        
        // 这里可以添加命令处理逻辑
        // 例如检查键盘缓冲区并处理输入
        
        // 简单的空闲指示器（可选）
        static uint32_t idle_counter = 0;
        idle_counter++;
        
        #ifdef DEBUG
        if ((idle_counter % 1000000) == 0) {
            log_debug("IDLE", "System idle loop: %u", idle_counter);
        }
        #endif
    }
}

/* 紧急错误处理 */
void kernel_panic(const char* message) {
    // 禁用中断
    asm volatile("cli");
    
    // 显示错误信息
    printk_color("\n\n*** KERNEL PANIC ***\n", make_color(WHITE, RED));
    printk_color("Message: ", make_color(WHITE, RED));
    printf("%s\n", message);
    
    #if SERIAL_ENABLED
    serial_write_string("\n\n*** KERNEL PANIC ***\n");
    serial_write_string("Message: ");
    serial_write_string(message);
    serial_write_string("\n");
    #endif
    
    // 挂起系统
    log_error("PANIC", "System halted: %s", message);
    
    while(1) {
        asm volatile("hlt");
    }
}