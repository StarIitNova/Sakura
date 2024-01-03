#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sakura.h"
#include "sap.h"

#ifndef SAKURA_VERSION
#define SAKURA_VERSION "UNKNOWN"
#endif

SakuraState *currentState;

void onSignal(int signum);
void dumpDebugStateInfo(SakuraState *S);

#ifdef _WIN32
#include <Windows.h>

LONG WINAPI CriticalExceptionHandler(EXCEPTION_POINTERS *ExceptionInfo);

LONG WINAPI CriticalExceptionHandler(EXCEPTION_POINTERS *ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_HEAP_CORRUPTION) {
        if (GlobalLogger.useColors) {
            printf("\033[34m[Core \033[1;31mFATAL\033[0;34m]:\033[0m Critical error \033[33mC0000374\033[0m "
                   "detected "
                   "(heap corruption)\n");
        } else {
            printf("[Core FATAL]: Critical error c0000374 detected (heap corruption)\n");
        }
        printf("    Run the program with sanitization enabled (add -fsanitize=address to the compiler flags)\n");
        printf("    Alternatively, run it through valgrind.\n");
        exit(1);
    } else if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        printf("[Core FATAL]: Critical error 80000003 detected (SIGTRAP/breakpoint)\n");
        exit(1);
    }

    if (GlobalLogger.useColors) {
        printf("\033[34m[Core \033[1;31mFATAL\033[0;34m]:\033[0m Windows exception: \033[33m%08lX\033[0m\n",
               ExceptionInfo->ExceptionRecord->ExceptionCode);
        printf("    \033[34m[Core INFO]:\033[0m Generating debug info\n\n");
    } else {
        printf("[Core FATAL]: Windows exception: %08lX", ExceptionInfo->ExceptionRecord->ExceptionCode);
        printf("    [Core INFO]: Generating debug info\n\n");
    }

    dumpDebugStateInfo(currentState);

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif // _WIN32

#include <signal.h>

void onSignal(int signum) {
    if (signum == SIGSEGV) {
        if (GlobalLogger.useColors) {
            printf("\033[1;31mSegmentation fault\033[0m (core dumped)\n");
        } else {
            printf("Segmentation fault (core dumped)\n");
        }
        printf("    Load the program into gdb (gdb sakura then run test.sa)\n");
        exit(1);
    }
}

int main(int argc, const char **argv) {
    SakuraState *S;     // stored above
    int disasmMode = 0; // 1 << 8 = child assembly, RESERVED/DO NOT USE.
    const char *filename = 0;

    sakuraLoggerInit();
    signal(SIGSEGV, onSignal);

#ifdef _WIN32
    AddVectoredExceptionHandler(1, CriticalExceptionHandler);
#endif // _WIN32

    LOG_CALL();

    if (argc <= 1) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-h") == 0) {
                printf("Usage: %s <file>\n", argv[0]);
                return 0;
            } else if (strcmp(argv[i], "-v") == 0) {
                printf("Sakura version %s\n", SAKURA_VERSION);
                return 1;
            } else if (strcmp(argv[i], "-l") == 0) {
                disasmMode |= 1;
            } else if (strcmp(argv[i], "--bytecode") == 0) {
                disasmMode |= 1 << 1;
            } else if (strcmp(argv[i], "--kdump") == 0) {
                disasmMode |= 1 << 2;
            }
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("Usage: %s <file>\n", argv[0]);
        printf("Error: no file specified\n");
        return 1;
    }

    S = sakura_createState();
    currentState = S;

    sakuraL_loadfile(S, filename, disasmMode);

    sakura_destroyState(S);
    currentState = NULL;

    LOG_POP();

    sakuraLoggerClose();
    return 0;
}

void dumpDebugStateInfo(SakuraState *S) {
    const char *debuggerName = "[Core Debugger]:";
    if (GlobalLogger.useColors) {
        debuggerName = "\033[34m[Core Debugger]:\033[0m";
    }

    printf("%s DEBUG STATE INFO\n", debuggerName);
    printf(" %s Current state: ", debuggerName);
    switch (S->currentState) {
    case SAKURA_FLAG_LEXER:
        printf("lexer\n");
        break;
    case SAKURA_FLAG_PARSER:
        printf("parser\n");
        break;
    case SAKURA_FLAG_ASSEMBLING:
        printf("assembling\n");
        break;
    case SAKURA_FLAG_RUNTIME:
        printf("runtime\n");
        break;
    case SAKURA_FLAG_ENDED:
        printf("ended\n");
        break;
    case SAKURA_FLAG_DISASSEMBLING:
        printf("disassembling\n");
        break;
    default:
        printf("unknown\n");
        break;
    }
    printf(" %s Current internal offset: %lld\n", debuggerName, S->internalOffset);
    printf(" %s Stack capacity: %d\n", debuggerName, SAKURA_STACK_SIZE);
    printf(" %s Stack index: %d\n", debuggerName, S->stackIndex);
    printf("  ");
    sakuraDEBUG_dumpStack(S);
    printf(" %s Global table: %p\n", debuggerName, S->globals.pairs);
    printf(" %s Global table size: %lld\n", debuggerName, S->globals.capacity);
    printf(" %s Global table index: %lld\n", debuggerName, S->globals.size);
    printf(" %s Constant pool: %p\n", debuggerName, S->pool.constants);
    printf(" %s Constant pool size: %lld\n", debuggerName, S->pool.capacity);
    printf(" %s Constant pool index: %lld\n", debuggerName, S->pool.size);
    printf("  ");
    sakuraDEBUG_dumpConstantPool(&S->pool);
    printf(" %s Local table: %p\n", debuggerName, S->locals);
    printf(" %s Local table size: %lld\n", debuggerName, S->localsSize);
    printf(" %s Last error flag (%d): ", debuggerName, S->error);
    switch (S->error) {
    case SAKURA_EFLAG_NONE:
        printf("none\n");
        break;
    case SAKURA_EFLAG_SYNTAX:
        printf("syntax\n");
        break;
    case SAKURA_EFLAG_RUNTIME:
        printf("runtime\n");
        break;
    case SAKURA_EFLAG_FATAL:
        printf("fatal/internal\n");
        break;
    default:
        printf("unknown\n");
        break;
    }

    printf(" %s Last error message: %.*s\n", debuggerName, S->errorMessage.len, S->errorMessage.str);
    printf(" %s End debug info\n", debuggerName);

    LOG_DUMP();
}