#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"
#include "pagerank.h"

// FOR DEBUG
#include <ctime>

using namespace std;
using namespace boost;
using namespace boost::filesystem;

template<typename T> int pagerank_t(const std::string& db_dir)
{
  const string hash = "testhash";
  const string adj_file      = db_dir + FILE_ADJLIST_FORWARD + hash;
  const string seq_file      = db_dir + FILE_SEQUENCE        + hash;
  const string pagerank_file = db_dir + FILE_PAGERANK        + hash;

  seq_create(db_dir, hash);

  if(! exists(adj_file))
  {
    adjlist_create(db_dir, true, hash);
  }
  if(! exists(adj_file)) return 0;

// デバッグON！！！
#define _DEBUG_

  /** テストコードのため各種パラメータは定数で代用 **/
  const double alpha = 0.15;       // 緩衝係数
  const double eps = 1.0e-8;        // 許容誤差（収束判定用）
  const int itr_upper_bound = 100; // 最大イテレーション回数
  /**************************************************/

#ifdef _DEBUG_
  time_t start_time = time(0);
  cout << "<start>" << endl;
#endif

  MmapVector< T > * adj = new MmapVector< T >(adj_file);
  adj->open(false);

  const uint64_t N = seq_size(db_dir, hash);

  // 転置隣接行列の取得（隣接リストとして）
  T *adjlist = adj->at(0);

  vector<double> vpr(N, 1./N);
  vector<double> tmp_vpr(N);

  uint64_t iteration_time = 0;
  double d, delta, pagerank_src, norm, pre_norm = 1.0;
  const uint64_t max_list_idx = adj->size();

  for (;iteration_time<itr_upper_bound;++iteration_time) {
    cout << "<" << time(0) - start_time << ">" << endl;
    cout << "iteration " << iteration_time << endl;

    uint64_t list_idx = 0;
    uint64_t node_idx = 0;
	// 各種パラメータの初期化
	fill(tmp_vpr.begin(),tmp_vpr.end(),0.0);
	norm = 0.0;
	pagerank_src = 0.0;

    const int free_interval = 500000000;
    size_t free_next = free_interval;
    while (list_idx<=max_list_idx) {
	  // PageRank PAPER 2.6 Parameter E - calculation of pagerank_src will finish at the end of this loop
	  pagerank_src += 1.0 * alpha / N * vpr[node_idx];

      const uint64_t forward_num = adjlist[list_idx];
      list_idx++;
      // アウトリンクをまったく持たないノードへの処理
      if (forward_num == 0) {
         node_idx++;
         continue;
      }

      norm += vpr[node_idx] * (1.0 - alpha);
      const double score = vpr[node_idx++] / forward_num * (1.0 - alpha);
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
        free_next += free_interval;
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

#ifdef _DEBUG_
    cout << "d = " << d << " pr_src = " << pagerank_src << endl;
#endif

    delta = 0.0;
    for (uint64_t i=0;i<N;++i) {
      tmp_vpr[i] += pagerank_src + 1.0 / N * d;
      delta += fabs(tmp_vpr[i] - vpr[i]);
      pre_norm += tmp_vpr[i];
    }

#ifdef _DEBUG_
    cout << "delta = " << delta << " norm = " << pre_norm << endl;
#endif

    // judge finishing convergence
    if (delta < eps) {
#ifdef _DEBUG_
		cout << "delta: " << delta << endl;
		cout << "eps  : " << eps << endl;
#endif
      iteration_time++;
      break;
    }

    swap(vpr,tmp_vpr);
  }

  cout << endl;
  cout << ">>>>>>>>>>>>>>>>>" << endl;
  cout << "RESULT" << endl;
  cout << "<" << time(0) - start_time << ">" << endl;
  cout << "Iter Time = " << iteration_time << endl;

  MmapVector<double> * pagerank = new MmapVector<double>(pagerank_file);
  pagerank->open(true);
  for(size_t i = 0; i < N; i++)
  {
    cout << "i = " << vpr[i] << endl;
    pagerank->push_back(vpr[i]);
  }
  pagerank->close();
  delete pagerank;

  return 1;
}

int pagerank(const std::string& db_dir)
{
  const string hash = "testhash";
  const string adj_file = db_dir + FILE_ADJLIST_FORWARD + hash;
  const string seq_file = db_dir + FILE_SEQUENCE        + hash;
  const string pagerank_file = db_dir + FILE_PAGERANK   + hash;
  
  if(boost::filesystem::exists(pagerank_file))
  {
    return 1;
  }

  seq_create(db_dir, hash);

  if(seq_32bit(db_dir, hash))
  {
    pagerank_t<uint32_t>(db_dir);
  }else{
    pagerank_t<uint64_t>(db_dir);
  }
}

