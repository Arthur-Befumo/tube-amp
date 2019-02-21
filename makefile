CC = gcc
CFLAGS = -g -Wall
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lsndfile
clean:
	$(RM) $(TARGE)