################################################
# Copyright (c) 2020 Intel Corporation.
# All rights reserved.
#
# SPDX-License-Identidfier: Apache-2.0
#
################################################

PROG_NAME := secure_erase_daemon
CC := clang

CFLAGS += -I.
CFLAGS += -g0 -O2 -std=gnu99
CFLAGS += -Wall -Wextra -Wbad-function-cast \
	  -Winit-self -Wswitch-default \
	  -Wredundant-decls -Wnested-externs -Winline -Wstack-protector \
	  -Woverlength-strings -Waggregate-return \
	  -Wmissing-field-initializers -Wpointer-arith -Wcast-qual \
	  -Wcast-align -Wwrite-strings \
	  -fno-delete-null-pointer-checks -fstack-protector-strong \
      -Wa,--noexecstack -fwrapv \
	  -D_FORTIFY_SOURCE=2 -fPIE -fPIC \
	  -Wformat -Wformat-security -Werror=format-security

LDFLAGS += -Wl,-z,relro,-z,now -pie

SRCS := secure_erase_daemon.c
SRCS += secure_wipe_partition.c

OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(PROG_NAME)

.PHONY: clean

$(PROG_NAME): $(OBJS)
	@echo "OBJS = '$(OBJS)'"
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o
	rm $(PROG_NAME)
