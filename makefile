CC=gcc

TARGET=sakura

CFLAGS=-g -Og -DSAKURA_VERSION=\"$(APP_VERSION)\" # use -fsanitize=address for heap debugging
LDFLAGS=
.PHONY: all

VERSION_FILE := version.txt
ifeq ($(OS),Windows_NT)
	APP_VERSION := $(shell type $(VERSION_FILE))
else
	APP_VERSION := $(shell cat $(VERSION_FILE))
endif

all: $(TARGET)

$(TARGET): $(wildcard source/*.c) $(wildcard source/*.h)
	$(CC) $(CFLAGS) $(wildcard source/*.c) -o $@ $(LDFLAGS)
