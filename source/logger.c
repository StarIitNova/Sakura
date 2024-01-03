#include "logger.h"

#include <string.h>

SakuraLogger GlobalLogger;

void sakuraLoggerInit(void) {
    GlobalLogger.callstack = (char **)malloc(sizeof(char *) * 128);
    GlobalLogger.callstackCapacity = 128;
    GlobalLogger.callstackSize = 0;

    GlobalLogger.useColors = getenv("TERM") != NULL || getenv("ANSICON") != NULL || getenv("ConEmuANSI") != NULL ||
                             getenv("WT_SESSION") != NULL || strcmp(getenv("TERM_PROGRAM"), "vscode") == 0 ||
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
        sprintf(allocVal, "\033[36m%s\033[0m at \033[35m%s\033[0m:\033[33m%d\033[0m", funcName, filename, line);
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
        printf("\033[31m[Debugger]:\033[0m Full callstack dump\n");
    else
        printf("[Debugger]: Full callstack dump\n");
    for (size_t i = 0; i < logger->callstackSize; i++) {
        if (logger->useColors) {
            printf("    \033[1;32m[%lld]\033[0m  %s\n", i, logger->callstack[i]);
            continue;
        }

        printf("    [%lld]  %s\n", i, logger->callstack[i]);
    }
}