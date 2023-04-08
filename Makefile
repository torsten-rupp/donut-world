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
  islands3d

.PHONY: clean
clean:
	rm -f color.o map.o mapGenerator.o
	rm -f islands3d.o islands3d

# ----------------------------------------------------------------------

color.o: color.cpp color.h

mapGenerator.o: mapGenerator.cpp mapGenerator.h color.h

map.o: map.cpp map.h color.h

islands3d.o: islands3d.cpp color.h map.h mapGenerator.h

islands3d: islands3d.o color.o map.o mapGenerator.o
	$(LD) $(LDFLAGS) -o $@ islands3d.o color.o map.o mapGenerator.o $(LIBRARIES)

# ----------------------------------------------------------------------
.PHONY: run
run: islands3d
	./islands3d

.PHONY: check
check:
	valgrind --leak-check=full ./islands3d
