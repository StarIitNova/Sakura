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
            printf("\x1b[34m[Core \x1b[1;31mFATAL\x1b[0;34m]:\x1b[0m Critical error \x1b[33mC0000374\x1b[0m "
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
        printf("\x1b[34m[Core \x1b[1;31mFATAL\x1b[0;34m]:\x1b[0m Windows exception: \x1b[33m%08lX\x1b[0m\n",
               ExceptionInfo->ExceptionRecord->ExceptionCode);
        printf("    \x1b[34m[Core INFO]:\x1b[0m Generating debug info\n\n");
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
            printf("\x1b[1;31mSegmentation fault\x1b[0m (core dumped)\n");
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
        debuggerName = "\x1b[34m[Core Debugger]:\x1b[0m";
    }

    sakura_printf("%s DEBUG STATE INFO\n", debuggerName);
    sakura_printf(" %s Current state: ", debuggerName);
    switch (S->currentState) {
    case SAKURA_FLAG_LEXER:
        sakura_printf("\x1b[33mlexer\x1b[0m\n");
        break;
    case SAKURA_FLAG_PARSER:
        sakura_printf("\x1b[33mparser\x1b[0m\n");
        break;
    case SAKURA_FLAG_ASSEMBLING:
        sakura_printf("\x1b[33massembling\x1b[0m\n");
        break;
    case SAKURA_FLAG_RUNTIME:
        sakura_printf("\x1b[33mruntime\x1b[0m\n");
        break;
    case SAKURA_FLAG_ENDED:
        sakura_printf("\x1b[33mended\x1b[0m\n");
        break;
    case SAKURA_FLAG_DISASSEMBLING:
        sakura_printf("\x1b[33mdisassembling\x1b[0m\n");
        break;
    default:
        sakura_printf("\x1b[33munknown\x1b[0m\n");
        break;
    }
    sakura_printf(" %s Current internal offset: \x1b[33m%lld\x1b[0m\n", debuggerName, S->internalOffset);
    sakura_printf(" %s Stack capacity: \x1b[33m%d\x1b[0m\n", debuggerName, SAKURA_STACK_SIZE);
    sakura_printf(" %s Stack index: \x1b[33m%d\x1b[0m\n", debuggerName, S->stackIndex);
    sakura_printf("  ");
    sakuraDEBUG_dumpStack(S);
    sakura_printf(" %s Global table: \x1b[33m%p\x1b[0m\n", debuggerName, S->globals.pairs);
    sakura_printf(" %s Global table size: \x1b[33m%lld\x1b[0m\n", debuggerName, S->globals.capacity);
    sakura_printf(" %s Global table index: \x1b[33m%lld\x1b[0m\n", debuggerName, S->globals.size);
    sakura_printf(" %s Constant pool: \x1b[33m%p\x1b[0m\n", debuggerName, S->pool.constants);
    sakura_printf(" %s Constant pool size: \x1b[33m%lld\x1b[0m\n", debuggerName, S->pool.capacity);
    sakura_printf(" %s Constant pool index: \x1b[33m%lld\x1b[0m\n", debuggerName, S->pool.size);
    sakura_printf("  ");
    sakuraDEBUG_dumpConstantPool(&S->pool);
    sakura_printf(" %s Local table: \x1b[33m%p\x1b[0m\n", debuggerName, S->locals);
    sakura_printf(" %s Local table size: \x1b[33m%lld\x1b[0m\n", debuggerName, S->localsSize);
    sakura_printf(" %s Last error flag (%d): ", debuggerName, S->error);
    switch (S->error) {
    case SAKURA_EFLAG_NONE:
        sakura_printf("\x1b[33mnone\x1b[0m\n");
        break;
    case SAKURA_EFLAG_SYNTAX:
        sakura_printf("\x1b[33msyntax\x1b[0m\n");
        break;
    case SAKURA_EFLAG_RUNTIME:
        sakura_printf("\x1b[33mruntime\x1b[0m\n");
        break;
    case SAKURA_EFLAG_FATAL:
        sakura_printf("\x1b[33mfatal/internal\x1b[0m\n");
        break;
    default:
        sakura_printf("\x1b[33munknown\x1b[0m\n");
        break;
    }

    sakura_printf(" %s Last error message: %.*s\n", debuggerName, S->errorMessage.len, S->errorMessage.str);
    sakura_printf(" %s End debug info\n", debuggerName);

    LOG_DUMP();
}