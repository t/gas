#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <google/gflags.h>
#include <glog/logging.h>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"
#include "hits.h"
#include "db.h"
#include "arg.h"

/*
DEFINE_double(alpha,         0.15,      "Alpha");
DEFINE_double(eps,           1.0e-8,    "Eps");
DEFINE_int32 (max_iteration, 100,       "Max Iteration");
DEFINE_int32 (free_interval, 500000000, "free_interval");
*/

using namespace std;
using namespace boost;
using namespace boost::filesystem;

const string supporters_path()
{
  string ret = string(FILE_SUPPORTERS);
       //      + "_eps_"   + lexical_cast<string>(FLAGS_eps)
       //      + "_maxi_"  + lexical_cast<string>(FLAGS_max_iteration);

  return db_path(ret);
}

int supporters_calc()
{
  const vector<string> argvs = get_argvs();
  const string supporters_file = hits_path();


  delete algorithms;

  return 1;
}

int supporters_show()
{
/*
  const vector<string> argvs = get_argvs();

  const string hits_file = hits_path();
  const string seq_file  = db_path(FILE_SEQUENCE);

  const uint64_t N = seq_size();

  MmapVector< pair<double,double> > * hits = new MmapVector< pair<double,double> >(hits_file);
  hits->open(false);
  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  cout << "result:" << endl;
  cout << "  hits:" << endl;

  if(argvs.size() == 2 || argvs[2] == "" || argvs[2] == "all"){
    for(size_t i = 0; i < N; i++){
      pair<double,double> * p = hits->at(i);
      cout << "    " << (* seq->at(i) ) << ": {authority: " << p->first << ", hub: " << p->second << "}" << endl;
    }
  }else if(argvs[2] == "select"){
    assert(argvs.size() >= 4);
    uint64_t key = lexical_cast<uint64_t>(argvs[3]);
    uint64_t idx = seq_get(seq, key);
    pair<double,double> * p = hits->at(idx);
    cout << "    " << key << ": {authority: " << p->first << ", hub: " << p->second << "}" << endl; 
  }

  hits->close();
  seq->close();
  delete hits;
  delete seq;
*/
}

int supporters()
{
  const vector<string> argvs = get_argvs();
  assert(argvs.size() >= 2);

  supporters_calc();
  supporters_show();

  return 1;
}

