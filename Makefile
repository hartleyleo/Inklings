# Variables
CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++20 -g -O3
PROGRAMS = inklings
CPP = main.cpp ascii_art.cpp

# Targets and Dependencies
all: $(PROGRAMS) 

remake: clean inklings run

inklings:
	$(CXX) $(CXXFLAGS) $(CPP) -o $(PROGRAMS)

run:
	./$(PROGRAMS)

logs: clean
	$(CXX) $(CXXFLAGS) logs.cpp -o logs
	./logs

clean:
	rm -f $(PROGRAMS) *.o
	rm -f logs
	rm -rf inklings.dSYM
