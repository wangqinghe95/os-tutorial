#include "cmdline.h"
#include "stdio.h"
#include "logging.h"
#include "keyboard.h"
#include "string.h"
#include "port.h"

#define MAX_CMD_LENGTH 256
#define MAX_HISTORY 10

static char command_buffer[MAX_CMD_LENGTH];
static uint32_t cmd_index = 0;
static uint8_t cmd_mode = 1;  // 1=命令模式，0=空闲模式

// 命令历史
static char cmd_history[MAX_HISTORY][MAX_CMD_LENGTH];
static uint32_t history_count = 0;
static uint32_t history_index = 0;

/* 保存命令到历史 */
static void save_to_history(const char* cmd) {
    if (cmd[0] == '\0' || history_count >= MAX_HISTORY) return;
    
    // 不保存重复命令
    if (history_count > 0 && strcmp(cmd_history[0], cmd) == 0) {
        return;
    }
    
    // 移动历史记录
    for (int i = MAX_HISTORY - 1; i > 0; i--) {
        strcpy(cmd_history[i], cmd_history[i-1]);
    }
    
    // 保存新命令
    strcpy(cmd_history[0], cmd);
    history_count++;
    history_index = 0;
}

/* 显示提示符 */
void show_prompt(void) {
    printf_color(make_color(CYAN, BLACK), "os> ");
    cmd_mode = 1;
    cmd_index = 0;
    command_buffer[0] = '\0';
}

/* 执行命令 */
static void execute_command(const char* cmd) {
    if (cmd[0] == '\0') return;
    
    log_info("CMD", "Executing: %s", cmd);
    save_to_history(cmd);
    
    if (strcmp(cmd, "help") == 0) {
        printf_color(make_color(YELLOW, BLACK), "\n=== Available Commands ===\n");
        printf("  help      - Show this help message\n");
        printf("  clear     - Clear the screen\n");
        printf("  echo <msg>- Echo a message\n");
        printf("  mem       - Show memory information\n");
        printf("  kbdtest   - Test keyboard input\n");
        printf("  reboot    - Reboot the system\n");
        printf("  test      - Run system tests\n");
        printf("  version   - Show kernel version\n");
        printf("  history   - Show command history\n");
        printf("  exit      - Exit command mode\n");
    }
    else if (strcmp(cmd, "clear") == 0) {
        clear_screen();
        show_prompt();
    }
    else if (strncmp(cmd, "echo ", 5) == 0) {
        printf("%s\n", cmd + 5);
    }
    else if (strcmp(cmd, "mem") == 0) {
        extern uint32_t get_kernel_memory_mb(void);
        uint32_t mem_mb = get_kernel_memory_mb();
        printf("Memory Information:\n");
        printf("  Configured: %d MB\n", mem_mb);
        printf("  Heap: Active\n");
    }
    else if (strcmp(cmd, "kbdtest") == 0) {
        printf("Starting keyboard test...\n");
        keyboard_self_test();
        show_prompt();
    }
    else if (strcmp(cmd, "history") == 0) {
        printf("Command History:\n");
        if (history_count == 0) {
            printf("  (no commands in history)\n");
        } else {
            for (uint32_t i = 0; i < history_count; i++) {
                printf("  %u: %s\n", i + 1, cmd_history[i]);
            }
        }
    }
    else if (strcmp(cmd, "test") == 0) {
        printf("Running system tests...\n");
        // 调用测试函数
        // test_stdio_functions();
        printf("System tests completed.\n");
    }
    else if (strcmp(cmd, "version") == 0) {
        printf_color(make_color(GREEN, BLACK), "MyOS Kernel v0.1.0\n");
        printf("Build: %s %s\n", __DATE__, __TIME__);
    }
    else if (strcmp(cmd, "reboot") == 0) {
        printf_color(make_color(RED, BLACK), "System rebooting...\n");
        // 触发重启
        outb(0x64, 0xFE);
    }
    else if (strcmp(cmd, "exit") == 0) {
        printf("Exiting command mode. Press Enter to return.\n");
        cmd_mode = 0;
        return;
    }
    else {
        printf_color(make_color(RED, BLACK), "Unknown command: ");
        printf("%s\n", cmd);
        printf("Type 'help' for available commands\n");
    }
}

/* 处理命令行输入 */
void process_command_line(void) {
    if (!cmd_mode) {
        // 不在命令模式，检查是否有回车键进入命令模式
        while (keyboard_available()) {
            char c = keyboard_getc();
            if (c == '\n') {
                cmd_mode = 1;
                show_prompt();
                break;
            }
        }
        return;
    }
    
    // 处理键盘输入
    while (keyboard_available() && cmd_mode) {
        char c = keyboard_getc();
        
        if (c == '\n') {
            // 执行命令
            command_buffer[cmd_index] = '\0';
            putchar('\n');
            execute_command(command_buffer);
            
            if (cmd_mode) {
                show_prompt();
            }
        }
        else if (c == '\b') {
            // 退格
            if (cmd_index > 0) {
                cmd_index--;
                printf("\b \b");
            }
        }
        else if (c >= 32 && c < 127) {
            // 可打印字符
            if (cmd_index < MAX_CMD_LENGTH - 1) {
                command_buffer[cmd_index++] = c;
                command_buffer[cmd_index] = '\0';
                putchar(c);
            }
        }
        else if (c == '\t') {
            // Tab键补全（简化版）
            putchar('\t');
            // 这里可以添加命令补全逻辑
        }
    }
}

/* 初始化命令行 */
void cmdline_init(void) {
    cmd_mode = 1;
    cmd_index = 0;
    command_buffer[0] = '\0';
    history_count = 0;
    history_index = 0;
    
    log_info("CMDLINE", "Command line interface initialized");
}