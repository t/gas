#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"
#include "ielias_vector.h"
#include "ipagerank.h"

// FOR DEBUG
#include <ctime>

using namespace std;
using namespace boost;
using namespace boost::filesystem;

int ipagerank(const std::string& db_dir)
{
  const string hash = "testhash";
  const string adj_file      = db_dir + FILE_ADJLIST_IELIAS_FORWARD + hash;
  const string seq_file      = db_dir + FILE_SEQUENCE               + hash;
  const string pagerank_file = db_dir + FILE_IPAGERANK              + hash;

  seq_create(db_dir, hash);

  if(! exists(adj_file))
  {
    adjlist_ielias_create(db_dir, true, hash);
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

  iEliasVector<uint64_t> * adj = new iEliasVector<uint64_t>(adj_file);
  adj->open(false);

  const uint64_t N = seq_size(db_dir, hash);

  vector<double> vpr(N, 1./N);
  vector<double> tmp_vpr(N);

  uint64_t iteration_time = 0;
  double d, delta, pagerank_src, norm, pre_norm = 1.0;

  for (;iteration_time<itr_upper_bound;++iteration_time) {
    cout << "<" << time(0) - start_time << ">" << endl;
    cout << "iteration " << iteration_time << endl;

    iEliasVector<uint64_t>::iterator adj_i = adj->begin();
    uint64_t node_idx = 0;
	// 各種パラメータの初期化
	fill(tmp_vpr.begin(),tmp_vpr.end(),0.0);
	norm = 0.0;
	pagerank_src = 0.0;

    const int free_interval = 500000000;
    int free_i = 0;
    while ( adj_i != adj->end() ) {
	  // PageRank PAPER 2.6 Parameter E - calculation of pagerank_src will finish at the end of this loop
	  pagerank_src += 1.0 * alpha / N * vpr[node_idx];

      const uint64_t forward_num = (*adj_i); 
      uint64_t before_node = 0;
      adj_i++;
      // アウトリンクをまったく持たないノードへの処理
      if (forward_num == 0) {
         node_idx++;
         continue;
      }

      norm += vpr[node_idx] * (1.0 - alpha);
      const double score = vpr[node_idx++] / forward_num * (1.0 - alpha);
      for(uint64_t i = 0; i < forward_num; i++)
      {
        uint64_t idx = (*adj_i) + before_node;
        tmp_vpr[idx] += score;
        adj_i++;
        before_node = idx;
      }

      /* 
      free_i++; 
      if(free_i > free_interval)
      {
        cout << "refresh" << endl;
        adj->refresh();
        adjlist = adj->at(0);
      }*/
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


