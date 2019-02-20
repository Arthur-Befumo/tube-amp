CC = g++
CFLAGS = -g -Wall -DUSE_DOUBLE
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp -lcsound64
clean:
	$(RM) $(TARGE)