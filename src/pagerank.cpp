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

template<typename T> int pagerank_calc_t()
{
  LOG(INFO) << "pagerank_calc_t is starting." << endl;

  const string adj_file      = db_path(FILE_ADJLIST_FORWARD);
  const string seq_file      = db_path(FILE_SEQUENCE);
  const string pagerank_file = pagerank_path();

  seq_create();

  if(! exists(adj_file))
    adjlist_create(true);

  if(! exists(adj_file)) return 0;

  MmapVector< T > * adj = new MmapVector< T >(adj_file);
  adj->open(false);

  const uint64_t N = seq_size();

  // 転置隣接行列の取得（隣接リストとして）
  T *adjlist = adj->at(0);

  vector<double> vpr(N, 1./N);
  vector<double> tmp_vpr(N);

  uint64_t iteration_time = 0;
  double d, delta, pagerank_src, norm, pre_norm = 1.0;
  const uint64_t max_list_idx = adj->size();

  for (;iteration_time < FLAGS_max_iteration; ++iteration_time) {
    LOG(INFO) << "iteration " << iteration_time << endl;

    uint64_t list_idx = 0;
    uint64_t node_idx = 0;

    // 各種パラメータの初期化
    fill(tmp_vpr.begin(),tmp_vpr.end(),0.0);
    norm = 0.0;
    pagerank_src = 0.0;

    size_t free_next = FLAGS_free_interval;
    while (list_idx<=max_list_idx) {
      // PageRank PAPER 2.6 Parameter E - calculation of pagerank_src will finish at the end of this loop
      pagerank_src += 1.0 * FLAGS_alpha / N * vpr[node_idx];

      const uint64_t forward_num = adjlist[list_idx];
      list_idx++;
      // アウトリンクをまったく持たないノードへの処理
      if (forward_num == 0) {
         node_idx++;
         continue;
      }

      norm += vpr[node_idx] * (1.0 - FLAGS_alpha);
      const double score = vpr[node_idx++] / forward_num * (1.0 - FLAGS_alpha);
      const uint64_t border = list_idx + forward_num;
      while (list_idx < border)
      {
        size_t idx = adjlist[list_idx++];
        tmp_vpr[idx] += score;
      }
  
      if(list_idx > free_next)
      {
        cout << "refresh" << endl;
        adj->refresh();
        adjlist = adj->at(0);
        free_next += FLAGS_free_interval;
        /*
        adj->madv_dontneed(total_free_count, list_idx - total_free_count);
        total_free_count = list_idx;
        */
      }
    }

    // add total pagerank source(caused by alpha) to make norm exactly
    norm += pagerank_src;

    pagerank_src = pagerank_src / N;

    // normalize and add pagerank_src
    d = pre_norm - norm;
    pre_norm = 0.0;

    LOG(INFO) << "d = " << d << " pr_src = " << pagerank_src << endl;

    delta = 0.0;
    for (uint64_t i=0;i<N;++i) {
      tmp_vpr[i] += pagerank_src + 1.0 / N * d;
      delta += fabs(tmp_vpr[i] - vpr[i]);
      pre_norm += tmp_vpr[i];
    }

    LOG(INFO) << "delta = " << delta << " norm = " << pre_norm << endl;

    // judge finishing convergence
    if (delta < FLAGS_eps) {
      LOG(INFO) << "delta: " << delta;
      LOG(INFO) << "eps  : " << FLAGS_eps;
      iteration_time++;
      break;
    }

    swap(vpr, tmp_vpr);
  }

  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(true);
  for(size_t i = 0; i < N; i++)
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

  const string adj_file      = db_path(FILE_ADJLIST_FORWARD);
  const string seq_file      = db_path(FILE_SEQUENCE);
  const string pagerank_file = pagerank_path();

  if(argvs.size() == 2    ||
     argvs[2] == "calc"   || 
     argvs[2] == "select" ||
     argvs[2] == "all"){ 

     LOG(INFO) << "checking pagerank file [" << pagerank_file << "]." ;
     if(! boost::filesystem::exists(pagerank_file)){
       seq_create();
       if(seq_32bit()) {
         pagerank_calc_t<uint32_t>();
       }else{
         pagerank_calc_t<uint64_t>();
       }
     }else{
       LOG(INFO) << "pagerank already computed.";
     }
  }

  if(argvs.size() == 2 || argvs[2] == "all"){
    return pagerank_all();
  }else if(argvs[2] == "select"){
    return pagerank_select();
  }

  return 0;
}

