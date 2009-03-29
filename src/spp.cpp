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

template<typename Heuristic> void push_path(ShortestpathTree<Heuristic> & spt, uint64_t start, uint64_t target, vector<uint64_t> & path)
{
  uint64_t current = target;
  while(true){
    path.push_back(current);
    if(current == start) break;
    current = spt.parent(current);
  }
}

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
  cout << "result:" << endl;
  cout << "  found: " << meet << endl;
  cout << "  iteration: " << i << endl;

  if(meet){
    vector<uint64_t> path;
    push_path(spt_fwd, start, target, path);
    reverse(path.begin(), path.end());
    cout << "  path:" << endl;
    for(vector<uint64_t>::iterator i = path.begin(); i != path.end(); ++i){
      cout << "    - " << (*i) << endl;
    } 
  }

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
  uint64_t meet_node = 0;
  while(! meet)
  { 
    i++;
    ShortestpathTree<Heuristic> * spt_a = (i % 2) ? &spt_fwd : &spt_rev;
    ShortestpathTree<Heuristic> * spt_b = (i % 2) ? &spt_rev : &spt_fwd;
 
    if(! spt_a->next(visit) ) break;

    meet = spt_b->scaned(visit.node);
    if(meet) meet_node = visit.node;
  }
  LOG(INFO) << "bidirectional_t finished" << endl;

  cout << "result: " << endl;
  cout << "  found: "  << meet << endl;
  cout << "  iteration: " << i << endl;

  if(meet){
    vector<uint64_t> path_fwd;
    vector<uint64_t> path_rev;
    push_path(spt_fwd, start,  meet_node, path_fwd);
    push_path(spt_rev, target, meet_node, path_rev);
    reverse(path_fwd.begin(), path_fwd.end());
    cout << "  path:" << endl;
    for(vector<uint64_t>::iterator i = path_fwd.begin(); i != path_fwd.end(); ++i){
      cout << "    - " << (*i) << endl;
    }
    for(vector<uint64_t>::iterator i = path_rev.begin(); i != path_rev.end(); ++i){
      if((*i) != meet_node){
        cout << "    - " << (*i) << endl;
      }
    }

  }

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

