// kernel/kernel.c
#include "interrupt.h"
#include "memory.h"
#include "timer.h"
#include "keyboard.h"
#include "heap.h"
#include "stdio.h"      // 统一输出接口
#include "logging.h"
#include "cmdline.h"

void kernel_main(void) {
    // 1. 清屏
    clear_screen();
    
    // 2. 初始化标准I/O（必须最先初始化）
    stdio_init();
    
    // 3. 初始化日志系统
    log_init();
    
    // 4. 显示启动横幅（使用统一接口）
    printf_color(make_color(CYAN, BLACK), "=========================================\n");
    printf_color(make_color(YELLOW, BLACK), "              MyOS Kernel               \n");
    printf_color(make_color(CYAN, BLACK), "=========================================\n\n");
    
    log_info("SYSTEM", "MyOS Kernel Starting...");
    
    // 5. 初始化其他系统
    idt_init();
    init_pic();
    install_timer_interrupt();
    install_keyboard_interrupt();
    memory_init();
    init_timer();
    keyboard_init();
    
    // 6. 测试
    // test_heap_allocator();
    // test_stdio_functions();
    
    // 7. 启用中断
    // asm volatile("sti");
    
    // 8. 初始化命令行
    cmdline_init();

    log_info("SYSTEM", "System ready. Type 'help' for commands.");
    printf_color(make_color(CYAN, BLACK), "os> ");
    
    // 9. 主循环
    while (1) {
        asm volatile("hlt");
        process_command_line();
    }
}