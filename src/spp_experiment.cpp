#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include "btree.h"
#include "edge.h"
#include "spp.h"
#include "dijkstra_heuristic.h"
#include "shortestpath_tree.h"
#include "seq.h"
#include "spp_experiment.h"

using namespace std;
using namespace boost;

pair<uint64_t, uint64_t> farthest_pair(const std::string& db_dir)
{
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);
  const string seq_file = db_dir + FILE_SEQUENCE + "testhash";
  MmapVector<uint64_t> * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  uint64_t source = seq_get_random(seq);
  uint64_t target = seq_get_random(seq);

  /*
  DijkstraHeuristic h;
  VItem<DijkstraHeuristic> visit;

  source = random;

  ShortestpathTree<DijkstraHeuristic> spt_f(f_btree, source, 0, &h);
  while( spt_f.next(visit) )
  {
    target = visit.node;
  }*/

  seq->close();
  delete seq;
  delete f_btree;
  delete b_btree;

  return make_pair(source, target);
}

int spp_experiment(const std::string& db_dir, const std::string& func, const std::string& strategy, const int& landmark_size)
{
  srand(200);

  int total = 0;
  int c = 0;
  int max = 0;
  for(int i = 0; i < 100; i++)
  {
    pair<uint64_t, uint64_t> testset = farthest_pair(db_dir);
    uint64_t source = testset.first;
    uint64_t target = testset.second;
    cout << "test source = " << source << " target = " << target << endl;
   
    int result = 0;

    if(func == "alt"){
      result = bidirectional_alt(db_dir, source, target, strategy, landmark_size);
    }else if(func == "malt"){
      result = bidirectional_malt(db_dir, source, target, strategy, landmark_size);
    }else if(func == "dijkstra"){
      result = bidirectional_dijkstra(db_dir, source, target);
    }

    if(result > 0)
    {
      total += result;
      c++;
      if(result > max) max = result;
    } 
  }
  cout << "avg = " << total / c << endl;
  cout << "max = " << max << endl;
}



