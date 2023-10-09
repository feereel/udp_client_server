CC=gcc
CFLAGS=-Wall -pedantic -Werror
BIN=client.exe server.exe

all: clean $(BIN)

%.exe:%.c
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -o $@ $<

clean:
	@echo "Cleaning"
	@rm -fv *.exe
