#ifndef DB_H_
#define DB_H_

#include <string>

int db_init(const std::string& db_dir);
int db(int argc, char *argv[]);

#endif

