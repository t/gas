#include <map>
#include <set>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <time.h>
#include <google/gflags.h>
#include <glog/logging.h>

#include "btree.h"
#include "edge.h"
#include "seq.h"
#include "db.h"

DEFINE_bool(forward, true, "db path");

using namespace std;
using namespace boost;
using namespace boost::filesystem;

bool operator < (const Edge &e, const Edge &f) {
  return e.key != f.key ? e.key < f.key : e.to < f.to;
}

int edge_init()
{
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  true);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_BACKWARD), true);

  delete f_btree;
  delete b_btree;

  LOG(INFO) << "edge inited";
}

int edge_select()
{
  LOG(INFO) << "edge_select is starting." << endl;

  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 4);
  
  uint64_t key = lexical_cast<uint64_t>(argvs[3]);

  Btree<uint64_t, Edge> * btree;
  if(FLAGS_forward)
  {
    btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  false);
  }else{
    btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_BACKWARD), false);
  }

  vector<Edge> result = btree->find(key);
  cout << "result: " << endl;
  cout << "  edges: " << endl;
  for(vector<Edge>::iterator i = result.begin(); i < result.end(); i++)
  {
    cout << "    - [" << (*i).key << ","  << (*i).to << "]" << endl;
  }
  cout << "  edge_count: "  << result.size() << endl;

  delete btree;

  LOG(INFO) << "edge_select is complete." << endl;
}

int edge_insert()
{
  LOG(INFO) << "edge_insert is starting." << endl;

  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_BACKWARD), false);

  string str, item;

  uint64_t c = 0;
  while(getline(cin, str))
  {
    c++;
    if(c % 10000 == 0) LOG(INFO) << "edge_insert is inserting: " << c << " times" << endl;

    istrstream istrs((char *)str.c_str());
    int i = 0;
    uint64_t f, t, w;
    while(istrs >> item)
    {
      istrstream istrs2((char *)item.c_str());
      if(i == 0) {
        istrs2 >> f;
      }else if(i == 1) {
        istrs2 >> t;
      }else if(i == 2) {
        istrs2 >> w;
      }else{
        break;
      }
      i++;
    }
    if(i == 2) {
      w = 1;
      i++; 
    }
    if(i == 3) {
      Edge fe;
      fe.key    = f;
      fe.to     = t;
      fe.length = w;

      Edge be;
      be.key    = t;
      be.to     = f;
      be.length = w;

      f_btree->insert(fe);
      b_btree->insert(be);
    }
  }

  delete f_btree;
  delete b_btree;

  cout << "result: " << endl;
  cout << "  edge_count: " << c << endl;

  LOG(INFO) << "edge_insert is complete." << endl;
}

int edge_random()
{
  srand(time(NULL));

  const string seq_file = db_path(FILE_SEQUENCE);
  MmapVector<uint64_t> * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  uint64_t node = seq_get_random(seq);
  cout << "rand: " << node << endl;

  seq->close();
  delete seq;
}

int edge()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 3);

  if(argvs[2] =="select"){
    return edge_select();
  }else if(argvs[2] == "insert"){
    return edge_insert();
  }

  return 0;
}

