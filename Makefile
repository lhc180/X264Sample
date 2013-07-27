CC=gcc
CFLAGS=-Wall -O0 -g -I/Users/apple/Dev/x264
LDFLAGS=-L/Users/apple/Dev/x264 -lx264

TARGET=avcenc
SOURCES=avcenc.c
OBJECTS=$(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(TARGET) out.264