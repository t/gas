#ifndef SPP_H_
#define SPP_H_

int unidirectional_dijkstra(const std::string& db_dir, uint64_t source, uint64_t target);
int unidirectional_malt(const std::string& db_dir, uint64_t start, uint64_t target, const std::string& strategy, const int& landmark_size);

int bidirectional_dijkstra(const std::string& db_dir, uint64_t source, uint64_t target);
int bidirectional_alt(const std::string& db_dir, uint64_t start, uint64_t target, const std::string& strategy, const int& landmark_size);
int bidirectional_malt(const std::string& db_dir, uint64_t start, uint64_t target, const std::string& strategy, const int& landmark_size);

#endif
