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
#include "pagerank.h"
#include "adjlist.h"
#include "seq.h"

DECLARE_string(db);

using namespace std;
using namespace boost;
using namespace boost::filesystem;

bool db_check()
{
  return true;
}

bool db_remove_file(const string & filename)
{
  const string filepath = db_path(filename);
  LOG(INFO) << "Removing [" << filepath;
  remove(filepath);
}

int db_tmp_clear()
{
  db_remove_file(FILE_PAGERANK);
  db_remove_file(FILE_ADJLIST_FORWARD);
  db_remove_file(FILE_ADJLIST_BACKWARD);
  db_remove_file(FILE_SEQUENCE);
}

int db_init()
{
  LOG(INFO) << "db_init is initializing [" << FLAGS_db << "]" << endl;

  create_directory(path(FLAGS_db));
  edge_init();
  db_tmp_clear();

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
  }else if(argvs[2] == "tmp_clear"){
    return db_tmp_clear();
  }
  return 0;
}

