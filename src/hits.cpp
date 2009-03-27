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

/*
DEFINE_double(alpha,         0.15,      "Alpha");
DEFINE_double(eps,           1.0e-8,    "Eps");
DEFINE_int32 (max_iteration, 100,       "Max Iteration");
DEFINE_int32 (free_interval, 500000000, "free_interval");
*/

using namespace std;
using namespace boost;
using namespace boost::filesystem;

const string hits_path()
{
  string ret = string(FILE_HITS);
       //      + "_eps_"   + lexical_cast<string>(FLAGS_eps)
       //      + "_maxi_"  + lexical_cast<string>(FLAGS_max_iteration);

  return db_path(ret);
}

int hits_calc()
{
  const string hits_file = hits_path();

  LOG(INFO) << "checking hits file [" << hits_file << "]." ;
  if(boost::filesystem::exists(hits_file)){
    LOG(INFO) << "hits already computed.";
    return 1;
  }

  LOG(INFO) << "hits_calc is starting." << endl;

  Adjlist adj = adjlist_create(true);
  adj.open();

  uint64_t node_count = (* adj.begin() );

  const int itr_upper_bound = 100; // 最大イテレーション回数
  const double eps = 1.0e-8;       // 収束判定用の定数 

  vector<double> vauth(node_count, 1.0); 
  vector<double> vhub( node_count, 0.0);
  vector<double> vtmp( node_count);
  
  uint64_t iteration_time = 0;

  for (;iteration_time<itr_upper_bound;++iteration_time) {
    LOG(INFO) << "iteration " << iteration_time << endl;

    uint64_t list_idx = 0;
    uint64_t node_idx = 0;
    double norm;

    // Hubの計算
    // 各種パラメータの初期化
    swap(vhub,vtmp);
    fill(vhub.begin(),vhub.end(),0.0);
    norm = 0.0;

    size_t node_i = 0;
    Adjlist::iterator adj_last = adj.end();
    for(Adjlist::iterator adj_i = adj.begin(); adj_i != adj_last; adj_i++){
      const uint64_t edge_count = (*adj_i);
      for(size_t i = 0; i < edge_count; i++){
        adj_i++;
        vhub[(*adj_i)] += vauth[node_i];
        norm           += vauth[node_i];
      }
      ++node_i;
    }
    // Hubの正規化・誤差判定
    double hub_diff = 0.0;
    for (uint64_t i = 0; i < node_count; ++i) {
      vhub[i] /= norm;
      hub_diff += fabs(vhub[i] - vtmp[i]);
    }

    // Authorityの計算
    // 各種パラメータの初期化
    node_i = 0;
    swap(vtmp,vauth);
    fill(vauth.begin(),vauth.end(),0.0);
    norm = 0.0;
    for(Adjlist::iterator adj_i = adj.begin(); adj_i != adj_last; adj_i++){
      const uint64_t edge_count = (*adj_i);
      for(size_t i = 0; i < edge_count; i++){
        adj_i++;
        vauth[node_i] += vhub[(*adj_i)];
      }
      norm += vauth[node_i];
      ++node_i;
    }
    // Authorityの正規化・誤差判定
    double auth_diff = 0.0;
    for (uint64_t i=0;i<node_count;++i) {
      vauth[i] /= norm;
      auth_diff += fabs(vauth[i] - vtmp[i]);
    }

    LOG(INFO) << "eps = " << eps << " auth_diff = " << auth_diff << " hub_diff = " << hub_diff << endl;

    // judge finishing convergence
    if (auth_diff < eps && hub_diff < eps) {
      iteration_time++;
      break;
    }
  }

  adj.close();

  MmapVector< pair<double, double> > * hits = new MmapVector< pair<double,double> >(hits_file);
  hits->open(true);
  for(size_t i = 0; i < node_count; i++)
  {
    hits->push_back(make_pair(vauth[i], vhub[i]));
  }
  hits->close();
  delete hits;

  return 1;
}

int hits_show()
{
  const vector<string> argvs = google::GetArgvs();

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
}

int hits()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 2);

  hits_calc();
  hits_show();

  return 1;
}

