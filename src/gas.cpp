#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <iostream>
#include <string>
#include <strstream>
#include <fstream>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <google/gflags.h>

#include "db.h"
#include "edge.h"
#include "pagerank.h"
#include "hits.h"
#include "spp.h"

DEFINE_string(c, "", "command");
DEFINE_string(d, "japanese", "default language");

using namespace std;

int main(int argc, char *argv[] )
{
  google::ParseCommandLineFlags(&argc, &argv, true);

  if(FLAGS_c == "init"){
    db_init();
  }else if(FLAGS_c == "edge_insert"){
    edge_insert();
  }else if(FLAGS_c == "edge_select"){
    edge_select();
  }else if(FLAGS_c == "spp"){
    spp();
  }else if(FLAGS_c == "pagerank"){
    pagerank();
  }else if(FLAGS_c == "hits"){
    hits();
  }
  
}
