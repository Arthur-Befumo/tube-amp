CC = gcc
CFLAGS = -g -Wall
TARGET = ampsim

all: $(TARGET)

$(TARGET): $(TARGET).o tubeamp.o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o tubeamp.o -lsndfile -lm -ldl
$(TARGET).o: $(TARGET).c tubeamp.h
	$(CC) $(CFLAGS) -c $(TARGET).c tubeamp.h
tubeamp.o: tubeamp.c tubeamp.h
	$(CC) $(CFLAGS) -c tubeamp.c tubeamp.h
clean:
	$(RM) $(TARGET)