// drivers/keyboard.c
#include "keyboard.h"
#include "port.h"
#include "stdio.h"
#include "logging.h"

/* 键盘扫描码到ASCII映射（US键盘布局） */
static const char keyboard_map_normal[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char keyboard_map_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/* 键盘状态 */
static uint8_t shift_pressed = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;
static uint8_t caps_lock = 0;

/* 键盘缓冲区（环形缓冲区） */
#define KEY_BUFFER_SIZE 256
static char key_buffer[KEY_BUFFER_SIZE];
static uint32_t buffer_head = 0;
static uint32_t buffer_tail = 0;
static uint32_t buffer_count = 0;

/* 当前输入行缓冲区（用于命令行） */
#define INPUT_BUFFER_SIZE 256
static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_index = 0;

/* 向键盘缓冲区添加字符 */
static void keyboard_buffer_put(char c) {
    if (buffer_count < KEY_BUFFER_SIZE) {
        key_buffer[buffer_tail] = c;
        buffer_tail = (buffer_tail + 1) % KEY_BUFFER_SIZE;
        buffer_count++;
        
        #ifdef DEBUG
        if (c >= 32 && c < 127) {
            log_debug("KEYBOARD", "Buffer put: '%c' (0x%02x)", c, c);
        } else {
            log_debug("KEYBOARD", "Buffer put: special 0x%02x", c);
        }
        #endif
    } else {
        log_warn("KEYBOARD", "Keyboard buffer full");
    }
}

/* 从键盘缓冲区获取字符 */
char keyboard_getc(void) {
    if (buffer_count == 0) {
        return 0;
    }
    
    char c = key_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % KEY_BUFFER_SIZE;
    buffer_count--;
    
    return c;
}

/* 检查是否有字符可读 */
int keyboard_available(void) {
    return buffer_count > 0;
}

/* 清空键盘缓冲区 */
void keyboard_clear_buffer(void) {
    buffer_head = 0;
    buffer_tail = 0;
    buffer_count = 0;
    input_index = 0;
    input_buffer[0] = '\0';
}

/* 处理特殊功能键 */
static void handle_special_key(uint8_t scancode, int pressed) {
    switch (scancode) {
        case 0x2A:  // 左Shift按下
        case 0x36:  // 右Shift按下
            shift_pressed = pressed;
            break;
            
        case 0x1D:  // Ctrl按下/释放
            ctrl_pressed = pressed;
            break;
            
        case 0x38:  // Alt按下/释放
            alt_pressed = pressed;
            break;
            
        case 0x3A:  // Caps Lock
            if (pressed) {
                caps_lock = !caps_lock;
                log_info("KEYBOARD", "Caps Lock: %s", caps_lock ? "ON" : "OFF");
            }
            break;
            
        case 0x45:  // Num Lock
        case 0x46:  // Scroll Lock
            // 暂时忽略
            break;
    }
}

/* 扫描码转ASCII */
static char scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) return 0;
    
    const char* map;
    
    if (shift_pressed ^ caps_lock) {  // Shift或Caps Lock按下
        map = keyboard_map_shift;
    } else {
        map = keyboard_map_normal;
    }
    
    char c = map[scancode];
    
    // Ctrl组合键处理
    if (ctrl_pressed && c >= 'a' && c <= 'z') {
        return c - 'a' + 1;  // Ctrl+A = 1, Ctrl+B = 2, ...
    }
    
    return c;
}

/* 处理输入回显 */
static void handle_input_echo(char c) {
    if (c == '\b') {  // 退格键
        if (input_index > 0) {
            input_index--;
            // 使用 stdio 的统一输出
            putchar('\b');
            putchar(' ');
            putchar('\b');
        }
    } 
    else if (c == '\n') {  // 回车键
        putchar('\n');
        
        if (input_index > 0) {
            input_buffer[input_index] = '\0';
            printf("You typed: %s\n", input_buffer);
            input_index = 0;
        }
    }
    else if (c >= 32 && c < 127) {  // 可打印字符
        if (input_index < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_index++] = c;
            putchar(c);  // 回显
        }
    }
    else if (c == 3) {  // Ctrl+C
        printf("^C\n");
        input_index = 0;
        input_buffer[0] = '\0';
    }
}

/* 键盘中断处理程序 */
void keyboard_interrupt_handler(void) {
    // 读取状态寄存器
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (!(status & 0x01)) {
        // 没有数据，直接返回
        outb(0x20, 0x20);  // 发送EOI
        return;
    }
    
    // 读取扫描码
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // 判断是按键按下还是释放
    int pressed = 1;
    uint8_t keycode = scancode;
    
    if (scancode & 0x80) {
        // 按键释放（最高位为1）
        pressed = 0;
        keycode = scancode & 0x7F;
    }
    
    // 处理特殊功能键
    handle_special_key(keycode, pressed);
    
    // 只处理按键按下事件
    if (pressed) {
        char ascii = scancode_to_ascii(keycode);
        
        if (ascii != 0) {
            // 添加到缓冲区
            keyboard_buffer_put(ascii);
            
            // 处理回显
            handle_input_echo(ascii);
        }
        else {
            // 特殊键处理
            switch (keycode) {
                case 0x0E:  // Backspace
                    keyboard_buffer_put('\b');
                    handle_input_echo('\b');
                    break;
                    
                case 0x1C:  // Enter
                    keyboard_buffer_put('\n');
                    handle_input_echo('\n');
                    break;
                    
                case 0x39:  // Space
                    keyboard_buffer_put(' ');
                    handle_input_echo(' ');
                    break;
                    
                case 0x0F:  // Tab
                    keyboard_buffer_put('\t');
                    putchar('\t');
                    break;
            }
        }
    }
    
    // 发送EOI
    outb(0x20, 0x20);
}

/* 键盘初始化 */
void keyboard_init(void) {
    // 清空缓冲区
    keyboard_clear_buffer();
    
    // 重置状态
    shift_pressed = 0;
    ctrl_pressed = 0;
    alt_pressed = 0;
    caps_lock = 0;
    
    // 启用键盘中断
    uint8_t mask = inb(0x21);
    mask &= 0xFD;  // 清除IRQ1位（键盘）
    outb(0x21, mask);
    
    log_info("KEYBOARD", "Keyboard driver initialized");
}

/* 获取当前输入行 */
const char* keyboard_get_input_line(void) {
    if (input_index == 0) {
        return "";
    }
    input_buffer[input_index] = '\0';
    return input_buffer;
}

/* 读取一行输入（阻塞） */
int keyboard_readline(char* buffer, uint32_t size) {
    uint32_t index = 0;
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getc();
            
            if (c == '\n') {
                buffer[index] = '\0';
                return index;
            }
            else if (c == '\b') {
                if (index > 0) {
                    index--;
                }
            }
            else if (c >= 32 && c < 127) {
                if (index < size - 1) {
                    buffer[index++] = c;
                }
            }
            
            // 回显
            putchar(c);
        }
    }
}

/* 键盘自测试 */
void keyboard_self_test(void) {
    log_info("KEYBOARD", "Starting keyboard self-test...");
    
    printf("Keyboard Test Mode (Press ESC to exit)\n");
    printf("Type some characters:\n");
    
    uint32_t key_count = 0;
    uint8_t last_scancode = 0;
    
    while (1) {
        if (keyboard_available()) {
            char c = keyboard_getc();
            
            if (c == 27) {  // ESC键
                break;
            }
            
            key_count++;
            printf("Key %u: ", key_count);
            
            if (c >= 32 && c < 127) {
                printf("'%c' ", c);
            }
            
            printf("(0x%02x)\n", c);
            
            if (key_count >= 20) {
                printf("Test completed.\n");
                break;
            }
        }
    }
    
    log_info("KEYBOARD", "Keyboard test completed, %u keys pressed", key_count);
}

/* 简单的getchar实现（阻塞） */
char keyboard_getchar(void) {
    while (!keyboard_available()) {
        // 等待键盘输入
        asm volatile("hlt");
    }
    return keyboard_getc();
}