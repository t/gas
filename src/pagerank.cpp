#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
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

template<typename T> int pagerank_calc_t()
{
  LOG(INFO) << "pagerank_calc_t is starting." << endl;

  const string adj_file      = db_path(FILE_ADJLIST_FORWARD);
  const string seq_file      = db_path(FILE_SEQUENCE);
  const string pagerank_file = db_path(FILE_PAGERANK);

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
      cout << "delta: " << delta << endl;
      cout << "eps  : " << FLAGS_eps   << endl;
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

int pagerank_select()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 3);


}

int pagerank_all()
{
  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(false);
  for(size_t i = 0; i < N; i++){
  }
  pagerank->close();
  delete pagerank;
}

int pagerank()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 2);

  const string adj_file      = db_path(FILE_ADJLIST_FORWARD);
  const string seq_file      = db_path(FILE_SEQUENCE);
  const string pagerank_file = db_path(FILE_PAGERANK);

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

