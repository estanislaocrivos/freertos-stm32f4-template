#include "logging.h"

#include "uart2.h"

#include <stdint.h>

#define MAX_LOG_BUFFER_SIZE 128

static char    g_buffer[MAX_LOG_BUFFER_SIZE];
static uint8_t g_buffer_index = 0;

void log_init(void)
{
    uart_init();
}

static void build_message(const char* prefix, const char* str)
{
    g_buffer_index = 0;
    while (*prefix != '\0')
    {
        g_buffer[g_buffer_index] = *prefix;
        prefix += 1;
        g_buffer_index += 1;
    }
    while (*str != '\0')
    {
        g_buffer[g_buffer_index] = *str;
        str += 1;
        g_buffer_index += 1;
    }
}

void log_info(const char* info)
{
    const char* prefix = "[INFO] ";
    build_message(prefix, info);
    uart_send(g_buffer);
}

void log_warn(const char* warn)
{
    const char* prefix = "[WARN] ";
    build_message(prefix, warn);
    uart_send(g_buffer);
}

void log_err(const char* err)
{
    const char* prefix = "[ERROR] ";
    build_message(prefix, err);
    uart_send(g_buffer);
}
