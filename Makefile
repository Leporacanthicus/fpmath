all: fpmath

CXX=clang++
CXXFLAGS=-std=c++11 -Wall -Werror -g

fpmath: fpmath.cpp

clean:
	rm -f fpmath
