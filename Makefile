build:
	g++ -std=c++2a -Wall -Wextra -o server main.cpp
	./server

clean:
	rm server
