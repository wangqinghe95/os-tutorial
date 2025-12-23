#include "logging.h"
#include "stdio.h"

log_level_t current_log_level = LOG_INFO;

static const char* level_name[] = {"DEBUG", "INFO", "WARN", "EROR", "FATAL"};
static const uint8_t level_colors[] = {
    LIGHT_GRAY, WHITE, YELLOW, LIGHT_RED, RED
};

void log_init()
{
    stdio_init();
    log_info("LOGGING", "Logging system initialized");
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

    printf_color(make_color(color, BLACK), "%s", buffer);
    putchar('\n');
}

void set_log_level(log_level_t level) {
    current_log_level = level;
    log_info("LOGGING", "Log level set to %s", level_name[level]);
}