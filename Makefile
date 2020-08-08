build:
	g++ -std=c++2a -Wall -Wextra -o server server.cpp
	g++ -std=c++2a -Wall -Wextra -o client client.cpp

client:
	g++ -std=c++2a -Wall -Wextra -o client client.cpp

clean:
	rm server client
