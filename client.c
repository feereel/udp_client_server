#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

#define TERM "!bye\n"
#define STDIN 0

/// @brief Set params into dtrgram structure
/// @param colour takes values from 1 to 8
/// @return Returns 0 if there are no error, -1 if len(name) > 32, -2 if colour not in [1,8]
int init_dtgram(struct dtgram* message, int32_t colour, char* sender){
  int sender_len = strlen(sender);
  if (sender_len > 32) return -1;
  if (colour < 1 || colour > 8) return -2;

  message->colour = colour + 47;
  
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
                        message, DTGRAMSIZE,
                        0, (struct sockaddr *) server,
                        (socklen_t) sizeof(*server));
  if (sent <= 0) return -1;
  return 0;
}

void input_prompt(int32_t colour, char* name){
  printf("\033[%02xm#%s -> \033[0m", colour + 47, name);
  fflush(stdout);
}

void print_message(struct dtgram* message){
  printf("\33[2K\r");
  printf("\033[%02xm%s\033[0m <- %s", message->colour, message->name, message->text);
}

int read_from_server(struct dtgram* message, int socket_desc){
  ssize_t recieved = recvfrom(socket_desc, message, DTGRAMSIZE,
                              0, NULL, NULL);
  if (recieved <= 0) return -1;
  return 0;
}

int main(int argc, char** argv){
  int socket_desc, fdmax;
  struct sockaddr_in server;
  struct dtgram message, data;
  fd_set read_fds;
  size_t len = 0;
  char *text = NULL;

  if (argc < 3){
    char usage[] = "Usage: ./client.exe COLOUR NAME IP\n"
                   "\tCOLOUR takes values from 1 to 8\n"
                   "\tE.g.: ./client.exe 1 Bob 127.0.0.1\n";
    printf("%s", usage);
    exit(0);
  }

  int32_t colour = atoll(argv[1]);
  char* name = argv[2];
  char* ip = argv[3];

  if (init_dtgram(&message, colour, name)){
    perror("error setting datagram");
    return __LINE__;
  }

#pragma region set_socket
  socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_desc < 0){
    perror("error calling server socket");
    return __LINE__;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = inet_addr(ip);
#pragma endregion

  fdmax = socket_desc;

  char info[] = "Print \"!bye\" to exit from program\n\n";
  printf("%s", info);
  input_prompt(colour, name);

  while(1){
    FD_ZERO(&read_fds);
    FD_SET(STDIN, &read_fds);
    FD_SET(socket_desc, &read_fds);

    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      return __LINE__;
    }

    // finding ready descriptors for reading
    if(FD_ISSET(STDIN, &read_fds)){
      getline(&text, &len, stdin);

      if(!strcmp(text, TERM)){
        printf("Safe exit\n");
        break;
      }

      if(send_to_server(&message, text, socket_desc, &server) < 0){
        perror("send to server");
        return __LINE__;
      }
      input_prompt(colour, name);

      free(text);
      text = NULL;
    }

    if(FD_ISSET(socket_desc, &read_fds)){
      if (read_from_server(&data, socket_desc) < 0){
        perror("read from server");
        return __LINE__;
      }
      
      print_message(&data);
      input_prompt(colour, name);
    }
  }

  close(socket_desc);
}
