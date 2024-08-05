/********************************************************
 * Description : base
 * Author      : yanrk
 * Email       : yanrkchina@163.com
 * Blog        : blog.csdn.net/cxxmaker
 * Version     : 1.0
 * Copyright(C): 2024
 ********************************************************/

#include <unistd.h>
#include <ctime>
#include <cstdarg>
#include "base.h"

static int s_log_max_level = 3;
static bool s_log_simplify = false;

void set_log_max_level(int level)
{
    s_log_max_level = level;
}

void set_log_simplify(bool simplify)
{
    s_log_simplify = simplify;
}

void run_log(int level, const char * file, const char * func, int line, const char * format, ...)
{
    if (level > s_log_max_level)
    {
        return;
    }

    char buffer[1024] = { 0x0 };
    size_t size = 0;
    if (!s_log_simplify)
    {
        size = snprintf(buffer, sizeof(buffer) - 1, "%s:%s:%d | ", file, func, line);
    }

    va_list args;
    va_start(args, format);
    vsnprintf(buffer + size, sizeof(buffer) - 1 - size, format, args);
    va_end(args);

    printf("%s\n", buffer);
}

void sleep_ms(uint32_t ms)
{
    usleep(1000 * ms);
}
