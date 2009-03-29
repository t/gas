#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <google/gflags.h>
#include <glog/logging.h>

#include "btree.h"
#include "edge.h"
#include "spp.h"
#include "db.h"
#include "shortestpath_tree.h"
#include "dijkstra_heuristic.h"

DEFINE_string(directional, "bi", "directional");

using namespace std;
using namespace boost;
using namespace boost::filesystem;

template<typename Heuristic> int unidirectional_t(uint64_t start, uint64_t target, Heuristic * fwd_h)
{
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  false);
  ShortestpathTree<Heuristic> spt_fwd(f_btree, start, target, fwd_h);

  VItem<Heuristic> visit;
  bool meet = false;
  int i = 0;
  while( spt_fwd.next(visit) )
  {
    i++;
    if(visit.node == target) {
      meet = true;
      break;
    }
  }
  cout << "meet?= " << meet << " count = " << i << endl;

  delete f_btree;
  if(meet) return i;
  return 0;
}

template<typename Heuristic> int bidirectional_t(uint64_t start, uint64_t target, Heuristic * fwd_h, Heuristic * rev_h)
{
  LOG(INFO) << "bidirectional_t is starting." << endl;

  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD),  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_BACKWARD), false);
  ShortestpathTree<Heuristic> spt_fwd(f_btree, start, target, fwd_h);
  ShortestpathTree<Heuristic> spt_rev(b_btree, target, start, rev_h);

  LOG(INFO) << "bidirectional_t started." << endl;

  VItem<Heuristic> visit;
  bool meet = false;
  int i = 0;
  while(! meet)
  { 
    i++;
    ShortestpathTree<Heuristic> * spt_a = (i % 2) ? &spt_fwd : &spt_rev;
    ShortestpathTree<Heuristic> * spt_b = (i % 2) ? &spt_rev : &spt_fwd;
 
    if(! spt_a->next(visit) ) break;

    meet = spt_b->scaned(visit.node);
  }
  LOG(INFO) << "bidirectional_t finished" << endl;

  cout << "result: " << endl;
  cout << "  meet: "  << meet << endl;
  cout << "  search_space: " << i << endl;

  delete f_btree;
  delete b_btree;

  if(meet) return i;
  return 0;
}

int unidirectional_dijkstra(uint64_t start, uint64_t target)
{
  DijkstraHeuristic h;
  return unidirectional_t<DijkstraHeuristic>(start, target, &h);
}

int bidirectional_dijkstra(uint64_t start, uint64_t target)
{
  DijkstraHeuristic h;
  return bidirectional_t<DijkstraHeuristic>(start, target, &h, &h);
}

int spp()
{
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 4);

  uint64_t source = lexical_cast<uint64_t>(argvs[2]);
  uint64_t target = lexical_cast<uint64_t>(argvs[3]);
 
  if(FLAGS_directional == "bi"){
    return bidirectional_dijkstra(source, target);
  }else if(FLAGS_directional == "uni"){
    return unidirectional_dijkstra(source, target);
  }

  return 0;
}

