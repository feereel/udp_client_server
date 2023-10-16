CC=gcc
CFLAGS=-Wall -pedantic -Werror
BIN=client.exe server.exe

build: clean $(BIN)

%.exe:%.c
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -o $@ $<

clean:
	@echo "Cleaning"
	@rm -fv *.exe *.bin
