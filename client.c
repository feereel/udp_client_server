#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 1068

struct message{
  unsigned long long index;
  char prefix[4];
  char name[32];
  char suffix[4];
  char text[1024];
};


int main(int argc, char** argv){
  int s;
  unsigned short port;
  struct sockaddr_in server;
  char buf[BUFFSIZE];

  if (argc < 3){
    printf("Usage client ID NAME TEXT IP PORT");
    exit(1);
  }

  char prefix[] = {0xFF, 0xEE, 0xDD, 0xCC };
  char suffix[] = {0x98, 0x76, 0x54, 0x32 };

  struct message package;
  package.index = atoll(argv[1]);
  memcpy(package.prefix, prefix, 4);
  strncpy(package.name, argv[2], 32);
  memcpy(package.suffix, suffix, 4);
  strncpy(package.text, argv[3], 32);

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0){
    perror("error calling socket");
    return __LINE__;
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(31006);
  server.sin_addr.s_addr = inet_addr("127.0.0.1");

  socklen_t len = sizeof(server);
  int sended = sendto(s, &package, BUFFSIZE, 0, (struct sockaddr *) &server, len);
  printf("%d\n", sended);

  while (1){
    char data[BUFFSIZE];
    recvfrom(s, &data, BUFFSIZE, 0, NULL, NULL);
    printf("\033[%02xm%s\033[0m:%s", *(int*)(data), data + 8, data + 44);
  }

  close(s);
}