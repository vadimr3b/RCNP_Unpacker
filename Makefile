CXX       = /usr/bin/g++
CXXFLAGS  = -fPIC -Wall -Wshadow -Wextra -Wno-unused-result -Weffc++ -isystem$(shell root-config --incdir) -std=c++11 -march=native
LDFLAGS   = $(shell root-config --libs) -L./
SOFLAGS   = -fPIC -shared

.PHONY: all
all: 

unpacker: unpacker.o
	@echo "Buildung 'unpacker'..."
	@$(CXX) -o $@ $^ $(LDFLAGS) -lz -O3

.PHONY: clean
clean: 
	@rm -f *.o unpacker

%.o: %.cpp Makefile
	@$(CXX) -c -o $@ $< $(CXXFLAGS)
