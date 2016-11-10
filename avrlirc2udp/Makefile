TARGET = avrlirc2udp
LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
OBJECTS = avrlirc2udp.o

default: $(TARGET)
all: default copy

avrlirc2udp.o: avrlirc2udp.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

copy:
	mkdir -p ../bin/linux
	cp $(TARGET) ../bin/linux
clean:
	-rm -f *.o
	-rm -f $(TARGET)
