#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

int addClient(struct sockaddr_in** clients, ssize_t* clients_len, struct sockaddr_in* client){
  int client_id = -1;
  for (size_t i = 0; i < *clients_len; i++){
    if((client->sin_addr.s_addr == (*clients)[i].sin_addr.s_addr) && (client->sin_port == (*clients)[i].sin_port)){
      client_id = i;
      break;
    }
  }
  if (client_id == -1){
    *clients_len += 1;
    *clients = realloc(*clients, sizeof(*client) * (*clients_len));
    (*clients)[*clients_len - 1] = *client;
    client_id = *clients_len - 1;
  }
  return client_id;
}

void broadcast(int socket, struct dtgram* message, size_t length, struct sockaddr_in* clients, ssize_t clients_len, int sender_id){
  socklen_t client_address_size;
  for (size_t i = 0; i < clients_len; i++){
    if (i == sender_id) continue;

    client_address_size = sizeof(clients[i]);

    if (sendto(socket, message, length, 0, (struct sockaddr *) &clients[i], client_address_size) < 0){
      printf("error sending message to socket id %d\n", sender_id);
    }
    printf("Port %d (%s) sended message \"%s\" to port: %d\n", clients[sender_id].sin_port,  message->name, message->text, clients[i].sin_port);
  }
  printf("\n");
}

/// @brief Is valid dgram
/// @return returns 0 if datagram is valid or less then 0 if is not
int validate_dgram(struct dtgram* message){
  if (message->colour < 48 || message->colour > 55) return -1;
  if (message->prefix != PREFIX) return -2;
  if (message->prefix != PREFIX) return -3;

  int sender_len = strlen(message->name);
  if (sender_len > 32) return -4;

  int text_len = strlen(message->text);
  if (text_len > 1024) return -5;

  if (message->text[0] == '\0') return -6;
  if (message->text[0] == '\n') return -7;
  
  return 0;
}

int main(int argc, char** argv){
  int s, client_address_size, client_id, recived;
  struct sockaddr_in client, server;
  struct sockaddr_in* clients = malloc(0);
  ssize_t clients_len = 0;
  struct dtgram message;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0){
    perror("error calling socket");
    return __LINE__;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(31006);
  server.sin_addr.s_addr = INADDR_ANY;

  if (bind(s, (struct sockaddr*)&server, sizeof(server)) < 0){
    perror("bind");
    return __LINE__;
  } 

  printf("Port assigned is %d\n", ntohs(server.sin_port));

  client_address_size = sizeof(client);

  FILE* file = fopen("newfile", "w");
  while(1){
    if((recived = recvfrom(s, &message, DTGRAMSIZE, 0, (struct sockaddr *) &client, (socklen_t *) &client_address_size)) >= 0){
      client_id = addClient(&clients, &clients_len, &client);
    }
    
    if (validate_dgram(&message) == 0){
      broadcast(s, &message, recived, clients, clients_len, client_id);
      fwrite(&message, sizeof(char), recived, file);
    }
  }
  
  fclose(file);
  close(s);
}
