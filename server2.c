#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define TRUE 1
#define FALSE 0
#define PORT 8888

// function declarations
int new_connection(int socket, struct sockaddr_in *address, int *addrlen);
void add_to_master_set(fd_set *master_set, int new_socket, int *max_sd, int *cc);
void user_disc(fd_set *master_set, int sd, int *cc, struct sockaddr_in *address, int *addrlen);
void snd_server_info(fd_set *master_set, int max_sd, int msock, int cc, char flag);
void rcv_request(char *buf, int requester, fd_set *master_set, int msock, int max_sd);

// MOTD
char *message = "Welcome to metou (me to you) v0.1\n";

int main(int argc, char *argv[])
{
  int opt = TRUE;
  int master_socket, addrlen, new_socket;
  int activity, valread, sd;
  int i;
  int max_sd;
  int conn_count = 0;
  struct sockaddr_in address;

  char buffer[1024];

  // set of socket descriptors
  fd_set master_set;
  fd_set readfds;

  // create a master socket
  if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // set master socket to allow multiple connections,
  // this is just a good habit, it will work without this
  if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt)) < 0){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // bind the socket to localhost port 8888
  if(bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  printf("Listener on port %d\n", PORT);

  // try to specify maximum of 3 pending connections for the master socket
  if(listen(master_socket, 3) < 0){
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // setup the master_set
  FD_SET(master_socket, &master_set);
  max_sd = master_socket;

  // accept the incoming connection
  addrlen = sizeof(address);
  puts("Waiting for connections ...");

  while(TRUE){
    // copy master_set to readfds
    readfds = master_set;

    
    // wait for an activity on one of the sockets, timeout is NULL
    // so wait indefinitely
    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if((activity < 0) && (errno != EINTR))
      printf("select error");

    // if something happened on the master socket,
    // then it is an incoming connection
    if(FD_ISSET(master_socket, &readfds))
    {
      new_socket = new_connection(master_socket, &address, &addrlen);
      add_to_master_set(&master_set, new_socket, &max_sd, &conn_count);
      snd_server_info(&master_set, max_sd, master_socket, conn_count, 'C');
    }

    else
    {
    // else it is some IO operation on some other socket
      for(i = 0; i <= max_sd; i++)
      {
        sd = i;

        if(FD_ISSET(sd, &readfds))
        {
          // read the incoming message
          if((valread = read(sd, buffer, 1024)) == 0)
          {
            /* if user disconnected */
            user_disc(&master_set, sd, &conn_count, &address, &addrlen);
            snd_server_info(&master_set, max_sd, master_socket, conn_count, 'D');
          }

          // echo back the message that came in
          else
          {
            // set the string terminating NULL byte on the end of the data read
            buffer[valread] = '\0';
            rcv_request(buffer, i, &master_set, master_socket, max_sd);

          } // else IO operation
        } // if FD_ISSET()
      } // for loop finding socket
    } // else incoming IO request or close connection
  }

  return 0;
}


int new_connection(int socket, struct sockaddr_in *address, int *addrlen)
{
  int new_socket;
  if((new_socket = accept(socket,
          (struct sockaddr *) address, (socklen_t*) addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  // inform user of socket number - used in send and receive commands
  printf("New connection, socket fd is %d, ip is : %s, port: %d\n",
      new_socket, inet_ntoa(address->sin_addr), ntohs(address->sin_port));

  // send new connection greeting message
  if(send(new_socket, message, strlen(message), 0) != strlen(message))
    perror("send");
  puts("Welcome message sent successfully");

  return new_socket;
}

void snd_server_info(fd_set *master_set, int max_sd, int msock, int cc, char flag)
{
  // TODO: send user count and username when username is implemented
  // inform users that a connection was made or dropped
  int j;
  for(j = 0; j <= max_sd; j++)
  {
    if(j == msock) continue;
    if(FD_ISSET(j, master_set))
    {
      char tmp[] = {flag,
        (cc >> 24) & 0xFF,
        (cc >> 16) & 0xFF,
        (cc >> 8) & 0xFF,
        cc & 0xFF, '\0'};
      send(j, tmp , sizeof(tmp), 0);
    }
  }
}

void add_to_master_set(fd_set *master_set, int new_socket, int *max_sd, int *cc)
{
  FD_SET(new_socket, master_set);
  if(new_socket > *max_sd)
    *max_sd = new_socket;
  (*cc)++;
  printf("Adding to master set of sockets as %d\n", new_socket);
}

void user_disc(fd_set *master_set, int sd, int *cc, struct sockaddr_in *address, int *addrlen)
{
  // somebody disconnected, get his details and print
  getpeername(sd, (struct sockaddr*) address,
      (socklen_t*) addrlen);
  printf("Host disconnected, ip %s, port %d\n",
      inet_ntoa(address->sin_addr), ntohs(address->sin_port));

  // close the socket and remove from master_set
  close(sd);
  FD_CLR(sd, master_set);
  (*cc)--;
}

void rcv_request(char *buf, int requester, fd_set *master_set, int msock, int max_sd)
{
  int j;
  switch(buf[0]){
    case 'M': // broadcast message to all sockets in master_set
      for(j = 0; j <= max_sd; j++){
        if(j == requester || j == msock) continue;
        if(FD_ISSET(j, master_set))
          send(j, buf, strlen(buf), 0);
      }
      break;
  }
}
