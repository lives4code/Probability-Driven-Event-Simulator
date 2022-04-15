all: main

main: main.cpp
	g++ main.cpp -o simulator -std=c++17
clean:
	rm -f simulator