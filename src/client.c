//Код тестового клиента для сервера

#include "czmq.h"
#include "stdio.h"
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

char *str;
zmq_msg_t msg;
long msg_size;

char *add_zero(char *txt,int size) {
	char *dest;
	dest=calloc(sizeof(char), size + 1);
	strncpy(dest,txt,size);
	return dest;
}

void send_message(char *text, void *socket) {
	int rc = zmq_msg_init_size (&msg,strlen(text)+1);
	memcpy(zmq_msg_data (&msg),text,strlen(text)+1);
	rc = zmq_msg_send (&msg, socket, 0);
}

void *get_answer(void *socket) {
	int rc = zmq_msg_init (&msg);
  	assert (rc == 0);
  	msg_size = zmq_msg_recv (&msg, socket, 0);
	str=(char*)malloc(msg_size);
	strcpy(str,zmq_msg_data(&msg));
	if (str[strlen(str)-1]=='\n') str[strlen(str)-1]='\0';

}

int main (void)
{	
	int rc;
	void *context = zmq_ctx_new ();
	void *client = zmq_socket (context, ZMQ_REQ);
	char _ip[32],ip_def[32]="tcp://localhost:5555";
	puts("Enter server IP-adress (default ""tcp://localhost:5555""): ");
	fgets(_ip,32,stdin);
	_ip[strlen(_ip)-1]='\0';
	if (strlen(_ip)==0) strcpy(_ip,ip_def);
	zmq_connect (client, _ip);

	do {
		char inp[128];
		printf("Enter command: ");
		fgets(inp,128,stdin);
		inp[strlen(inp)-1]='\0';
		send_message(inp,client);
		get_answer(client);
		printf("SERVER: %s\n",str);
	} while (1);

	return 0;
	zmq_close (client);
	zmq_ctx_destroy (context);
	zmq_msg_close(&msg);
}
