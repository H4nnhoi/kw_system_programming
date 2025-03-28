CC = gcc
SRC = proxy_cache.c sha1Utils.c fileUtils.c dirUtils.c
TARGET = proxy

OPENSSL_DIR = /opt/homebrew/opt/openssl@3

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -I$(OPENSSL_DIR)/include -L$(OPENSSL_DIR)/lib -o $(TARGET) -lcrypto

clean:
	rm -f $(TARGET)
