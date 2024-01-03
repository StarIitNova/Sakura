CC=gcc

TARGET=sakura

CWARNSCPP=-Wfatal-errors -Wextra -Wshadow -Wundef -Wwrite-strings -Wredundant-decls\
		  -Wdisabled-optimization -Wdouble-promotion -Wmissing-declarations
CWARNSGCC=-Wlogical-op -Wno-aggressive-loop-optimizations
CWARNSC=-Wdeclaration-after-statement -Wmissing-prototypes -Wnested-externs -Wstrict-prototypes -Wc++-compat -Wold-style-definition

CWARNS=$(CWARNSCPP) $(CWARNSGCC) $(CWARNSC)

# use -fsanitize=address for heap debugging
DEBUGCFLAGS=-g -Og -DSAKURA_DEBUG
RELEASECFLAGS=-O3 -DSAKURA_RELEASE

MYCFLAGS=$(CWARNS) $(DEBUGCFLAGS) -std=c99 -DSAKURA_VERSION=\"$(APP_VERSION)\"

CFLAGS=-Wall $(MYCFLAGS) -fno-stack-protector -fno-common -march=native
LDFLAGS=
.PHONY: all

VERSION_FILE := version.txt
ifeq ($(OS),Windows_NT)
	APP_VERSION := $(shell type $(VERSION_FILE))
else
	APP_VERSION := $(shell cat $(VERSION_FILE))

	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		LDFLAGS += -lm -ldl
	endif
endif

all: $(TARGET)

$(TARGET): $(wildcard source/*.c) $(wildcard source/*.h)
	$(CC) $(CFLAGS) $(wildcard source/*.c) -o $@ $(LDFLAGS)
