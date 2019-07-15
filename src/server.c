#include <czmq.h>
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "hiredis.h"
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

//Syntax of message command
#define CREATE_TABLE 	0
#define DELETE_TABLE 	1
#define UPDATE		2
#define DELETE		3
#define	GET		4
#define not_exist	15

char *message,*str,*pub_text;
zmq_msg_t msg,p_msg;
bool snd_msg,in_msg,pub_msg=0;
long int answer_i;
int msg_size=0;
char db[15][255];
short int db_num=-1;
unsigned char nn=0;

struct ___frames{
	signed char comm;
	char tab_name[255];
	char key[64];
	char value[1024];
	char ttl_sec[64];
	};//_frames
struct ___frames frames;

static void *start_server (void *context) {
	//  Socket to talk to dispatcher
	context = zmq_ctx_new ();
	void *server = zmq_socket (context, ZMQ_REP);
	//  Socket for subscribers	
	void *publisher = zmq_socket (context, ZMQ_PUB); 	
	int pc = zmq_bind (server, "tcp://*:5555");
	int rc = zmq_bind (publisher, "tcp://*:4040");
	while (1) {
		if (pub_msg) {
			int rc = zmq_msg_send (&p_msg, publisher, 0);
			pub_msg=false;
		}
		else if (snd_msg) {
			int rc = zmq_msg_send (&msg, server, 0);
			snd_msg=false;}
		else if (in_msg) {
			int rc = zmq_msg_init (&msg);
		  	assert (rc == 0);
	  		rc = zmq_msg_recv (&msg, server, 0);
			msg_size=zmq_msg_size(&msg);
			str=(char*)malloc(msg_size);
			memcpy(str,zmq_msg_data(&msg),msg_size);
			in_msg=false;
		}
    	}
    	zmq_close (server);
    	zmq_close (publisher);
    	return NULL;
}

void send_msg(char *text) {
	int rc = zmq_msg_init_size (&msg,strlen(text)+1);
	memcpy(zmq_msg_data (&msg),text,strlen(text)+1);
	snd_msg=true;
}

void publishing(byte _type) {
	pub_text=(char*)malloc(strlen(frames.tab_name)+255);
	strcpy(pub_text,frames.tab_name);
	switch (_type) {
		case CREATE_TABLE: 
			strcat(pub_text," created tab"); break;
		case DELETE_TABLE:
			strcat(pub_text," deleted tab"); break;
	 	case UPDATE:
			strcat(pub_text," updated "); 
			strcat(pub_text,frames.key); break;
		case DELETE:
			strcat(pub_text," deleted ");
			strcat(pub_text,frames.key); break;
	}//switch
	int rc = zmq_msg_init_size (&p_msg,strlen(pub_text)+1);
	memcpy(zmq_msg_data (&p_msg),pub_text,strlen(pub_text)+1);
	pub_msg=true;
}

void *get_answer() {
	in_msg=true;
	while (in_msg) {}
}

byte get_db_index(char name[]) {
	byte i;	
	for (i=0; i<15; i++) {	
		if (strcmp(db[i],name)==0) {break;}}
	return i; 	
}

byte add_db(char name[]) {
	byte i;
	for (i=0; i<15; i++) if (strlen(db[i])==0) {break;}
	if (get_db_index(name)==not_exist) {strcpy(db[i],name); return i;}
}

int del_db(char name[]) {
	byte i;
	if (get_db_index(name)==not_exist) {return -1;}	
	for (i=0; i<15; i++) if (strcmp(db[i],name)==0) {break;}
	strcpy(db[i],"");
	return 0;	
}

char db_count() {
	byte i,n=0;
	for (i=0; i<15; i++) if (strlen(db[i])>0) {n++;}
	return n;	
}

bool db_exist_error() {
	if (get_db_index(frames.tab_name)==not_exist) {	//если такой таблицы нет, сообщить об ошибке
		send_msg("error"); get_answer();
		send_msg("not exist!");
		nn=0;
		return 1;			
	}			
	else return 0;
}

int main (void)
{	
	void *context = zmq_ctx_new ();
	pthread_t server_thread, publisher_thread;
	pthread_create (&server_thread, NULL, start_server, context);

   	unsigned int j, isunix = 0;
    	redisContext *c;
    	redisReply *reply;
    	const char *hostname = "127.0.0.1";
    	int port = 6379;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	{c = redisConnectWithTimeout(hostname, port, timeout);}
	
   	if (c == NULL || c->err) {
    		if (c) {printf("Connection error: %s\n", c->errstr);
  	 	        redisFree(c);
			}//if
   	     	else {printf("Connection error: can't allocate redis context\n");}
     	exit(1);
    	}//if

	str=(char*)malloc(255+64+1024+64);
	frames.comm=-1;
	nn=0;

	do {
		nn++;
		get_answer();
		printf("clients command: %s\n",str);	
		if (nn==1) {

		frames.comm=str[0]-48;	
		if (frames.comm==7) {
			nn=0;
			send_msg("test");} //else if		
		else if ((frames.comm<0)||(frames.comm>5)) {
			nn=0; 
			send_msg("error"); sleep(1); get_answer();
			send_msg("wrong command"); 
			}//if frames.comm....		
		else {send_msg("ok");}

		}//if nn==1
		else if (nn==2) { 
			strcpy(frames.tab_name,str);
			//ВЫБОР БД по имени
			if (frames.comm==CREATE_TABLE) {
				printf("%d\n",frames.comm);
				if (get_db_index(frames.tab_name)!=not_exist) {	//если название таблицы уже такое было, сообщить об ошибке
					send_msg("error"); get_answer();
					send_msg("already exist!");
					nn=0;			
				}//if get_db_index
				else 	{db_num=add_db(frames.tab_name);			
					nn=0;
					char s[strlen((char*)&str)+10];
					strcpy(s,"select ");
					char p[3];
					sprintf(p,"%d",db_num);
					strcat(s,p);
					printf("%s\n",s);
					reply = redisCommand(c,s); 
					char  *pch = strtok (reply->str," ,.-");
					if (strcmp(pch,"ERR")==0) {send_msg("error");} 
					else { publishing(CREATE_TABLE); sleep(1); send_msg(reply->str); }
				}//else
			}//if frames=CREATE_TABLE

			else if (frames.comm==DELETE_TABLE) {
   				printf("delete table");
				if (!db_exist_error()) {  char p[3];
					sprintf(p,"%d",get_db_index(frames.tab_name));
					char *s=(char*)malloc(10);
					strcpy(s,"select ");
					strcat(s,p);
					reply = redisCommand(c,s);
					del_db(frames.tab_name);
					reply = redisCommand(c,"flushdb");
					publishing(DELETE_TABLE);
					sleep(1);
					send_msg("ok"); 
					reply = redisCommand(c,"select 0"); 
					nn=0;
				}//else
			}
			else send_msg("ok");
		}//if nn==2
		
		else if (nn==3) {
			strcpy(frames.key,str);
			if (frames.comm==DELETE) {
				if (!db_exist_error()) {  char p[3];
					sprintf(p,"%d",get_db_index(frames.tab_name));
					char *s=(char*)malloc(10);
					strcpy(s,"select ");
					strcat(s,p);
					reply = redisCommand(c,s);
					puts(reply->str);
					s=(char*)malloc(128);
					strcpy(s,"del ");
					strcat(s,frames.key);
					reply = redisCommand(c,s);
					answer_i=(int)reply->integer;
					if (answer_i!=1) {send_msg("error"); get_answer(); send_msg("not exist");}
					else { publishing(DELETE); sleep(1); send_msg("ok");}
				}//if db
				nn=0;
			}//if DELETE

			else if (frames.comm==GET) {
				nn=0;
				if (!db_exist_error()) {  char p[3];
					sprintf(p,"%d",get_db_index(frames.tab_name));
					char *s=(char*)malloc(10);
					strcpy(s,"select ");
					strcat(s,p);
					reply = redisCommand(c,s);
					s=(char*)malloc(128);
					strcpy(s,"get ");
					strcat(s,frames.key);
					reply = redisCommand(c,s); 
					answer_i=(int)reply->integer;
					if (reply->type==REDIS_REPLY_NIL) {send_msg("error"); get_answer(); send_msg("not exist");}
					else { send_msg("ok"); get_answer();  send_msg(reply->str);}
				}//if !db_exist...
				nn=0;
			}//if GET
		else send_msg("ok");
		}//if nn==3
	
		else if (nn==4) {
			strcpy(frames.value,str);
			send_msg("ok");	
		}//if nn==4

		else if (nn==5) {						//UPDATE
			strcpy(frames.ttl_sec,str);
			if (!db_exist_error()) {  char p[3];
				sprintf(p,"%d",get_db_index(frames.tab_name));
				char *s=(char*)malloc(10);
				strcpy(s,"select ");
				strcat(s,p);
				puts(s);
				reply = redisCommand(c,s);
				s=(char*)malloc(1500);
				strcpy(s,"set ");
				strcat(s,frames.key); strcat(s," ");
				strcat(s,frames.value); strcat(s," ");
				if (strcmp(frames.ttl_sec,"0")!=0) {strcat(s,"EX "); strcat(s,frames.ttl_sec);}
				puts(s);
				reply = redisCommand(c,s); 
				char answer[64];
				strcpy(answer,reply->str);
				char  *pch = strtok (reply->str," ,.-");
				if (strcmp(pch,"ERR")==0) 
					{send_msg("error"); get_answer(); send_msg(answer);}
				else { publishing(UPDATE); sleep(1); send_msg("ok");}
			}//if !db_exist...
			nn=0;
		}//if nn=5
   	} while(1);//do
	return(0);
	zmq_ctx_destroy (context);	
	redisFree(c);
}//void main
