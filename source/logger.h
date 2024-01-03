#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char **callstack;
    size_t callstackSize;
    size_t callstackCapacity;
    int useColors;
} SakuraLogger;

extern SakuraLogger GlobalLogger;

void sakuraLoggerInit(void);
void sakuraLogger_insertCallStack(SakuraLogger *logger, const char *funcName, const char *filename, int line);
void sakuraLogger_popCallStack(SakuraLogger *logger);
void sakuraLoggerClose(void);

void sakuraLogger_dumpCallStack(SakuraLogger *logger);

#ifdef SAKURA_DEBUG
#define LOG_CALL() sakuraLogger_insertCallStack(&GlobalLogger, __func__, __FILE__, __LINE__)
#define LOG_POP() sakuraLogger_popCallStack(&GlobalLogger)
#define LOG_DUMP() sakuraLogger_dumpCallStack(&GlobalLogger)
#else
#define LOG_CALL()
#define LOG_POP()
#define LOG_DUMP()
#endif