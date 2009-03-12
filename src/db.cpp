#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <google/gflags.h>
#include <glog/logging.h>
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
  LOG(INFO) << "db_init is initializing [" << FLAGS_db << "]" << endl;

  create_directory(path(FLAGS_db));
  edge_init();

  LOG(INFO) << "db_init is complete." << endl;
  return 1;
}

const string db_path(const string& filename)
{
  string ret = FLAGS_db + "/" + filename;
  return ret;
}

int db()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 2);

  if(argvs[2] =="init"){
    return db_init();
  }
  return 0;
}

