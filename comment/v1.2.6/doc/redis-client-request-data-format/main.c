#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "sds.h"

sds sdscatrepr(sds s, const char *p, size_t len) {
    s = sdscatlen(s, "\"", 1);
    while (len--) {
        switch (*p) {
            case '\\':
            case '"':
                s = sdscatprintf(s, "\\%c", *p);
                break;
            case '\n':
                s = sdscatlen(s, "\\n", 2);
                break;
            case '\r':
                s = sdscatlen(s, "\\r", 2);
                break;
            case '\t':
                s = sdscatlen(s, "\\t", 2);
                break;
            case '\a':
                s = sdscatlen(s, "\\a", 2);
                break;
            case '\b':
                s = sdscatlen(s, "\\b", 2);
                break;
            default:
                if (isprint(*p))
                    s = sdscatprintf(s, "%c", *p);
                else
                    s = sdscatprintf(s, "\\x%02x", (unsigned char) *p);
                break;
        }
        p++;
    }
    return sdscatlen(s, "\"", 1);
}

void redisCommand(int fd, char *format, ...) {
    va_list ap;
    size_t size;
    char *arg, *c = format;
    sds cmd = sdsempty();

    /* Build the command string accordingly to protocol */
    va_start(ap, format);
    while (*c != '\0') {
        if (*c != '%' || c[1] == '\0') {
            cmd = sdscatlen(cmd, c, 1);
        } else {
            switch (c[1]) {
                case 's':
                    arg = va_arg(ap, char*);
                    cmd = sdscat(cmd, arg);
                    break;
                case 'b':
                    arg = va_arg(ap, char*);
                    size = va_arg(ap, size_t);
                    cmd = sdscatprintf(cmd, "%zu\r\n", size);
                    cmd = sdscatlen(cmd, arg, size);
                    break;
                case '%':
                    cmd = sdscat(cmd, "%");
                    break;
            }
            c++;
        }
        c++;
    }
    cmd = sdscat(cmd, "\r\n");
    va_end(ap);

    sds show = sdscatrepr(sdsempty(), cmd, sdslen(cmd));
    printf("%s\n", show);
    sdsfree(show);
    sdsfree(cmd);
}

int main(int argc, char **argv) {
    redisCommand(5, "SET %s %b", "wufeihu", "fix", strlen("fix"));
    redisCommand(5, "SET %%");
    redisCommand(5, "hset %s %s", "a", "b");
    redisCommand(5, "%");
    redisCommand(5, "%%");
    redisCommand(5, "A%%B");
    redisCommand(5, "A%CDE");
    redisCommand(5, "PING");
    return 0;
}