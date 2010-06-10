CXXFLAGS=-std=gnu++0x -c -save-temps 
LDFLAGS=-std=gnu++0x
BIN=lambda_inline_2 lambda_inline uninit_reference init_list \
static_unused

all:$(BIN)

lambda_inline: lambda_inline_a.o lambda_inline_b.o
	g++ -o $@ $^ $(LDFLAGS)

lambda_inline_a.o: lambda_inline_a.cpp lambda_inline.hpp
	g++ $< -o $@ $(CXXFLAGS)

lambda_inline_b.o: lambda_inline_b.cpp lambda_inline.hpp
	g++ $< -o $@ $(CXXFLAGS)

lambda_inline_2: lambda_inline_c.o lambda_inline_d.o
	g++ -o $@ $^ $(LDFLAGS)

lambda_inline_c.o: lambda_inline_c.cpp lambda_inline_2.hpp
	g++ $< -o $@ $(CXXFLAGS)

lambda_inline_d.o: lambda_inline_d.cpp lambda_inline_2.hpp
	g++ $< -o $@ $(CXXFLAGS)

uninit_reference: uninit_reference.o
	g++ -o $@ $^ $(LDFLAGS)

uninit_reference.o: uninit_reference.cpp
	g++ $< -o $@ $(CXXFLAGS)

init_list: init_list.o
	g++ -o $@ $^ $(LDFLAGS)

init_list.o: init_list.cpp
	g++ $< -o $@ $(CXXFLAGS)

static_unused.o: static_unused.cpp
	g++ $< -o $@ $(CXXFLAGS) -Wall -O2

static_unused: static_unused.o
	g++ -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN) *.o

.PHONY: clean
