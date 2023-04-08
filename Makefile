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

islands.o: islands.cpp

islands: islands.o map.o
	$(LD) $(LDFLAGS) -o $@ islands.o map.o

islands-flood.o: islands-flood.cpp

islands-flood: islands-flood.o map.o
	$(LD) $(LDFLAGS) -o $@ islands.o map.o

.PHONY: run
run: islands
	./islands TestFiles/map3_5islands.txt

.PHONY: check
check:
	valgrind --leak-check=full ./islands TestFiles/map3_5islands.txt

.PHONY: 1 2 3 4 5 6
1: islands
	./islands TestFiles/map1_6islands.txt
2: islands
	./islands TestFiles/map2_6islands.txt
3: islands
	./islands TestFiles/map3_5islands.txt
4: islands
	./islands TestFiles/map4_3islands.txt
5: islands
	./islands TestFiles/map5_4islands.txt
6: islands
	./islands TestFiles/map6_0islands.txt
X x: islands
	./islands TestFiles/islands.txt

# ----------------------------------------------------------------------

cube3d.o: cube3d.cpp

cube3d: cube3d.o
	$(LD) $(LDFLAGS) -o $@ cube3d.o $(LIBRARIES)

.PHONY: run_cube3d
run_cube3d: cube3d
	./cube3d

# ----------------------------------------------------------------------

islands3d.o: islands3d.cpp color.h map.h mapGenerator.h

islands3d: islands3d.o color.o map.o mapGenerator.o
	$(LD) $(LDFLAGS) -o $@ islands3d.o color.o map.o mapGenerator.o $(LIBRARIES)

.PHONY: run_islands3d
run_islands3d: islands3d
	./islands3d
