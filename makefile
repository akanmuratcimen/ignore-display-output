CC := gcc
CFLAGS := -Wall -Wextra -std=gnu11 -pedantic -fdiagnostics-color=always
LIBS := -lm -lX11 -lXrandr
OUTPUT := ignore-display-output
SRC := main.c

build:
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -O3 -o $(OUTPUT)

build-debug:
	$(CC) $(SRC) $(CFLAGS) $(LIBS) -DDEBUG -o $(OUTPUT)

install: build
	sudo cp $(OUTPUT) /etc/lightdm/

run: ./$(OUTPUT)
	./$(OUTPUT) $(ARGS)

clean:
	rm $(OUTPUT)

