
#include <stdlib.h>
#include <string.h>
#include "zmq.h"
int main (int argc, char const *argv[])
{
	void* context = zmq_ctx_new();
	void* subscriber = zmq_socket(context, ZMQ_SUB);
	char _ip[32],ip_def[32]="tcp://localhost:4040";
	puts("Enter server IP-adress (default ""tcp://localhost:4040""): ");
	fgets(_ip,32,stdin);
	_ip[strlen(_ip)-1]='\0';
	if (strlen(_ip)==0) strcpy(_ip,ip_def);
	printf("Collecting stock information from the server.\n");
	int conn = zmq_connect(subscriber,_ip);
	conn = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, NULL, 0);
	int i;
	zmq_msg_t reply[2];
	for(;;) {
		zmq_msg_init(&reply[0]);
		zmq_msg_recv(&reply[0], subscriber, 0);
		int length = zmq_msg_size(&reply[0]);
		char* value = malloc(length);
		memcpy(value, zmq_msg_data(&reply[0]), length);
        	zmq_msg_close(&reply[0]);
		printf("%s\n", value);
		free(value);
	}
	zmq_close(subscriber);
	zmq_ctx_destroy(context);
	return 0;
}
