CXX=g++
CXXFLAGS=-std=c++14 -fno-rtti -ftemplate-depth=1024
WFLAGS=-Wall -Wextra -pedantic-errors

BIN=TemplMandel
SOURCES=$(BIN).cxx

all: $(BIN)
$(BIN):
	$(CXX) $(CXXFLAGS) $(WFLAGS) $(SOURCES) -o $(BIN)

.PHONY: clean
clean:
	$(RM) $(BIN)
