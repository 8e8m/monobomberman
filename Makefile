#!/usr/bin/make -f

SOURCE.dir      := source
OBJECT.dir      := object
INCLUDE.dir     := include
LIBRARY.dir     := library
LIBRARY         := $(addprefix ${LIBRARY.dir}/, libraylib.amd64.a)
HEADER          := $(wildcard ${SOURCE.dir}/*.h) $(wildcard ${INCLUDE.dir}/*.h) include/raygui.h include/raylib.h
HEADER.orig     := $(HEADER:${SOURCE.dir}/%=%)
HEADER.orig     := $(HEADER.orig:${INCLUDE.dir}/%=%)
HEADER.pch      := $(filter-out ${OBJECT.dir}/style_dark.h,$(addprefix ${OBJECT.dir}/, ${HEADER.orig:.c=}))
SOURCE.orig     := $(wildcard ${SOURCE.dir}/*.c)
SOURCE.orig     := $(SOURCE.orig:${SOURCE.dir}/%=%)
SOURCE          := $(addprefix ${SOURCE.dir}/, ${SOURCE.orig})
OBJECT          := $(addprefix ${OBJECT.dir}/, ${SOURCE.orig:.c=.o})
DEPEND          := $(wildcard ${OBJECT.dir}/*.d)

TARGET := $(shell basename $$PWD)

CFLAGS   := -std=c23 -pthread -MMD -MP
CPPFLAGS := -Iinclude -D_GNU_SOURCE -I ${OBJECT.dir}
LDFLAGS  := -lm -lbsd

ifneq ($(shell which mold 2> /dev/null),)
        LDFLAGS += -fuse-ld=mold
        LD := mold
endif

ifneq ($(shell which clang 2> /dev/null),)
	CC := clang
else
        ifneq ($(shell which gcc 2> /dev/null),)
                CC := gcc
	else
	$(error GCC & Clang not available.)
        endif
endif

ifeq (${CC},gcc)
        HEADER.pch := $(addsuffix .gch,${HEADER.pch})
else
        HEADER.pch := $(addsuffix .pch,${HEADER.pch})
endif

DEBUG      ?= 0
SANITIZE   ?= 0
VECTORIZED ?= 0

ifeq (${DEBUG},1)
        CFLAGS += -O2 -ggdb -pg -fno-inline -Wall -Wextra -Wpedantic -Wshadow -Wundef
else
        CFLAGS   += -O0 -flto=auto -ftree-vectorize -march=x86-64 -mtune=generic
        CPPFLAGS += -DNDEBUG -D_FORTIFY_SOURCE=2
endif

ifeq (${SANITIZE},1)
        CFLAGS += -fsanitize=address,undefined
endif

ifeq (${VECTORIZED},1)
        ifeq (${CC},clang)
                CFLAGS += -Rpass=loop-vectorize
        else
                CFLAGS += -fopt-info-vec-optimized
        endif
endif

ifeq (${VECTORIZED_ALL},1)
        ifeq (${CC},clang)
          CFLAGS += -Rpass=loop-vectorize -Rpass-missed=loop-vectorize
        else
          CFLAGS += -fopt-info-vec-all
        endif
endif

vpath %.c ${SOURCE.dir}
vpath %.h ${SOURCE.dir} ${INCLUDE.dir}

${OBJECT.dir}/%.o: %.c
	@echo "CC	$<"
	@${COMPILE.c} -o $@ $<

# The tree builds incorrectly if these two are merged
# at the target level at a bare build.
${OBJECT.dir}/%.h.pch: %.h
	@echo "PCH	$<"
	@${COMPILE.c} ${PCHFLAGS} -x c-header -o $@ $<

 ${OBJECT.dir}/%.h.gch: %.h
	@echo "GCH	$<"
	@${COMPILE.c} ${PCHFLAGS} -x c-header -o $@ $<

.PHONY: all clean
all: include/raylib.h include/raygui.h .WAIT ${HEADER.pch} ${TARGET}

${HEADER.pch}: ${LIBRARY.dir}/libraylib.amd64.a include/raygui.h include/style_dark.h

clean:
	rm -f ${OBJECT.dir}/*.o ${OBJECT.dir}/*.pch ${OBJECT.dir}/*.gch ${OBJECT.dir}/*.d ${LIBRARY.dir}/*.a ${INCLUDE.dir}/*.h ${TARGET}
	rm -f /tmp/raygui.tgz /tmp/raylib.tgz

${TARGET}: ${LIBRARY.dir}/libraylib.amd64.a ${OBJECT}
	@echo "LD	$@"
	@${LINK.c} -o "$@" $+ ${LDFLAGS}

include/raylib.h ${LIBRARY.dir}/libraylib.amd64.a &:
	@echo Updating Raylib
	@[ ! -e /tmp/raylib.tgz ] && wget -qO /tmp/raylib.tgz "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz" || true
	@tar xvC /tmp/ -f /tmp/raylib.tgz raylib-5.5_linux_amd64/lib/libraylib.a raylib-5.5_linux_amd64/LICENSE raylib-5.5_linux_amd64/include > /dev/null
	@mv -f /tmp/raylib-5.5_linux_amd64/lib/libraylib.a library/libraylib.amd64.a
	@mv -f /tmp/raylib-5.5_linux_amd64/include/* include/
	@mv -f /tmp/raylib-5.5_linux_amd64/LICENSE library/libraylib.LICENSE.a

include/raygui.h include/style_dark.h &:
	@echo Updating RayGUI
	@[ ! -e /tmp/raygui.tgz ] && wget -qO /tmp/raygui.tgz https://github.com/raysan5/raygui/archive/refs/tags/4.0.tar.gz || true
	@tar xvC /tmp/ -f /tmp/raygui.tgz raygui-4.0/src/raygui.h raygui-4.0/styles/dark/style_dark.h > /dev/null
	@mv -f /tmp/raygui-4.0/src/raygui.h include/
	@mv -f /tmp/raygui-4.0/styles/dark/style_dark.h include/

-include ${DEPEND}
