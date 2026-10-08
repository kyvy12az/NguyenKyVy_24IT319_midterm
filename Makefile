CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -D_NETBSD_SOURCE -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS =

TARGET := myls
OBJECTS := src/main.o src/options.o src/util.o src/entry.o src/format.o src/listing.o

.PHONY: all clean debug test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

src/main.o: src/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/main.c -o src/main.o

src/options.o: src/options.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/options.c -o src/options.o

src/util.o: src/util.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/util.c -o src/util.o

src/entry.o: src/entry.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/entry.c -o src/entry.o

src/format.o: src/format.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/format.c -o src/format.o

src/listing.o: src/listing.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/listing.c -o src/listing.o

debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -O0 -g3 -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" $(TARGET)

test: $(TARGET)
	sh tests/test.sh

clean:
	rm -f $(OBJECTS) $(TARGET)
