#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include "btree.h"
#include "edge.h"
#include "spp.h"
#include "shortestpath_tree.h"
#include "dijkstra_heuristic.h"
#include "alt_heuristic.h"
#include "alt_precompute.h"
#include "malt_heuristic.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

template<typename Heuristic> int unidirectional_t(const std::string& db_dir, uint64_t start, uint64_t target, Heuristic * fwd_h)
{
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  ShortestpathTree<Heuristic> spt_fwd(f_btree, start, target, fwd_h);

  VItem<Heuristic> visit;
  bool meet = false;
  int i = 0;
  while( spt_fwd.next(visit, true) )
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

template<typename Heuristic> int bidirectional_t(const std::string& db_dir, uint64_t start, uint64_t target, Heuristic * fwd_h, Heuristic * rev_h)
{
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);
  ShortestpathTree<Heuristic> spt_fwd(f_btree, start, target, fwd_h);
  ShortestpathTree<Heuristic> spt_rev(b_btree, target, start, rev_h);

  VItem<Heuristic> visit;
  bool meet = false;
  int i = 0;
  while(! meet)
  { 
    i++;
    ShortestpathTree<Heuristic> * spt_a = (i % 2) ? &spt_fwd : &spt_rev;
    ShortestpathTree<Heuristic> * spt_b = (i % 2) ? &spt_rev : &spt_fwd;
 
    if(! spt_a->next(visit, true) ) break;

    meet = spt_b->scaned(visit.node);
  }
  cout << "meet?= " << meet << " count = " << i << endl;

  delete f_btree;
  delete b_btree;
  if(meet) return i;
  return 0;
}

int unidirectional_dijkstra(const std::string& db_dir, uint64_t start, uint64_t target)
{
  DijkstraHeuristic h;
  return unidirectional_t<DijkstraHeuristic>(db_dir, start, target, &h);
}

int bidirectional_dijkstra(const std::string& db_dir, uint64_t start, uint64_t target)
{
  DijkstraHeuristic h;
  return bidirectional_t<DijkstraHeuristic>(db_dir, start, target, &h, &h);
}

int bidirectional_alt(const std::string& db_dir, uint64_t start, uint64_t target, 
                      const std::string& landmark_strategy, const int& landmark_size)
{
  const string pre_file = db_dir + "tmp_alt_" + landmark_strategy + "_" + lexical_cast<string>(landmark_size);

  if(! exists(pre_file) ){
    alt_precompute(db_dir, landmark_strategy, landmark_size);
  }

  Btree<uint64_t, LandmarkDist> * alt_btree = new Btree<uint64_t, LandmarkDist>(pre_file, false);

  AltHeuristic fwd_h(true,  alt_btree);
  AltHeuristic rev_h(false, alt_btree);

  int result = bidirectional_t<AltHeuristic>(db_dir, start, target, &fwd_h, &rev_h);

  delete alt_btree;
  return result;
}

int bidirectional_malt(const std::string& db_dir, uint64_t start, uint64_t target, const std::string& strategy, const int& landmark_size)
{
  const string pre2_file = db_dir + "tmp_malt_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string pre3_file = db_dir + "tmp_malt_landmark_" + strategy + "_" + lexical_cast<string>(landmark_size);
  Btree<uint64_t, LandmarkDist> * malt_btree     = new Btree<uint64_t, LandmarkDist>(pre2_file, false);
  Btree<uint64_t, LandmarkDist> * landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, false);

  MaltHeuristic fwd_h(true,  true, malt_btree, landmark_btree);
  MaltHeuristic rev_h(false, true, malt_btree, landmark_btree);

  int result = bidirectional_t<MaltHeuristic>(db_dir, start, target, &fwd_h, &rev_h);

  delete malt_btree;
  delete landmark_btree;
  return result;
}

int unidirectional_malt(const std::string& db_dir, uint64_t start, uint64_t target, const std::string& strategy, const int& landmark_size)
{
  const string pre2_file = db_dir + "tmp_malt_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string pre3_file = db_dir + "tmp_malt_landmark_" + strategy + "_" + lexical_cast<string>(landmark_size);
  Btree<uint64_t, LandmarkDist> * malt_btree     = new Btree<uint64_t, LandmarkDist>(pre2_file, false);
  Btree<uint64_t, LandmarkDist> * landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, false);

  MaltHeuristic fwd_h(true, false,  malt_btree, landmark_btree);

  int result = unidirectional_t<MaltHeuristic>(db_dir, start, target, &fwd_h);

  delete malt_btree;
  delete landmark_btree;
  return result;
}



