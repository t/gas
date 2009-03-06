#ifndef ADJLIST_H_
#define ADJLIST_H_

#define FILE_ADJLIST_FORWARD          "tmp_adjlist_forward_"
#define FILE_ADJLIST_BACKWARD         "tmp_adjlist_backward_"
#define FILE_ADJLIST_IELIAS_FORWARD   "tmp_adjlist_ielias_forward_"
#define FILE_ADJLIST_IELIAS_BACKWARD  "tmp_adjlist_ielias_backward_"

#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "btree.h"
#include "edge.h"
#include "seq.h"

int adjlist_create(const std::string& db_dir, bool is_foward, const std::string& hash);
int adjlist_ielias_create(const std::string& db_dir, bool is_foward, const std::string& hash);
int adjlist_ielias_test(const std::string& db_dir);
//int adjlist_hash_create(const std::string& db_dir, bool is_forward, const std::string& hash);

#endif
