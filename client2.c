#include "client2.h"

void getinput(char *buffer, int *size);

int main(int argc, char const *argv[])
{
  int sock = 0;
  int valread;
  struct sockaddr_in serv_addr;
  char buf_send[BUFMAX] = {0};
  char buf_rcv[BUFMAX] = {0};
  int size;
  int activity;

  // set of socket descriptors.
  fd_set readfds;

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("\n Socket creation error\n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // convert ipv4 and ipv6 addresses from text to binary form
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
    printf("\nInvalid address/ Address not supported\n");
    return -1;
  }

  if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    printf("\nConnection failed\n");
    return -1;
  }


  // get and print greeting message
  valread = read(sock, buf_rcv, BUFMAX);
  printf("%s\n", buf_rcv);
  memset(buf_rcv, 0, BUFMAX); // clear the buffer

  while(TRUE){

    FD_ZERO(&readfds); // clear readfds
    FD_SET(0, &readfds); // add stdin to readfds
    FD_SET(sock, &readfds); // add socket to server

    // listen for activity on readfds
    activity = select(sock + 1, &readfds, NULL, NULL, NULL);
    if(activity < 0){
      printf("Error: select error");
      continue;
    }

    // input from stdin
    if(FD_ISSET(0, &readfds)){
      getinput(buf_send, &size);
      if(size > 1) send(sock, buf_send, size, 0);
    }

    // else it is input from server
    else{
      if((valread = read(sock, buf_rcv, BUFMAX)) > 0){
        puts(buf_rcv);
        memset(buf_rcv, 0, BUFMAX);
      }
    }
  }

  return 0;
}

void getinput(char *buffer, int *size)
{
  int i;
  memset(buffer, 0, BUFMAX);
  for(i = 0; i < BUFMAX - 1; i++){
    buffer[i] = getchar();
    if(buffer[i] == '\n') break;
  }
  buffer[i+1] = '\0';
  *size = i;
}
