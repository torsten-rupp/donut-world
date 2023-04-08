CC = gcc
CXX = g++
CFLAGS = -g -Wall
CXXFLAGS = -g -Wall `pkg-config --cflags gtk+-3.0`
LD = g++
LDFLAGS =
LIBRARIES = `pkg-config --libs gtk+-3.0` \
            -lepoxy \
            -lpthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c -o $@

.PHONY: default
default: all

.PHONY: all
all: \
  donut-world

.PHONY: clean
clean:
	rm -f color.o map.o mapGenerator.o
	rm -f donut-world.o donut-world

# ----------------------------------------------------------------------

color.o: color.cpp color.h

map.o: map.cpp map.h color.h

mapGenerator.o: mapGenerator.cpp mapGenerator.h color.h

donut-world.o: donut-world.cpp color.h map.h mapGenerator.h

donut-world: donut-world.o color.o map.o mapGenerator.o
	$(LD) $(LDFLAGS) -o $@ donut-world.o color.o map.o mapGenerator.o $(LIBRARIES)

# ----------------------------------------------------------------------
.PHONY: run
run: donut-world
	./donut-world

.PHONY: check
check: donut-world
	valgrind --leak-check=full ./donut-world
