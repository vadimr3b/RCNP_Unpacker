CXX       = /usr/bin/g++
CXXFLAGS  = -fPIC -Wall -Wextra -Wno-unused-result -Weffc++ -isystem$(shell root-config --incdir) -std=c++11 -march=native
LDFLAGS   = $(shell root-config --libs) -L./
SOFLAGS   = -fPIC -shared

.PHONY: all
all: unpacker

unpacker: main_unpacker.o Unpacker.o RCNP_Detector.o
	@echo "Buildung 'unpacker'..."
	@$(CXX) -o $@ $^ $(LDFLAGS) -lz -O3

calibrator: main_calibrator.o Calibrator.o
	@echo "Buildung 'calibrator'..."
	@$(CXX) -o $@ $^ $(LDFLAGS) -lz -O3
	
.PHONY: clean
clean: 
	@rm -f *.o unpacker

%.o: %.cxx Makefile
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.h %.cxx Makefile
	@$(CXX) -c -o $@ $< $(CXXFLAGS)
