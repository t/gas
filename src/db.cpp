#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <google/gflags.h>
#include "db.h"
#include "edge.h"

DECLARE_string(db);

using namespace std;
using namespace boost;
using namespace boost::filesystem;

bool db_check()
{
  return true;
}

int db_init()
{
  create_directory(path(FLAGS_db));
  edge_init();
}

int db(int argc, char *argv[])
{
  assert(argc >= 2);

  if(argv[2] == "init"){
    db_init();
  }
}

