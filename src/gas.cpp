#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <google/gflags.h>
#include <glog/logging.h>

#include "db.h"
#include "edge.h"
#include "spp.h"
#include "modularityq.h"
#include "adjlist.h"
#include "pagerank.h"
#include "hits.h"
#include "arg.h"
#include "supporters.h"

using namespace std;

DEFINE_string(db, "", "db path");
DEFINE_bool(tmp_clear, false, "tmp_clear");

int main(int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);
  init_arg(argc, argv);
  vector<string> argvs = get_argvs();

  LOG(INFO) << "gas started";
  LOG(INFO) << "db is ["      << FLAGS_db << "]" << endl;
  LOG(INFO) << "command is [" << argvs[1] << "]"<<  endl;

  if(FLAGS_tmp_clear){
    db_tmp_clear();
  }

  if(argvs[1] == "db"){
    db();
  }else if(argvs[1] == "edge"){
    edge();
  }else if(argvs[1] == "adjlist"){
    adjlist();
  }else if(argvs[1] == "spp"){
    spp();
  }else if(argvs[1] == "modularityq"){
    modularityq();
  }else if(argvs[1] == "pagerank"){
    pagerank();
  }else if(argvs[1] == "hits"){
    hits();
  }else if(argvs[1] == "supporters"){
    supporters();
  }

  LOG(INFO) << "gas finished";
}

