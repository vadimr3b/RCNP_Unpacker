CXX       = /usr/bin/g++
CXXFLAGS  = -fPIC -Wall -Wextra -Wno-unused-result -Weffc++ -isystem$(shell root-config --incdir) -std=c++11 -march=native
LDFLAGS   = $(shell root-config --libs) -L./
SOFLAGS   = -fPIC -shared

.PHONY: all
all: unpacker

unpacker: main.o Unpacker.o
	@echo "Buildung 'unpacker'..."
	@$(CXX) -o $@ $^ $(LDFLAGS) -lz -O3

.PHONY: clean
clean: 
	@rm -f *.o unpacker

%.o: %.cxx Makefile
	@$(CXX) -c -o $@ $< $(CXXFLAGS)
