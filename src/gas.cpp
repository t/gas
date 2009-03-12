#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <assert.h>
#include <google/gflags.h>
#include <glog/logging.h>

#include "db.h"
#include "edge.h"
#include "pagerank.h"
#include "hits.h"
#include "spp.h"

using namespace std;

DEFINE_string(db, "", "db path");

int main(int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);

  char *home(getenv("HOME"));
  if(home != NULL){
    string option_file = string(home) + "/.gas";
    FILE *fp = fopen(option_file.c_str(), "r");
    if(fp != NULL){
      fclose(fp);
      google::ReadFromFlagsFile(option_file, "gas", false);
    }
  }
  google::ParseCommandLineFlags(&argc, &argv, true);
  const vector<string> argvs = google::GetArgvs();

  assert(argvs.size() >= 1);

  LOG(INFO) << "gas started";
  LOG(INFO) << "db is ["      << FLAGS_db << "]" << endl;
  LOG(INFO) << "command is [" << argvs[1] << "]"<<  endl;

  if(argvs[1] == "db"){
    db();
  }else if(argvs[1] == "edge"){
    edge();
  }else if(argvs[1] == "spp"){
    spp();
  }

  /*
  }else if(argv[1] == "spp"){
    //spp(argc, argv);
  }else if(argv[1] == "pagerank"){
    //pagerank(argc, argv);
  }else if(argv[1] == "hits"){
    //hits(argc, argv);
  }else{
    cout << "Command Not Found [" << argvs[1] << "]" << endl;
  }
  */

  LOG(INFO) << "gas finished";
}

