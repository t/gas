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

int edge_init(const std::string& db_dir);
int edge_select(const std::string& db_dir, uint64_t key, bool is_forward);
int edge_insert(const std::string& db_dir);
int edge_random(const std::string& db_dir);
int edge_test(const std::string& db_dir);
int edge_walker_test(const std::string& db_dir);
int edge_hadoop_adj_test(const std::string& db_dir);
int edge_iterator_test(const std::string& db_dir);
int edge_from_count(const std::string& db_dir);
int edge_random_sample(const std::string& db_dir);

#endif

