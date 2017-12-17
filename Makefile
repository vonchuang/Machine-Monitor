#!/bin/bash

CC= gcc -std=c99
CCFLAGS= -Wall
OPTIMIZE= -O3

SERVER= server
CLIENT= client
TARGET_SERVER= server.c
TARGET_CLIENT= client.c
GIT_HOOKS := .git/hooks/pre-commit
EXEC= target

.PHONY: all

obj-m += klsp.o

all: $(GIT_HOOKS) $(EXEC) klsp ulsp

klsp:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ulsp: ulsp.c
	gcc ulsp.c -o procprint

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

target:
	$(CC) -o $(SERVER) $(CCFLAGS) $(OPTIMIZE) $(TARGET_SERVER)
	$(CC) -o $(CLIENT) $(CCFLAGS) $(OPTIMIZE) $(TARGET_CLIENT)

debug:
	$(CC) -o $(SERVER) $(CCFLAGS) -g $(TARGET_SERVER)
	$(CC) -o $(CLIENT) $(CCFLAGS) -g $(TARGET_CLIENT)

prof:
	$(CC) -o $(SERVER) $(CCFLAGS) -g -pg $(TARGET_SERVER)
	$(CC) -o $(CLIENT) $(CCFLAGS) -g -pg $(TARGET_CLIENT)

clean:
	rm -rf $(SERVER)
	rm -rf $(CLIENT)
	rm -rf gmon.out
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm procprint
