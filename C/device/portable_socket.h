#pragma once

#include "stdio.h"
#include "stdlib.h"

#if defined _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
static int inet_pton(int af, const char *src, void *dst){
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN+1];

  ZeroMemory(&ss, sizeof(ss));
  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
    switch(af) {
      case AF_INET:
    *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
    return 1;
      case AF_INET6:
    *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
    return 1;
    }
  }
  return 0;
}
static void init_socket_api(void){
    WSADATA mywsadata; //your wsadata struct, it will be filled by WSAStartup
    WSAStartup(0x0202,&mywsadata); //0x0202 refers to version of sockets we want to use.
}
static long unsigned int rx_bytes_available(int sockfd){
    long unsigned int bytes_available;
    ioctlsocket(sockfd,FIONREAD,&bytes_available);
    return bytes_available;
}
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
static void init_socket_api(void){}
static long unsigned int rx_bytes_available(int sockfd){
    long unsigned int bytes_available;
    ioctl(sockfd,FIONREAD,&bytes_available);
    return bytes_available;
}
#endif

typedef struct socket_params_struct_t {
    const char*address;//"127.0.0.1"
    uint32_t port;//5000
} socket_params_t;

static int socket_client_init(socket_params_t*params){
    //printf("socket_client_init %s : %u\n",params->address,params->port);
    struct sockaddr_in serv_addr;
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error : Could not create socket \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(params->port);
    if(inet_pton(AF_INET, params->address, &serv_addr.sin_addr)<=0){
        printf("\n inet_pton error occured\n");
        return -2;
    }
    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
       printf("\n Error : Connect Failed \n");
       return -3;
    }
    return sockfd;
}

static int socket_server_init(socket_params_t*params){
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(params->port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 1);//listen to a single connection

    int sockfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    return sockfd;
}
