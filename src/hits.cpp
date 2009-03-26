#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "hits.h"
#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"

DEFINE_double(alpha,         0.15,      "Alpha");
DEFINE_double(eps,           1.0e-8,    "Eps");
DEFINE_int32 (max_iteration, 100,       "Max Iteration");
DEFINE_int32 (free_interval, 500000000, "free_interval");

using namespace std;
using namespace boost;
using namespace boost::filesystem;

const string hits_path()
{
  string ret = string(FILE_HITS)
             + "_eps_"   + lexical_cast<string>(FLAGS_eps)
             + "_maxi_"  + lexical_cast<string>(FLAGS_max_iteration);

  return db_path(ret);
}

int hits (const std::string& db_dir)
{
  const string hash = "testhash"; 
  const string adj_file = db_dir + FILE_ADJLIST_FORWARD + hash;
  const string seq_file = db_dir + FILE_SEQUENCE        + hash;

  if(! exists(adj_file))
  {
    adjlist_create(db_dir, hash);
  }
  if(! exists(adj_file)) return 0;

// デバッグON！！！
#define _DEBUG_

  /** テストコードのため各種パラメータは定数で代用 **/
  const int itr_upper_bound = 100; // 最大イテレーション回数
  const double eps = 1.0e-8;       // 収束判定用の定数 
  /**************************************************/

#ifdef _DEBUG_
  time_t start_time = time(0);
  cout << "<start>" << endl;
#endif

  MmapVector< uint64_t > * adj = new MmapVector< uint64_t >(adj_file);
  adj->open(false);

  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  // 転置隣接行列の取得（隣接リストとして）
  uint64_t *adjlist = adj->at(0);
  const uint64_t N = seq->size();
 
  vector<double> vauth(N,1.0); 
  vector<double> vhub(N,0.0);
  vector<double> vtmp(N);
  
  uint64_t iteration_time = 0;
  const uint64_t max_list_idx = adj->size() - 1;

#ifdef _DEBUG_
  if (max_list_idx<100) {
    cout << ">>>> adjlist >>>>>" << endl;
    for (uint64_t i=0;i<max_list_idx;++i)
      cout << adjlist[i] << " ";
    cout << endl;
	  
    cout << ">>>> adjlist >>>>>" << endl;
    uint64_t tmp_idx = 0;
    while (tmp_idx <= max_list_idx) {
      uint64_t colnum = adjlist[tmp_idx];
      tmp_idx+=2;
      for (uint64_t i=0;i<colnum;++i)
        cout << adjlist[tmp_idx++] << " ";
      cout << endl;
    }
    cout << "<<<<<<<<<<<<<<<<<<" << endl;
  }
#endif

  for (;iteration_time<itr_upper_bound;++iteration_time) {
#ifdef _DEBUG_
    cout << "<" << time(0) - start_time << ">" << endl;
    cout << "iteration " << iteration_time << endl;

    cout << "auth => ";
    for (uint64_t i=0;i<N;++i)
      cout << vauth[i] << " ";
    cout << endl;
    cout << "hub => ";
    for (uint64_t i=0;i<N;++i)
      cout << vhub[i] << " ";
    cout << endl;
#endif

    uint64_t list_idx = 0;
    uint64_t node_idx = 0;
    double norm;

    // Hubの計算
    // 各種パラメータの初期化
    swap(vhub,vtmp);
    fill(vhub.begin(),vhub.end(),0.0);
    norm = 0.0;
    while (list_idx<=max_list_idx) {
      const uint64_t forward_num = adjlist[list_idx];
      list_idx += 2;
      const uint64_t boarder = list_idx + forward_num;
      while (list_idx < boarder) {
        vhub[adjlist[list_idx++]] += vauth[node_idx];
        norm += vauth[node_idx];
      }
      ++node_idx;
    }
    // Hubの正規化・誤差判定
    double hub_diff = 0.0;
    for (uint64_t i=0;i<N;++i) {
      vhub[i] /= norm;
      hub_diff += fabs(vhub[i] - vtmp[i]);
    }

#ifdef _DEBUG_
    cout << "half <" << time(0) - start_time << ">" << endl;
#endif

    // Authorityの計算
    // 各種パラメータの初期化
    list_idx = 0;
    node_idx = 0;
    swap(vtmp,vauth);
    fill(vauth.begin(),vauth.end(),0.0);
    norm = 0.0;
    while (list_idx<=max_list_idx) {
      const uint64_t forward_num = adjlist[list_idx];
      list_idx += 2;
      const uint64_t boarder = list_idx + forward_num;
      while (list_idx < boarder)
        vauth[node_idx] += vhub[adjlist[list_idx++]];
      norm += vauth[node_idx++];
    }
    // Authorityの正規化・誤差判定
    double auth_diff = 0.0;
    for (uint64_t i=0;i<N;++i) {
      vauth[i] /= norm;
      auth_diff += fabs(vauth[i] - vtmp[i]);
    }

#ifdef _DEBUG_
    cout << "eps = " << eps << " auth_diff = " << auth_diff << " hub_diff = " << hub_diff << endl;

    // judge finishing convergence
    if (auth_diff < eps && hub_diff < eps) {
#endif
      iteration_time++;
      break;
    }
  }

#ifdef _DEBUG_
   cout << endl;
   cout << ">>>>>>>>>>>>>>>>>" << endl;
   cout << "RESULT" << endl;
   cout << "<" << time(0) - start_time << ">" << endl;
   cout << "Iter Time = " << iteration_time << endl;
   for (uint64_t i=0;i<N;++i)
      cout << "ID:" << i << "\t" << vauth[i] << " " << vhub[i] << endl;
   cout << "<<<<<<<<<<<<<<<<<" << endl;
#endif

  return 1;
}

int hits()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 2);

  const string adj_file  = db_path(FILE_ADJLIST_FORWARD);
  const string seq_file  = db_path(FILE_SEQUENCE);
  const string hits_file = pagerank_path();

  if(argvs.size() == 2    ||
     argvs[2] == "calc"   ||
     argvs[2] == "select" ||
     argvs[2] == "all"){

     LOG(INFO) << "checking hits file [" << hits_file << "]." ;
     if(! boost::filesystem::exists(hits_file)){
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

