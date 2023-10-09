#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 1068
#define PREFIX 0xFFEEDDCC
#define SUFFIX 0x98765432
#define PORT 31006
#define STDIN 0

struct dtgram{
  uint32_t colour;
  uint32_t prefix;
  char name[32];
  uint32_t suffix;
  char text[1024];
};

/// @brief Set params into dtrgram structure
/// @return Returns 0 if there are no error, or -1 if an error has occured
int init_dtgram(struct dtgram* message, int32_t colour, char* sender){
  int sender_len = strlen(sender);
  if (sender_len > 32) return -1;

  message->colour = colour;
  
  message->prefix = PREFIX;
  message->suffix = SUFFIX;

  strncpy(message->name, sender, 32);
  return 0;
}

/// @brief Send datagram to server
/// @return Returns 0 if there are no error, or -1 if an error has occured
int send_to_server(struct dtgram* message, char* text, int socket_desc, struct sockaddr_in* server){
  int text_len = strlen(text);
  if (text_len > 1024) return -1;

  strncpy(message->text, text, 1024);

  ssize_t sent = sendto(socket_desc,
                        message, BUFFSIZE,
                        0, (struct sockaddr *) server,
                        (socklen_t) sizeof(*server));
  if (sent <= 0) return -1;
  return 0;
}

void print_message(struct dtgram* message){
  printf("\033[%02xm%s\033[0m:%s", message->colour, message->name, message->text);
}

int read_from_server(struct dtgram* message, int socket_desc){
  ssize_t recieved = recvfrom(socket_desc, message, BUFFSIZE,
                              0, NULL, NULL);
  if (recieved <= 0) return -1;
  return 0;
}

int main(int argc, char** argv){
  int socket_desc;
  struct sockaddr_in server;
  struct dtgram message, data;

  if (argc < 3){
    printf("Usage client ID NAME IP\n");
    printf("Print \"!stop\" to exit from program\n");
    exit(1);
  }

  if (init_dtgram(&message, atoll(argv[1]), argv[2])){
    perror("error setting datagram");
    return __LINE__;
  }

#pragma region server_socket
  socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_desc < 0){
    perror("error calling server socket");
    return __LINE__;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr(argv[3]);
#pragma endregion server_socket

  fd_set read_fds;
  int fdmax = socket_desc;

  char *text = NULL;
  size_t len = 0;

  while(1){

    //its strange
    FD_ZERO(&read_fds);
    FD_SET(STDIN, &read_fds);
    FD_SET(socket_desc, &read_fds);

    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      return __LINE__;
    }

    // finding ready descriptors for reading
    for (size_t i = 0; i <= fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        if(i == 0){
          //printf("Произошел ввод с клаиватуры!\n");

          getline(&text, &len, stdin);

          if (send_to_server(&message, text, socket_desc, &server) < 0){
            perror("send to server");
            return __LINE__;
          }
        } else {
          //printf("Что-то прилетело в сокет!\n");

          if (read_from_server(&data, socket_desc) < 0){
            perror("read from server");
            return __LINE__;
          }
          print_message(&data);
        }
      }
    }
  }

  free(text);
  close(socket_desc);
}
