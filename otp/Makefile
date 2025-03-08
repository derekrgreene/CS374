CC = gcc
CFLAGS = --std=c99
SRC = enc_client.c enc_server.c dec_client.c dec_server.c keygen.c
EXEC = enc_client enc_server dec_client dec_server keygen
all: $(EXEC)

$(EXEC): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXEC) *.o