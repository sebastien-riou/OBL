#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <stdint.h>

#include "portable_socket.h"
int sockfd = 0;
static void obl_putchar(char c){
  send(sockfd, &c, 1,0);
}
static char obl_getchar(){
  char c;
  recv(sockfd, &c, 1, MSG_WAITALL);
  return c;
}

#define OBL_CUSTOM_IO
#include "obl_device.h"

int main(int argc, char *argv[]){
    uint32_t port = 5000;
    if(argc>1) port = strtol(argv[1], 0, 0);

    socket_params_t params;
    params.address="127.0.0.1";
    params.port = port;
    init_socket_api();
    sockfd = socket_client_init(&params);

    if(sockfd <= 0) return sockfd;
    printf("Entering OBL\n");
    obl_main();
    printf("OBL exited\n");

    return 0;
}
