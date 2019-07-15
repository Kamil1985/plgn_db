all:
	./install_redis.sh
	./install_zmq.sh
	./install_hiredis.sh
	gcc src/server.c -o server -lzmq -lczmq -lhiredis -lpthread	
	gcc src/client.c -o clt -lzmq -lczmq
	gcc src/subscriver.c -o sub -lzmq -lczmq 

