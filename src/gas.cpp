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
#include <boost/program_options.hpp>

#include "db.h"
#include "edge.h"
#include "pagerank.h"
#include "ipagerank.h"
#include "spp.h"
#include "hits.h"
#include "adjlist.h"
#include "alt_precompute.h"
#include "malt_precompute.h"
#include "spp_experiment.h"

#define CONFIG_FILE "/etc/graph.conf"

using namespace std;
using namespace boost;
using namespace boost::program_options;

int main(int argc, char *argv[] ){

  options_description opt("options");
  opt.add_options()
    ("help,h",                                                             "ヘルプを表示")
    ("data_dir",              value<string>(),                             "データディレクトリ")
    ("db,d",                  value<string>(),                             "データベース")
    ("command,c",             value<string>(),                             "コマンド(init,edge_select,edge_insert,pagerank)")
    ("source,s",              value<uint64_t>()->default_value(0),         "source")
    ("target,t",              value<uint64_t>()->default_value(0),         "target")
    ("spp_directional",       value<string>()->default_value("bi"),        "spp directional")
    ("spp_heuristic",         value<string>()->default_value("dijkstra"),  "spp heuristic")
    ("spp_landmark_strategy", value<string>()->default_value("avoid"),     "landmark strategy")
    ("spp_landmark_size",     value<int>()->default_value(16),             "landmark size")
    //("pagerank_seeds",        value<string>()->default_value(""),          "pagerank_seeds")
  ;

  variables_map vm;
  store(parse_command_line(argc, argv, opt), vm);
  std::ifstream ifs(CONFIG_FILE); 
  store(parse_config_file(ifs, opt), vm);

  notify(vm);

  if( vm.count("help") || !vm.count("data_dir") || !vm.count("db") || !vm.count("command") ) {
    cout << opt << endl; 
    return 0;
  }  

  const string   data_dir              = vm["data_dir"].as<string>();
  const string   db                    = vm["db"].as<string>();
  const string   db_dir                = data_dir + "/" + db + "/";
  const string   cmd                   = vm["command"].as<string>();
  const uint64_t source                = vm["source"].as<uint64_t>();
  const uint64_t target                = vm["target"].as<uint64_t>();
  const string   spp_directional       = vm["spp_directional"].as<string>();
  const string   spp_heuristic         = vm["spp_heuristic"].as<string>();
  const string   spp_landmark_strategy = vm["spp_landmark_strategy"].as<string>();
  const int      spp_landmark_size     = vm["spp_landmark_size"].as<int>();

  if(cmd == "init")
  {
    db_init(db_dir);
  }else if(cmd == "edge_insert"){
    edge_insert(db_dir);
  }else if(cmd == "edge_select"){

    if( (source == 0 && target == 0) || (source > 0 && target > 0) ){
      cout << opt << endl;
      return 0;
    }

    if(source > 0)
    {
      cout << "source = " << source << endl;
      edge_select(db_dir, source, true);
    }else{
      cout << "target = " << target << endl;
      edge_select(db_dir, target, false);
    }

  }else if(cmd == "edge_random")
  {
    edge_random(db_dir);
  }else if(cmd == "spp")
  {
    if(source == 0 || target == 0){
      cout << opt << endl;
      return 0;
    }

    if(spp_heuristic == "dijkstra"){
      if(spp_directional == "bi"){
        bidirectional_dijkstra(db_dir, source, target);
      }else if(spp_directional == "uni"){
        unidirectional_dijkstra(db_dir, source, target);
      }
    }else if(spp_heuristic == "alt"){
      if(spp_directional == "bi"){
        bidirectional_alt(db_dir, source, target, spp_landmark_strategy, spp_landmark_size);
      }else if(spp_directional == "uni"){
        
      }
    }else if(spp_heuristic == "malt"){
      if(spp_directional == "bi"){
        bidirectional_malt(db_dir, source, target, spp_landmark_strategy, spp_landmark_size);
      }else if(spp_directional == "uni"){
        unidirectional_malt(db_dir, source, target, spp_landmark_strategy, spp_landmark_size);
      }
    }else{
      cout << opt << endl;
      return 0;
    }

  }else if(cmd == "alt_precompute"){
    alt_precompute(db_dir, spp_landmark_strategy, spp_landmark_size);
  }else if(cmd == "malt_precompute"){
    malt_precompute(db_dir, spp_landmark_strategy, spp_landmark_size);
  }else if(cmd == "pagerank"){
    pagerank(db_dir);
  }else if(cmd == "ipagerank"){
    ipagerank(db_dir);
  }else if(cmd == "dbg")
  {
  }else if(cmd == "hits")
  {
    //hits(db_dir);
  }else if(cmd == "ielias_test")
  {
    adjlist_ielias_test(db_dir);
  }else if(cmd == "edge_random_sample"){
    edge_random_sample(db_dir);
  }else if(cmd == "alt_e"){
    spp_experiment(db_dir, "alt",  spp_landmark_strategy, spp_landmark_size);
  }else if(cmd == "malt_e"){
    spp_experiment(db_dir, "malt", spp_landmark_strategy, spp_landmark_size);
  }else if(cmd == "d_e"){
    spp_experiment(db_dir, "dijkstra", spp_landmark_strategy, spp_landmark_size);
  }

  
}
