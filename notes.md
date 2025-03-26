g++ -pg `pkg-config opencv --cflags` main.cpp -o main.o `pkg-config opencv --libs`
./main.o
gprof main.o