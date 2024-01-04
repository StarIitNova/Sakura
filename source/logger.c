#include "logger.h"

#include <string.h>

SakuraLogger GlobalLogger;

void sakuraLoggerInit(void) {
    GlobalLogger.callstack = (char **)malloc(sizeof(char *) * 128);
    GlobalLogger.callstackCapacity = 128;
    GlobalLogger.callstackSize = 0;

    GlobalLogger.useColors = getenv("TERM") != NULL || getenv("ANSICON") != NULL || getenv("ConEmuANSI") != NULL ||
                             getenv("WT_SESSION") != NULL ||
                             (getenv("TERM_PROGRAM") && strcmp(getenv("TERM_PROGRAM"), "vscode") == 0) ||
                             getenv("COLORTERM") != NULL;
}

void sakuraLogger_insertCallStack(SakuraLogger *logger, const char *funcName, const char *filename, int line) {
    char *allocVal;

    if (logger->callstackSize == logger->callstackCapacity) {
        logger->callstackCapacity *= 2;
        logger->callstack = (char **)realloc(logger->callstack, sizeof(char *) * logger->callstackCapacity);
    }

    allocVal = (char *)malloc(sizeof(char) * (strlen(funcName) + strlen(filename) + 32 + logger->useColors * 8 * 6));
    if (logger->useColors)
        sprintf(allocVal, "\x1b[36m%s\x1b[0m at \x1b[35m%s\x1b[0m:\x1b[33m%d\x1b[0m", funcName, filename, line);
    else
        sprintf(allocVal, "%s at %s:%d", funcName, filename, line);

    logger->callstack[logger->callstackSize++] = allocVal;
}

void sakuraLogger_popCallStack(SakuraLogger *logger) { free((void *)logger->callstack[--logger->callstackSize]); }

void sakuraLoggerClose(void) {
    for (size_t i = 0; i < GlobalLogger.callstackSize; i++) {
        free((void *)GlobalLogger.callstack[i]);
    }

    free(GlobalLogger.callstack);
    GlobalLogger.callstack = NULL;
    GlobalLogger.callstackCapacity = 0;
    GlobalLogger.callstackSize = 0;
}

void sakuraLogger_dumpCallStack(SakuraLogger *logger) {
    if (logger->useColors)
        printf("\x1b[31m[Debugger]:\x1b[0m Full callstack dump\n");
    else
        printf("[Debugger]: Full callstack dump\n");
    for (size_t i = 0; i < logger->callstackSize; i++) {
        if (logger->useColors) {
            printf("    \x1b[1;32m[%lld]\x1b[0m  %s\n", i, logger->callstack[i]);
            continue;
        }

        printf("    [%lld]  %s\n", i, logger->callstack[i]);
    }
}

void sakura_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if (GlobalLogger.useColors) {
        vprintf(fmt, args);
    } else {
        char *output = (char *)malloc(strlen(fmt) + 1);
        char *outputPtr = output;

        if (!output) {
            printf("Error: failed to allocate memory for sakura_printf\n");
            exit(1);
        }

        while (*fmt != '\0') {
            if (*fmt == '\x1b') {
                while (*fmt != 'm' && *fmt != '\0') {
                    fmt++;
                }
            } else {
                *outputPtr++ = *fmt;
            }

            fmt++;
        }

        *outputPtr = '\0';

        vprintf(output, args);
        free(output);
    }

    va_end(args);
}