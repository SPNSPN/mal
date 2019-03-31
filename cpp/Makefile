GCC := g++ -std=c++14 -Wall -O2
CFLAGS := -Iinclude -DINITIAL_POOL_SIZE=128 -DRECURSIVE_PRINT=true -DTYPECHECK=true -DAREACHECK=true -DARGSCHECK=true -DGC_INTERVAL=100

SRC := src
INC := include
BLD := build

SRCS := $(filter-out $(SRC)/vm.cpp $(SRC)/repl.cpp, $(wildcard $(SRC)/*.cpp))
OBJS := $(patsubst $(SRC)/%.cpp,$(BLD)/%.o,$(SRCS))

DERXI := $(BLD)/derxivm
JETCE := $(BLD)/jetcevm
REPL := $(BLD)/repl

.PHONY: all
all: $(DERXI) $(JETCE) $(REPL)

.PHONY: derxi jetce repl
derxi: $(DERXI)
	:
jetce: $(JETCE)
	:
repl: $(REPL)
	rlwrap $(BLD)/repl

$(DERXI): $(OBJS) $(SRC)/vm.cpp
	$(GCC) $(CFLAGS) -DDERXIVM -c -o $(DERXI).o $(SRC)/vm.cpp
	$(GCC) -o $@ $(OBJS) $(DERXI).o

$(JETCE): $(OBJS) $(SRC)/vm.cpp
	$(GCC) $(CFLAGS) -DJETCEVM -c -o $(JETCE).o $(SRC)/vm.cpp
	$(GCC) -o $@ $(OBJS) $(JETCE).o 

$(REPL): $(OBJS) $(SRC)/repl.cpp
	$(GCC) $(CFLAGS) -c -o $(REPL).o $(SRC)/repl.cpp
	$(GCC) -o $@ $(OBJS) $(REPL).o

$(BLD)/%.o: $(SRC)/%.cpp
	$(GCC) $(CFLAGS) -c -o $@ $<

.PHONY: test
test: $(OBJS)
	$(GCC) $(CFLAGS) -o $(BLD)/test test/* $(OBJS)
	$(BLD)/test

.PHONY: clean
clean:
	rm $(BLD)/*

