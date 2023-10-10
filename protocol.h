#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <unistd.h>

#define DTGRAMSIZE 1068
#define PREFIX 0xCCDDEEFF
#define SUFFIX 0x32547698
#define PORT 31006

struct dtgram{
  uint32_t colour;
  uint32_t prefix;
  char name[32];
  uint32_t suffix;
  char text[1024];
};

#endif
