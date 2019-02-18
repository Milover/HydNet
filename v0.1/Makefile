# Makefile
# HydNet v0.1
#
# compile and link HydNet_v0.1.cpp

# source
SRC = ./src/HydNet_v0.1.cpp

# program
EXE = HydNet

# dependencies
NODE = Node.o Source.o Reservoir.o Valve.o
ELEMENT = Element.o Element_accessors.o Element_mutators.o\
	Element_utility.o Element_computation.o
LOOP = Loop.o Loop_accessors.o Loop_mutators.o\
	Loop_utility.o Loop_computation.o

BASIC = $(NODE) $(ELEMENT) $(LOOP) Fluid.o Material.o Settings.o
NETWORK = Network.o Network_input.o LoopDepot.o
COMPUTE = steadyState.o transientState.o

VAR = $(BASIC) $(NETWORK) $(COMPUTE)

# dependency pattern matching
OBJ = $(patsubst %.o, ./%.o, $(VAR))

# include search path
CPPFLAGS = -I ./src \
	   -I ./src/basic \
	   -I ./src/basic/node \
	   -I ./src/basic/element \
	   -I ./src/basic/loop \
	   -I ./src/network \
	   -I ./src/network/depot \
	   -I ./src/constant \
	   -I ./src/compute \
	   -I ./src/Eigen

# recipe search path
VPATH = ./src:./src/basic:./src/basic/node:./src/basic/element:\
	./src/basic/loop:./src/network:./src/network/depot:\
	./src/constant:./src/compute:./src/Eigen

# compiler
CC = g++ -g -Wall -std=c++14 -O3

# program
$(EXE): $(OBJ) $(SRC)
	$(CC) -o $@ $(CPPFLAGS) $(OBJ) $(SRC)
	rm -rf $(OBJ)

# dependencies
%.o: %.cpp
	$(CC) -c -o $@ $(CPPFLAGS) $<

# clean targets
clean:
	rm -rf $(OBJ) $(EXE)
