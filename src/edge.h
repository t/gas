#ifndef EDGE_H_
#define EDGE_H_

#include <stdint.h>

#define FILE_EDGE_TEST     "edge_test"
#define FILE_EDGE_FORWARD  "edge_forward"
#define FILE_EDGE_BACKWARD "edge_backward"

struct Edge{
  public:
  uint64_t key;
  uint64_t to;
  uint64_t length;
};

bool operator < (const Edge &e, const Edge &f);

int edge_init();
int edge();

#endif

