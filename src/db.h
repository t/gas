#ifndef DB_H_
#define DB_H_

#include <string>

const std::string db_path(const std::string& filename);
int db_tmp_clear();
int db();

#endif

