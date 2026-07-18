#include <asm-generic/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <malloc.h>

void *client_side(void *arg);

#define MAX_CLIENT 10

int client_list[MAX_CLIENT];
int client_count = 0;
pthread_mutex_t clientl_lock =  PTHREAD_MUTEX_INITIALIZER;

int main(){
  int gai_status = 0;
  int tcp_socket = 0;
  int bned_tcps = 0;
  int lis = 0;
  int client = 0;
  int reuseaddr_opt = 1;

  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints,0,sizeof(hints)); // trash cleaning
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP
  hints.ai_flags = AI_PASSIVE; //Fill in my local IP

  gai_status = getaddrinfo(NULL , "6767" , &hints, &res);
  if(gai_status != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_status));
    return -1;
  }
  printf("Address created successfully\n");

  tcp_socket = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
  if(tcp_socket < 0){
    perror("socket():");
    goto EXIT;
  }
  printf("Socket created sucsessfully...\n");
  
  if(setsockopt(tcp_socket, SOL_SOCKET , SO_REUSEADDR , &reuseaddr_opt , sizeof(reuseaddr_opt)) < 0){
    perror("setsockopt()");
    goto EXIT;
  }

  bned_tcps = bind(tcp_socket, res->ai_addr , res->ai_addrlen );
  if(bned_tcps < 0){
    perror("bind():");
    goto EXIT;
  }
  printf("Binded successfully...\n");

  lis = listen( tcp_socket , 10 );
  if(lis < 0){
    perror("listen():");
    goto  EXIT;
  }
  printf("listening successfully...\n");
  
  while(1){
    printf("listening...\n");

    client = accept(tcp_socket, NULL, NULL);
    if(client < 0){ 
      perror("accept():");
      continue;
    }
    
    pthread_mutex_lock(&clientl_lock);
    client_list[client_count] = client;
    client_count++;
    pthread_mutex_unlock(&clientl_lock);

    int *client_copy = malloc(sizeof(int));
    *client_copy = client;

    pthread_t tid;
    pthread_create(&tid, NULL , client_side, client_copy );
    pthread_detach(tid);

  }

EXIT:
  close(client);
  close(tcp_socket);
  freeaddrinfo(res);

  return 0;

}; 

void *client_side(void *arg){
  int clnt = *(int *)arg;
  free(arg);
  
  printf("Connection successfull!\n");

  char msg[512];
  int  msg_len = 0;

  while(1){
    
    int message_length = recv(clnt, msg + msg_len, sizeof(msg) -msg_len -1, 0);

    if (message_length <= 0){
      break;
    }
    
    msg_len += message_length;
    msg[msg_len]= '\0';
    char *newline = strchr(msg,'\n' );
    while((newline = strchr(msg, '\n')) != NULL){
      *newline = '\0';
      printf("complete message: %s\n",msg);
      pthread_mutex_lock(&clientl_lock);
      for(int i = 0; i < client_count; i++){
        if (client_list[i]!=clnt){
          send(client_list[i],msg,strlen(msg), 0);
          send(client_list[i],"\n",1,0);
        }
      }
      pthread_mutex_unlock(&clientl_lock);
    
      int whole_message = (newline - msg) + 1;
      int remaining = msg_len - whole_message;
      
      memmove(msg, newline + 1, remaining);
      msg_len = remaining;

    }



  }

  printf("client disconnected\n");

  pthread_mutex_lock(&clientl_lock);
  for(int i = 0; i < client_count; i++){
    if (client_list[i]==clnt){
      client_list[i] = client_list[client_count-1];
      client_count--;
      break;
    }
  }
  pthread_mutex_unlock(&clientl_lock);

  close(clnt);
  return NULL;

}

//me stupy
//me bingy
