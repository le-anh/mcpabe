CC = gcc

PUBLIC-KEY = pub_key
MASTER-KEY = master_key
PRIVATE-KEY = lhanh_priv_key
ATTRIBUTES = att1 att2 att3
PLAINTEXT = plaintext.txt
POLICY = 'att1 and att2 and att3'
CIPHERTEXT-EXTENSION = .cpabe
CIPHERTEXT = $(PLAINTEXT)$(CIPHERTEXT-EXTENSION)

SETUP-MAIN = ../cpabe/setup.c
KEYGEN-MAIN = ../cpabe/keygen.c
ENC-MAIN = ../cpabe/enc.c
DEC-MAIN = ../cpabe/dec.c

SETUP-TARGET = ./setup.out
KEYGEN-TARGET = ./keygen.out
ENC-TARGET = ./enc.out
DEC-TARGET = ./dec.out

SOURCES = ../cpabe/common.c ../libbswabe-0.9/core.c ../libbswabe-0.9/misc.c
SOURCES-EXTENSION = ../cpabe/policy_lang.c

CFLAGS = -O3 -w $(shell pkg-config --cflags glib-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0) -lgmp /usr/local/include/mcl/libmcl.a -lcrypto -lstdc++

.PHONY: all clean run

all: $(TARGET)

build-setup:
	@echo "Building setup..."
	@$(CC) $(SETUP-MAIN) $(SOURCES) -o $(SETUP-TARGET) $(CFLAGS) $(LDFLAGS)
	@echo "Build setup completed."

run-setup:
	@echo "Runing setup..."
	@$(SETUP-TARGET)
	@echo "Run setup completed."

rm-setup:
	@echo "Removing setup..."
	@rm -f $(SETUP-TARGET) $(PUBLIC-KEY) $(MASTER-KEY)
	@echo "Remove setup completed."

build-keygen:
	@echo "Building keygen..."
	@$(CC) $(KEYGEN-MAIN) $(SOURCES) $(SOURCES-EXTENSION) -o $(KEYGEN-TARGET) $(CFLAGS) $(LDFLAGS)
	@echo "Build keygen completed."

run-keygen:
	@echo "Runing keygen..."
	@$(KEYGEN-TARGET) -o $(PRIVATE-KEY) $(PUBLIC-KEY) $(MASTER-KEY) $(ATTRIBUTES)
	@echo "Run keygen completed."

rm-keygen:
	@echo "Removing keygen..."
	@rm -f $(KEYGEN-TARGET) $(PRIVATE-KEY)
	@echo "Remove keygen completed."

build-enc:
	@echo "Building encryption..."
	@$(CC) $(ENC-MAIN) $(SOURCES) $(SOURCES-EXTENSION) -o $(ENC-TARGET) $(CFLAGS) $(LDFLAGS)
	@echo "Build encryption completed."

run-enc:
	@echo "Runing encryption..."
	@$(ENC-TARGET) $(PUBLIC-KEY) $(PLAINTEXT) $(POLICY)
	@echo "Run encryption completed."

rm-enc:
	@echo "Removing encryption..."
	@rm -f $(ENC-TARGET) $(CIPHERTEXT)
	@echo "Remove encryption completed."

build-dec:
	@echo "Building decryption..."
	@$(CC) $(DEC-MAIN) $(SOURCES) -o $(DEC-TARGET) $(CFLAGS) $(LDFLAGS)
	@echo "Building decryption completed."

run-dec:
	@echo "Runing decryption..."
	@$(DEC-TARGET) $(PUBLIC-KEY) $(PRIVATE-KEY) $(CIPHERTEXT)
	@echo "Run decryption completed."

rm-dec:
	@echo "Removing decryption..."
	@rm -f $(DEC-TARGET)
	@echo "Remove decryption completed."

build: build-setup build-keygen build-enc build-dec

run: run-setup run-keygen run-enc run-dec

rm: rm-setup rm-keygen rm-enc rm-dec