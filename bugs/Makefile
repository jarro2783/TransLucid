CXXFLAGS=-std=gnu++0x -c -save-temps
LDFLAGS=-std=gnu++0x

all:lambda_inline_2 lambda_inline uninit_reference init_list

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

init_list.o: init_list.cpp
	g++ $< -o $@ $(CXXFLAGS)
