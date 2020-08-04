build:
	g++ -std=c++2a -Wall -Wextra -o server server.cpp
	./server

clean:
	rm server
