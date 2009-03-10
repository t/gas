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
  google::ParseCommandLineFlags(&argc, &argv, true);
 
  for(int i = 0; i < argc; i++) {
    printf("argv[%d] = \"%s\"\n", i, argv[i]);
  }
  assert(argc >= 1);

  LOG(INFO) << "gas started";
  LOG(INFO) << "db is " << FLAGS_db << endl;

  if(argv[1] == "db"){
    db(argc, argv);
  }else if(argv[1] == "edge"){
    //edge(argc, argv);
  }else if(argv[1] == "spp"){
    //spp(argc, argv);
  }else if(argv[1] == "pagerank"){
    //pagerank(argc, argv);
  }else if(argv[1] == "hits"){
    //hits(argc, argv);
  }

  LOG(INFO) << "gas finished";
}

