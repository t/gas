#ifndef ADJLIST_H_
#define ADJLIST_H_

#define FILE_ADJLIST_FORWARD          "tmp_adjlist_forward"
#define FILE_ADJLIST_BACKWARD         "tmp_adjlist_backward"
#define FILE_ADJLIST_IELIAS_FORWARD   "tmp_adjlist_ielias_forward"
#define FILE_ADJLIST_IELIAS_BACKWARD  "tmp_adjlist_ielias_backward"

#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "btree.h"
#include "edge.h"
#include "seq.h"

int adjlist_create(bool is_foward);
int adjlist_ielias_create(bool is_foward);
int adjlist_ielias_test();
//int adjlist_hash_create(const std::string& db_dir, bool is_forward, const std::string& hash);

#endif
