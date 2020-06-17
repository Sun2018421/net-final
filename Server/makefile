all : server.cpp ../Socket/Mysocket.cpp ftpserver.cpp
	g++ -g ../Socket/Mysocket.cpp ftpserver.cpp server.cpp  -o server -lpthread
clean:
	rm server