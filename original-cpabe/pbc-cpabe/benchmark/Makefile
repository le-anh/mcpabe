CPABE_DIR ?= ../cpabe
LIBBSWABE_DIR ?= ../libbswabe

CC ?= cc
CFLAGS ?= -O3 -Wall
CPPFLAGS += -I$(CPABE_DIR) \
	-DPACKAGE_NAME=\"cpabe\" \
	-DPACKAGE_VERSION=\"0.11\"

GLIB_CFLAGS := $(shell pkg-config --cflags glib-2.0 2>/dev/null)
GLIB_LIBS := $(shell pkg-config --libs glib-2.0 2>/dev/null)
PBC_CFLAGS ?= -I/usr/include/pbc -I/usr/local/include/pbc
BSWABE_CFLAGS ?=
PBC_LIBS ?= -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lpbc
# BSWABE_LIBS ?= -L/usr/local/lib -lbswabe
BSWABE_LIBS ?= -L$(LIBBSWABE_DIR) -lbswabe
LDLIBS ?= $(GLIB_LIBS) $(BSWABE_LIBS) $(PBC_LIBS) -lgmp -lcrypto -w

TARGETS = setup keygen enc dec

.PHONY: all clean

all: $(TARGETS)

setup: $(CPABE_DIR)/setup.c $(CPABE_DIR)/common.c $(CPABE_DIR)/common.h $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GLIB_CFLAGS) $(PBC_CFLAGS) $(BSWABE_CFLAGS) -o $@ \
		$(CPABE_DIR)/setup.c $(CPABE_DIR)/common.c $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c \
		$(LDFLAGS) $(LDLIBS)

keygen: $(CPABE_DIR)/keygen.c $(CPABE_DIR)/common.c $(CPABE_DIR)/policy_lang.c \
		$(CPABE_DIR)/common.h $(CPABE_DIR)/policy_lang.h $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GLIB_CFLAGS) $(PBC_CFLAGS) $(BSWABE_CFLAGS) -o $@ \
		$(CPABE_DIR)/keygen.c $(CPABE_DIR)/common.c $(CPABE_DIR)/policy_lang.c $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c \
		$(LDFLAGS) $(LDLIBS)

enc: $(CPABE_DIR)/enc.c $(CPABE_DIR)/common.c $(CPABE_DIR)/policy_lang.c \
		$(CPABE_DIR)/common.h $(CPABE_DIR)/policy_lang.h $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GLIB_CFLAGS) $(PBC_CFLAGS) $(BSWABE_CFLAGS) -o $@ \
		$(CPABE_DIR)/enc.c $(CPABE_DIR)/common.c $(CPABE_DIR)/policy_lang.c $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c \
		$(LDFLAGS) $(LDLIBS)

dec: $(CPABE_DIR)/dec.c $(CPABE_DIR)/common.c $(CPABE_DIR)/common.h $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GLIB_CFLAGS) $(PBC_CFLAGS) $(BSWABE_CFLAGS) -o $@ \
		$(CPABE_DIR)/dec.c $(CPABE_DIR)/common.c $(LIBBSWABE_DIR)/core.c $(LIBBSWABE_DIR)/misc.c \
		$(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGETS)
