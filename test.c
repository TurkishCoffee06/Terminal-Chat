#include <asm-generic/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <netdb.h>

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

  lis = listen( tcp_socket , 20 );
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



    if(client > 0){
      printf("connection successfull!\n");
      goto EXIT;
    }

  }

EXIT:
  close(client);
  close(tcp_socket);
  freeaddrinfo(res);

  return 0;

}; 

//me stupy
//me bingy
