# Top-level Makefile for compiling the workspace utilities.

CC = gcc
CFLAGS = -g3 -Wall -Wextra -Wformat=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith -Wundef -m64 -fomit-frame-pointer -DNDEBUG -fno-stack-protector -O3 -fpic -DMCL_FP_BIT=384 -DMCL_FR_BIT=256 -DMCL_USE_LLVM=1 -DMCL_BINT_ASM=1 -DMCL_MSM=1 -DPACKAGE_NAME=\"cpabe\" -DPACKAGE_VERSION=\"0.9\"
INCLUDES = -Icpabe -Ilibbswabe-0.9 -Imcl/include
GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0 2>/dev/null || echo "-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include")
OPENSSL_CFLAGS = $(shell pkg-config --cflags openssl 2>/dev/null || echo "-I/usr/include")
CFLAGS += $(INCLUDES) $(GLIB_CFLAGS) $(OPENSSL_CFLAGS)

LDFLAGS = -m64
GLIB_LIBS = $(shell pkg-config --libs glib-2.0 2>/dev/null || echo "-lglib-2.0")
OPENSSL_LIBS = $(shell pkg-config --libs openssl 2>/dev/null || echo "-lssl -lcrypto")
#MCL_LIB ?= mcl/build/install/lib/libmcl.a
LDLIBS = -lrt -lgmp -lgmpxx -lstdc++ $(GLIB_LIBS) $(OPENSSL_LIBS) $(MCL_LIB)

LIB_DIR = /usr/local/include/mcl

SRC_SETUP = cpabe/setup.c cpabe/common.c libbswabe-0.9/core.c libbswabe-0.9/misc.c
OBJ_SETUP = $(SRC_SETUP:.c=.o)
TARGET_SETUP = cpabe-setup

.PHONY: all clean

all: pairing

pairing: pairing.c
	cc $(CFLAGS) $^ -o $@ $(LIB_DIR)/libmcl.a $(LDFLAGS) $(LDLIBS)

$(TARGET_SETUP): $(OBJ_SETUP)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f pairing
# 	rm -f pairing $(TARGET_SETUP) $(OBJ_SETUP)
