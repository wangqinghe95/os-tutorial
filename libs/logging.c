#include "logging.h"
#include "stdio.h"
#include "serial.h"

log_level_t current_log_level = LOG_INFO;

static const char* level_name[] = {"DEBUG", "INFO", "WARN", "EROR", "FATAL"};
static const uint8_t level_colors[] = {
    LIGHT_GRAY, WHITE, YELLOW, LIGHT_RED, RED
};

void log_init()
{
    #if SERIAL_ENABLED
    serial_init();
    #endif
    
    char buffer[80];
    sprintf(buffer, "[LOGGING] Logging system initialized (SERIAL_ENABLED=%d)", SERIAL_ENABLED);
    
    // 同时输出到屏幕和串口
    printk_color(buffer, make_color(LIGHT_BLUE, BLACK));
    printk("\n");
    
    #if SERIAL_ENABLED
    serial_write_string(buffer);
    serial_write_char('\n');
    #endif
    
    log_debug("LOGGING", "Logging system initialized");
}

void log_message(log_level_t level, const char* tag, const char* format, ...)
{
    if(level < current_log_level) return;

    char buffer[256];
    va_list args;
    uint8_t color = make_color(level_colors[level], BLACK);

    char* ptr = buffer;
    ptr += sprintf(ptr, "[%5s] %8s: ", level_name[level], tag);

    va_start(args, format);
    ptr += vsprintf(ptr, format, args);
    va_end(args);

    printk_color(buffer, color);
    put_char('\n', color);

    #if SERIAL_ENABLED
    serial_write_string(buffer);
    serial_write_char('\n');
    #endif
}

/* 串口功能测试函数 */
void test_serial_functions(void) {
    printk_color("\n=== Serial Functions Test ===\n", make_color(CYAN, BLACK));
    
    #if SERIAL_ENABLED
    log_info("SERIAL", "Serial port enabled - testing functions");
    
    // 测试1: 基本输出
    serial_write_string("Serial Test 1: Basic string output\n");
    log_info("SERIAL", "Test 1 passed: Basic string output");
    
    // 测试2: 字符输出
    serial_write_char('A');
    serial_write_char('B');
    serial_write_char('C');
    serial_write_char('\n');
    log_info("SERIAL", "Test 2 passed: Character output");
    
    // 测试3: 格式化输出
    char buffer[64];
    sprintf(buffer, "Serial Test 3: Numbers - %d, Hex - 0x%x, String - %s\n", 
            12345, 0xABCD, "Hello Serial");
    serial_write_string(buffer);
    log_info("SERIAL", "Test 3 passed: Formatted output");
    
    // 测试4: 回显测试（如果串口输入可用）
    log_info("SERIAL", "Test 4: Echo test - type characters to test input");
    log_info("SERIAL", "Press 'q' to quit echo test");
    
    /*
    // 如果有串口输入功能，可以启用这段代码
    while(1) {
        if (serial_received()) {
            char c = serial_read_char();
            serial_write_char(c); // 回显
            
            if (c == 'q') {
                serial_write_string("\nEcho test completed.\n");
                break;
            }
        }
    }
    */
    
    log_info("SERIAL", "All serial tests completed");
    
    #else
    log_warn("SERIAL", "Serial port not enabled in this build");
    log_warn("SERIAL", "Use 'make dev' or 'make ENABLE_SERIAL=1' to enable");
    #endif
    
    printk_color("=== Serial Test Complete ===\n", make_color(GREEN, BLACK));
}

void log_hex_dump(const char* tag, const void* data, uint32_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    char line[80];
    
    // 输出头部信息
    sprintf(line, "[%s] Hex dump (%u bytes):", tag, size);
    printf("%s\n", line);
    
    for (uint32_t i = 0; i < size; i += 16) {
        char* ptr = line;
        uint32_t line_bytes = (size - i) < 16 ? 
                            (size - i) : 16;
        
        // 地址部分
        ptr += sprintf(ptr, "  %04x: ", i);
        
        // 十六进制部分
        for (uint32_t j = 0; j < 16; j++) {
            if (j < line_bytes) {
                ptr += sprintf(ptr, "%02x ", bytes[i + j]);
            } else {
                ptr += sprintf(ptr, "   ");
            }
            if (j == 7) {
                ptr += sprintf(ptr, " ");
            }
        }
        
        // ASCII部分
        ptr += sprintf(ptr, "|");
        for (uint32_t j = 0; j < line_bytes; j++) {
            char c = bytes[i + j];
            *ptr++ = (c >= 32 && c < 127) ? c : '.';
        }
        *ptr++ = '|';
        *ptr = '\0';
        
        printf("%s\n", line);
    }
}

void test_hex_dump() {
    // 测试字符串
    char test_str[] = "This is a test string for hex dump!\x01\x02\x7F\xFF";
    log_hex_dump("TEST_STR", test_str, sizeof(test_str));
    
    // 测试二进制数据
    uint8_t binary_data[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x01, 0x02, 0x03, 0x04
    };
    log_hex_dump("BINARY", binary_data, sizeof(binary_data));
}

void test_logging_system()
{
    
    printk_color("\n=== LOGGING System Tests ===\n", make_color(YELLOW, BLACK));
    
    #if SERIAL_ENABLED
    serial_write_string("\n=== LOGGING System Tests ===\n");
    #endif
    
    // 设置日志级别
    // set_log_level(LOG_DEBUG);
    
    // 测试不同日志级别
    log_debug("TEST", "This is a debug message");
    log_info("TEST", "This is an info message");
    log_warn("TEST", "This is a warning message");
    log_error("TEST", "This is an error message");
    
    // 测试格式化日志
    log_info("TEST", "Format test: %d + %d = %d", 2, 3, 5);
    log_info("TEST", "String: %s, Char: %c, Hex: 0x%x", "hello", 'X', 255);
    
    // 测试十六进制转储
    // log_info("TEST", "Testing hex dump...");
    // test_hex_dump();
    
    // 测试断言宏
    log_info("TEST", "Testing assertions...");
    TEST_ASSERT(1 == 1, "Basic assertion should pass");
    
    int x = 5, y = 5;
    TEST_ASSERT(x == y, "Variable comparison should pass");
    
    // 测试日志宏
    log_info("TEST", "Testing log macros...");
    LOG("Simple log message");
    LOG_FMT("Formatted log: %d, %s", 42, "test");
    TEST_PHASE("Test phase demonstration");
    TEST_SUCCESS("LOGGING SYSTEM");
    
    // 测试串口功能
    test_serial_functions();
    
    // 恢复默认日志级别
    // set_log_level(LOG_INFO);
    
    #if SERIAL_ENABLED
    serial_write_string("=== Logging Tests Completed ===\n");
    #endif
    
    printk_color("=== Logging Tests Completed ===\n", make_color(GREEN, BLACK));}