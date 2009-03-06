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
#include "robust_pagerank.h"

// FOR DEBUG
#include <ctime>

using namespace std;
using namespace boost;
using namespace boost::filesystem;

#define ERR 1e-9

template<typename T> 
void push_back(const double pr, MmapVector<T> * b_adj, const int link_start, const int link_count, vector<double> * c, vector<double> * r, const T u) 
{
  double u_score = (* r)[u];
  double ALPHA = 0.15;

  (* c)[u] += (1-ALPHA) * u_score;
  (* r)[u] = 0;
  for(size_t i = 0; i < link_count; i++)
  {
    T v = (* b_adj->at(link_start + i) );
    (* r)[v] += ALPHA * u_score / link_count;
  }
}

template<typename T> 
void approx_contribution(const T v, const double pr, MmapVector<T> * b_adj, vector<double> * res, const size_t N)
{
  double EPS = 0.1;

  vector<double> r(N, 0);
  r[v] = 1;

  bool done = false;
  while(! done)
  {
    done = false;
    T u = 0;
    for(size_t i = 0; i < b_adj->size(); i++) 
    {
      T link_count = (* b_adj->at(i));
      if(r[u] > EPS + ERR)
      {
        push_back(pr, b_adj, i + 1, link_count, res, &r, u);
        done = false;
        break;
      }
      i+= link_count + 1;
      u++;
    }
  }
}

template<typename T>
vector<T> delta_significant_contributing_set(vector<double> * c, const T v, const double pr, const size_t N, const double DELTA)
{
  vector<T> ret;

  for(size_t i = 0; i < N; i++)
  {
    if((* c)[i] > DELTA * pr + ERR)
    {
      ret.push_back(i);
    }
  }
  return ret;
}

template<typename T>
int robust_pagerank_t(const std::string& db_dir, const uint64_t& key)
{
  const string hash = "testhash";
  const string f_adj_file    = db_dir + FILE_ADJLIST_FORWARD  + hash;
  const string b_adj_file    = db_dir + FILE_ADJLIST_BACKWARD + hash;
  const string pagerank_file = db_dir + FILE_PAGERANK         + hash;
  const string seq_file      = db_dir + FILE_SEQUENCE         + hash;

  const double DELTA = 0;

  adjlist_create(db_dir, true,  hash);
  adjlist_create(db_dir, false, hash);
  pagerank(db_dir);

  MmapVector<T> * b_adj = new MmapVector<T>(b_adj_file);
  b_adj->open(false);

  size_t N = seq_size(db_dir, hash);

  MmapVector<uint64_t> * seq = new MmapVector<uint64_t>(seq_file);
  seq->open(false);
  T v = seq_get(seq, key);
  seq->close();
  delete seq;

  MmapVector<double> * prv = new MmapVector<double>(pagerank_file);
  prv->open(false);
  double pr = (* prv->at(v) );
  prv->close();
  delete prv;

  vector<double> c;

  approx_contribution(v, pr, b_adj, & c, N);
  vector<T> sdv = delta_significant_contributing_set(& c, v, pr, N, DELTA);

  double total_contribution = 0.0;
  for(typename vector<T>::iterator i = sdv.begin(); i < sdv.end(); i++)
  {
    total_contribution += c[(*i)];
  }
  double rpr = pr - total_contribution / N + DELTA * sdv.size();

  cout << "rpr = " << rpr << endl;

  b_adj->close();
  delete b_adj;

  return 1;
}

int robust_pagerank(const std::string& db_dir, const uint64_t& key)
{
  const string hash = "testhash";
  const string adj_file = db_dir + FILE_ADJLIST_FORWARD + hash;
  const string seq_file = db_dir + FILE_SEQUENCE        + hash;

  seq_create(db_dir, hash);

  if(seq_32bit(db_dir, hash))
  {
    return robust_pagerank_t<uint32_t>(db_dir, key);
  }else{
    return robust_pagerank_t<uint64_t>(db_dir, key);
  }
}

