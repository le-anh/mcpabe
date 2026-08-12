CC = gcc
TARGET = ./setup.out
SOURCES = ./cpabe/setup.c ./cpabe/common.c ./libbswabe-0.9/core.c ./libbswabe-0.9/misc.c
CFLAGS = -O3 -w $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0) -lgmp /usr/local/include/mcl/libmcl.a -lcrypto -lstdc++

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

run: $(TARGET)
	./setup.out

clean:
	rm -f $(TARGET)

rm_key:
	rm -f ./pub_key ./master_key
