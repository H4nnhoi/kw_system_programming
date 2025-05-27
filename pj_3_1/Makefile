CC = gcc

# Target names
TARGET_SERVER = proxy_cache

# SRC files
SERVER_SRC = proxy_cache_server.c sub_process.c inputUtils.c dirUtils.c fileUtils.c hit_and_miss.c sha1Utils.c timeUtils.c serverUtils.c


# Default target
all : $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_SERVER):
	$(CC) -o $(TARGET_SERVER) $(SERVER_SRC) -lcrypto

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)
