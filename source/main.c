#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sakura.h"
#include "sap.h"

#ifndef SAKURA_VERSION
#define SAKURA_VERSION "UNKNOWN"
#endif

#ifdef _WIN32
#include <Windows.h>

LONG WINAPI CriticalExceptionHandler(EXCEPTION_POINTERS *ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_HEAP_CORRUPTION) {
        printf("FATAL: Critical error c0000374 detected (heap corruption)\n");
        printf("    Run the program with sanitization enabled (add -fsanitize=address to the compiler flags)\n");
        printf("    Alternatively, run it through valgrind.\n");
        exit(1);
    } else if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        printf("FATAL: Critical error 80000003 detected (SIGTRAP/breakpoint)\n");
        exit(1);
    }

    printf("Windows exception: %08X\n", ExceptionInfo->ExceptionRecord->ExceptionCode);

    return EXCEPTION_CONTINUE_SEARCH;
}
#endif // _WIN32

#include <signal.h>

void onSignal(int signum) {
    if (signum == SIGSEGV) {
        printf("Segmentation fault (core dumped)\n");
        printf("    Load the program into gdb (gdb sakura then run test.sa)\n");
        exit(1);
    }
}

int main(int argc, const char **argv) {
    signal(SIGSEGV, onSignal);

#ifdef _WIN32
    AddVectoredExceptionHandler(1, CriticalExceptionHandler);
#endif // _WIN32

    if (argc <= 1) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int disasmMode = 0;
    const char *filename = 0;
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

    SakuraState *S = sakura_createState();

    sakuraL_loadfile(S, filename, disasmMode);

    sakura_destroyState(S);

    return 0;
}