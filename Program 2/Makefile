# TCP Chat makefile

COMPILER = g++
FLAGS = -Wall -Werror

all:  cclient server

cclient: cclient.cpp exceptions.h message.h message.cpp
	$(COMPILER) $(FLAGS) cclient.cpp message.cpp -o cclient

server: server_main.cpp server.cpp server.h server_main.cpp exceptions.h message.h message.cpp
	$(COMPILER) $(FLAGS) server.cpp server_main.cpp message.cpp -o server

queit: server.cpp cclient.cpp exceptions.h message.h message.cpp
	$(COMPILER) cclient.cpp message.cpp -o cclient
	$(COMPILER) server.cpp message.cpp -o server

clean:
	rm cclient server
