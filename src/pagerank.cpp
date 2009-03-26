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
#include "pagerank.h"
#include "db.h"

DEFINE_double(alpha,         0.15,      "Alpha");
DEFINE_double(eps,           1.0e-8,    "Eps");
DEFINE_int32 (max_iteration, 100,       "Max Iteration");
DEFINE_int32 (free_interval, 500000000, "free_interval");

using namespace std;
using namespace boost;
using namespace boost::filesystem;

const string pagerank_path()
{       
  string ret = string(FILE_PAGERANK)
             + "_alpha_" + lexical_cast<string>(FLAGS_alpha)
             + "_eps_"   + lexical_cast<string>(FLAGS_eps)
             + "_maxi_"  + lexical_cast<string>(FLAGS_max_iteration);
    
  return db_path(ret);
}

int pagerank_calc()
{
  const string pagerank_file = pagerank_path();

  LOG(INFO) << "checking pagerank file [" << pagerank_file << "]." ;
  if(boost::filesystem::exists(pagerank_file)){
    LOG(INFO) << "pagerank already computed.";
    return 1;
  }

  LOG(INFO) << "pagerank_calc_t is starting." << endl;

  Adjlist adj = adjlist_create(true);
  adj.open();

  uint64_t node_count = (* adj.begin() );

  vector<double> vpr(node_count,  1. / node_count);

  double d, delta, pagerank_src, norm, pre_norm = 1.0;
  for (size_t iteration = 0; iteration < FLAGS_max_iteration; ++iteration) {
    LOG(INFO) << "iteration: " << iteration << endl;

    vector<double> tmp_vpr(node_count, 0.0);
    norm         = 0.0;
    pagerank_src = 0.0;

    size_t node_i = 0;
    Adjlist::iterator adj_last = adj.end();
    for(Adjlist::iterator adj_i = adj.begin(); adj_i != adj_last; adj_i++){
      // PageRank PAPER 2.6 Parameter E - calculation of pagerank_src will finish at the end of this loop
      pagerank_src += 1.0 * FLAGS_alpha / node_count * vpr[node_i];

      norm += vpr[node_i] * (1.0 - FLAGS_alpha);
      const uint64_t edge_count = (*adj_i);
      const double   score      = vpr[node_i] / edge_count * (1.0 - FLAGS_alpha);
      for(size_t i = 0; i < edge_count; i++){
        adj_i++;
        tmp_vpr[(*adj_i)] += score;
      }
      node_i++;
    }

    norm        += pagerank_src;
    pagerank_src = pagerank_src / node_count;

    // normalize and add pagerank_src
    d = pre_norm - norm;
    pre_norm = 0.0;

    LOG(INFO) << "d = " << d << " pr_src = " << pagerank_src << endl;

    delta = 0.0;
    for(size_t i = 0;  i< node_count; ++i) {
      tmp_vpr[i] += pagerank_src + 1.0 / node_count * d;
      delta      += fabs(tmp_vpr[i] - vpr[i]);
      pre_norm   += tmp_vpr[i];
    }

    LOG(INFO) << "delta = " << delta << " norm = " << pre_norm << endl;

    // judge finishing convergence
    if (delta < FLAGS_eps) {
      LOG(INFO) << "delta: " << delta;
      LOG(INFO) << "eps  : " << FLAGS_eps;
      iteration++;
      break;
    }

    swap(vpr, tmp_vpr);
  }

  adj.close();

  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(true);
  for(size_t i = 0; i < node_count; i++)
  {
    pagerank->push_back(vpr[i]);
  }
  pagerank->close();
  delete pagerank;

  return 1;
}

int pagerank_all()
{
  const vector<string> argvs = google::GetArgvs();

  const string pagerank_file = pagerank_path();
  const string seq_file      = db_path(FILE_SEQUENCE);

  const uint64_t N = seq_size();

  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(false);
  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  cout << "result:" << endl;
  cout << "  pagerank:" << endl;
  for(size_t i = 0; i < N; i++){
    double * p = pagerank->at(i);
    cout << "    " << (* seq->at(i) ) << ": " << (*p) << endl;
  }

  pagerank->close();
  seq->close();
  delete pagerank;
  delete seq;
}

int pagerank_select()
{
  const vector<string> argvs = google::GetArgvs();
  const string pagerank_file = pagerank_path();
  const string seq_file      = db_path(FILE_SEQUENCE);

  const uint64_t N = seq_size();

  assert(argvs.size() >= 4);
  uint64_t key = lexical_cast<uint64_t>(argvs[3]);

  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(false);
  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  uint64_t idx = seq_get(seq, key);

  double * p = pagerank->at(idx);

  cout << "result:" << endl;
  cout << "  pagerank:" << endl;
  cout << "    " << key << ": " << (*p) << endl;

  pagerank->close();
  seq->close();
  delete pagerank;
  delete seq;
}

int pagerank()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 2);

  if(argvs.size() == 2    ||
     argvs[2] == "calc"   || 
     argvs[2] == "select" ||
     argvs[2] == "all"){ 

    pagerank_calc();
  }

  if(argvs.size() == 2 || argvs[2] == "all"){
    return pagerank_all();
  }else if(argvs[2] == "select"){
    return pagerank_select();
  }

  return 0;
}

