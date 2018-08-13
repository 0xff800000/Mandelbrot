CC = g++
# CFLAGS = -W -Wall -std=c++11 -O3
CFLAGS = -W -Wall -std=c++11 -O3 -fopenmp
LDFLAGS = -lSDL2main -lSDL2 -lstdc++
EXEC = mandelbrot

all: $(EXEC)

$(EXEC): $(EXEC).cpp
	$(CC) $^ $(CFLAGS) -o $(EXEC) $(LDFLAGS)
