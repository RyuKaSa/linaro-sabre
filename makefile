SRC = src/
BIN = bin/
CC = g++
CFLAGS = `pkg-config opencv --libs` -pg
OTHER = -fopenmp `pkg-config opencv --cflags`

all:
	$(CC) $(OTHER) $^ -o $@ $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $<

clean:
	rm -Force $(BIN)*.o
