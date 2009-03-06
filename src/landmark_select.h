#ifndef LANDMARK_SELECT_H_ 
#define LANDMARK_SELECT_H_

#include <map>
#include <set>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/dynamic_bitset.hpp>

#include "btree.h"
#include "edge.h"
#include "shortestpath_tree.h"
#include "alt_heuristic.h"

using namespace std;

class DijkstraHeuristic
{
public:
  double operator()(uint source, uint target, uint current){
    return 0;
  }
};

/*
  strategy : 0-nothing 1- ferd 2 - farb
*/
template<typename Heuristic>
uint64_t landmark_add(
          MmapVector<uint64_t>          * seq,
          bool                            is_forward, 
          Btree<uint64_t, Edge>         * f_btree, 
          Btree<uint64_t, LandmarkDist> * p_btree, 
          const uint64_t&                 landmark, 
          uint64_t&                       saved
  )
{
  double   max_dist = 0;
  uint64_t max_node = 0;

  DijkstraHeuristic h;
  ShortestpathTree<DijkstraHeuristic> spt(f_btree, landmark, 0, &h);

  VItem<DijkstraHeuristic> visit;
  saved = 0;
  while( spt.next(visit) )
  { 
    LandmarkDist e;
    e.key      = visit.node;
    e.landmark = landmark;
    e.forward  = is_forward; 
    e.length   = (double) visit.dist;
    p_btree->insert(e);
    saved++;
  }

  std::cout << "saved: " << saved << std::endl;
  return max_node;
}

template<typename Heuristic>
uint64_t landmark_select_far(
          MmapVector<uint64_t>  * seq,
          Btree<uint64_t, Edge> * f_btree,
          const int&              strategy,
          const uint64_t&         landmark,
          const std::vector<uint64_t>& landmarks,
          std::map<uint64_t, double> * total_dist
  )
{
  double   max_dist = 0;
  uint64_t max_node = 0;

  DijkstraHeuristic h;
  ShortestpathTree<DijkstraHeuristic> spt(f_btree, landmark, 0, &h);

  VItem<DijkstraHeuristic> visit;
  while( spt.next(visit) )
  {
    double current_d;
    if(total_dist->find(visit.node) == total_dist->end())
    {
      current_d = UINT_MAX;
    }else{
      current_d = (* total_dist)[visit.node] + 0;
    }
    double new_d = 0;
    if(strategy == 1){
      new_d = visit.dist;
    }else if(strategy == 2){
      new_d = visit.hop;
    }
    if(new_d < current_d){
      (* total_dist)[visit.node] = new_d;
    }
  }

  for(std::map<uint64_t, double>::iterator i = total_dist->begin(); i != total_dist->end(); i++)
  {
    if(i->second > max_dist && find(landmarks.begin(), landmarks.end(), i->first) == landmarks.end() )
    {
      max_dist = i->second;
      max_node = i->first;
    }
  }

  cout << "landmark_select_far max_dist = " << max_dist << " max_node = " << max_node << endl;
  return max_node;
}


/*
vector<uint64_t> landmark_select_planner(MmapVector< uint64_t > * seq, Btree<uint64_t, Edge> * btree, int size)
{
}
*/

template<typename Heuristic>
double calc_reduce_cost(Btree<uint64_t, Edge> * btree, Heuristic * h, const std::string& db_dir)
{
  double cost = 0;

  Btree<uint64_t, Edge> * tree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);

  ForwardWalker<uint64_t, Edge> walker = btree->walkerBegin();
  while(! walker.is_end)
  {
    Edge value = walker.value();
    cost += (double)value.length - h->h_star(value.key, value.to);
    walker.increment();
  }

  delete tree;

  return cost;
}

template<typename Heuristic>
size_t btree_count(Btree<uint64_t, Edge> * btree)
{
  size_t count = 0; 

  ForwardWalker<uint64_t, Edge> walker = btree->walkerBegin();
  while(! walker.is_end)
  {
    count++; 
    walker.increment();
  }
  return count;
}

template<typename Heuristic>
int calc_covered_arc( map<uint64_t, dynamic_bitset<> >& input,  vector<uint64_t>& landmarks, size_t m_len)
{
  int result = 0;

  dynamic_bitset<> m(m_len);

  for(vector<uint64_t>::iterator i = landmarks.begin(); i < landmarks.end(); i++)
  {
    m |= input[(*i)];
  }

  return m.count();
}

template<typename Heuristic>
void merge_covered_arc( dynamic_bitset<> & input, dynamic_bitset<> & output)
{
  output |= input;
}

template<typename Heuristic>
void  alt_covered_arc_landmarks(Btree<uint64_t, LandmarkDist> * alt_btree, uint64_t s, uint64_t t, double length, vector<uint64_t>& result)
{
    result.clear();

    vector<LandmarkDist> source_dists = alt_btree->find(s);
    vector<LandmarkDist> target_dists = alt_btree->find(t);
    vector< map<uint64_t, vector<double> > > bi(2);

    for(vector<LandmarkDist>::iterator i = source_dists.begin(); i != source_dists.end(); i++)
    {
      if(i->forward)
      {
        bi[0][i->landmark].push_back(  i->length);
      }else{
        bi[1][i->landmark].push_back(- i->length);
      }
    }

    for(vector<LandmarkDist>::iterator i = target_dists.begin(); i != target_dists.end(); i++)
    {
      if(i->forward)
      {
        bi[0][i->landmark].push_back(  i->length);
      }else{
        bi[1][i->landmark].push_back(- i->length);
      }
    }

    for(vector< map<uint64_t, vector<double> > >::iterator i = bi.begin(); i != bi.end(); i++)
    {
      for(map<uint64_t, vector<double> >::iterator j = (*i).begin(); j != (*i).end(); j++)
      {
        uint64_t landmark = (*j).first;

        if( (*j).second.size() != 2) {
          //cout << "ignore - " << (*j).second.size() << endl;
          continue;
        }

        double d = (*j).second[1] - (*j).second[0];
        if(d >= length - 0.1)
        {
          result.push_back(landmark); 
        }
      }
    }

}

template<typename Heuristic>
void malt_covered_arc_landmarks(Btree<uint64_t, LandmarkDist> * malt_btree, Btree<uint64_t, LandmarkDist> * malt_landmark_btree, uint64_t s, uint64_t t, double length, vector< pair<uint64_t,uint64_t> >& result)
{
    result.clear();

    double max = 0;

    vector<LandmarkDist> source_dists = malt_btree->find(s);
    vector<LandmarkDist> target_dists = malt_btree->find(t);

    vector< map<uint64_t, vector<double> > > bi(2);

    vector< pair<uint64_t, double> > available_fwd;
    vector< pair<uint64_t, double> > available_rev;
    for(vector<LandmarkDist>::iterator i = source_dists.begin(); i != source_dists.end(); i++)
    {
      if(i->forward)
      {
         available_fwd.push_back( make_pair( i->landmark, i->length ) );
      }
    }

    for(vector<LandmarkDist>::iterator i = target_dists.begin(); i != target_dists.end(); i++)
    {
      if(! i->forward)
      {
         available_rev.push_back( make_pair( i->landmark, i->length ) );
      }
    }
    //cout << "available: fwd = " << available_fwd.size() <<   "  rev = " << available_rev.size() << endl;

    for(vector< pair<uint64_t, double> >::iterator i = available_fwd.begin(); i < available_fwd.end(); i++)
    {
       double org_p = i->second;

       vector<LandmarkDist> landmark_dists = malt_landmark_btree->find(i->first);
       map<uint64_t, double> landmark_dists_map;
       for(vector<LandmarkDist>::iterator j = landmark_dists.begin(); j < landmark_dists.end(); j++)
       {
         landmark_dists_map[j->landmark] = j->length;
       }

       for(vector< pair<uint64_t, double> >::iterator j = available_rev.begin(); j < available_rev.end(); j++)
       {
         double rev_p = j->second;

         double landmark_d = landmark_dists_map[j->first];

         double d1 = landmark_d - org_p;
         if(d1 <= 0) continue;

         double d2 = d1 - rev_p;
         if(d2 >= length - 0.1)
         {
           result.push_back( make_pair(i->first, j->first) );
         }
      }
    }
}

template<typename Heuristic>
map<uint64_t, dynamic_bitset<> > alt_coverd_arcs(const std::string& db_dir, Btree<uint64_t, Edge> * tree,  Btree<uint64_t, LandmarkDist> * alt_btree, vector<uint64_t>& landmarks, size_t m)
{
  cout << "start alt_coverd_arcs " << endl;


  map<uint64_t, dynamic_bitset<> > ret;
  for(vector<uint64_t>::iterator i = landmarks.begin(); i < landmarks.end(); i++)
  {
    ret[(*i)] = dynamic_bitset<>(m);
  }

  //Btree<uint64_t, Edge> * tree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);

  ForwardWalker<uint64_t, Edge> walker = tree->walkerBegin();
  size_t idx = 0;
  cout << "loop start" << endl;
  while(! walker.is_end)
  {
    Edge value = walker.value();
    
    vector<uint64_t> res;
    alt_covered_arc_landmarks<Heuristic>(alt_btree, value.key, value.to, value.length, res);
    for(vector<uint64_t>::iterator i = res.begin(); i < res.end(); i++)
    {
      if( ret[(*i)].size() == 0)
      {
        cout << "not set " << (*i) << endl;
        ret[(*i)] = dynamic_bitset<>(m);
      }
      //cout << ret[(*i)].size() << endl;
      ret[(*i)][idx] = 1;
    } 
    walker.increment();
    idx++;
  }
  //delete tree;

  cout << "end alt_coverd_arcs" << endl;

  return ret;
}

template<typename Heuristic>
void malt_coverd_arcs(const std::string& db_dir, Btree<uint64_t, Edge> * tree,  Btree<uint64_t, LandmarkDist> * alt_btree, Btree<uint64_t, LandmarkDist> * malt_landmark_btree, vector<uint64_t>& landmarks_fwd, vector<uint64_t>& landmarks_rev,  size_t m, map< pair<uint64_t, uint64_t> , dynamic_bitset<> >& ret)
{
  cout << "start malt_coverd_arcs " << endl;

  for(vector<uint64_t>::iterator i = landmarks_fwd.begin(); i < landmarks_fwd.end(); i++)
  {
    for(vector<uint64_t>::iterator j = landmarks_rev.begin(); j < landmarks_rev.end(); j++)
    {
      ret[make_pair((*i), (*j)) ] = dynamic_bitset<>(m);
    }
  }

  //Btree<uint64_t, Edge> * tree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);

  ForwardWalker<uint64_t, Edge> walker = tree->walkerBegin();
  size_t idx = 0;
  cout << "loop start" << endl;
  while(! walker.is_end)
  {
    Edge value = walker.value();

    vector< pair<uint64_t, uint64_t> > res;
    malt_covered_arc_landmarks<Heuristic>(alt_btree, malt_landmark_btree, value.key, value.to, value.length, res);
    for(vector< pair<uint64_t,uint64_t>  >::iterator i = res.begin(); i < res.end(); i++)
    {
      if( ret[(*i)].size() == 0)
      {
        cout << "not set " << endl;
        ret[(*i)] = dynamic_bitset<>(m);
      }
      //cout << ret[(*i)].size() << endl;
      ret[(*i)][idx] = 1;
    }
    walker.increment();
    idx++;
  }
  //delete tree;

  cout << "end malt_coverd_arcs" << endl;
}

template<typename Heuristic>  
bool pickup_avoid_maxnode(
           const std::multimap<uint64_t, VItem<DijkstraHeuristic> >& children, 
           const uint64_t source,
           const VItem<DijkstraHeuristic>& v, 
           const std::vector<uint64_t>& landmarks, 
           Heuristic * h,
           pair<uint64_t, double> * w
           )
{
  pair<uint64_t, double> ret;

  bool include_landmark = (find(landmarks.begin(), landmarks.end(), v.node) != landmarks.end() );

  pair< std::multimap<uint64_t, VItem<DijkstraHeuristic> >::const_iterator, 
        std::multimap<uint64_t, VItem<DijkstraHeuristic> >::const_iterator
      > range = children.equal_range(v.node);

  pair<uint64_t, double> ret_w = make_pair(0, 0);
  if(range.first == range.second)
  { 
    if(include_landmark){
      ret_w = make_pair(v.node, 0);
    }else{
      ret_w = make_pair(v.node, v.dist - h->h_star(source, v.node));
    }
  }else{
    std::vector< pair<uint64_t, double> > children_maxs;
    for( std::multimap<uint64_t, VItem<DijkstraHeuristic> >::const_iterator i = range.first; i != range.second; i++)
    {
      pair<uint64_t, double> children_max;
      bool children_include_landmark = pickup_avoid_maxnode<Heuristic>(children, source, i->second, landmarks, h, & children_max);
      if(children_include_landmark) include_landmark = true;
      children_maxs.push_back(children_max);
    }
 
    pair<uint64_t, double> max_w = make_pair(0, 0);
    for(std::vector< pair<uint64_t, double> >::iterator i = children_maxs.begin(); i != children_maxs.end(); i++)
    {
      if((*i).second > max_w.second) max_w = (*i); 
      if(! include_landmark ) ret_w.second += (*i).second;
    }   

    if(include_landmark)
    {
      ret_w = max_w;
    }else{
      ret_w.first   = max_w.first;
      ret_w.second += v.dist - h->h_star(source, v.node);
    }
  } 

  (*w) = ret_w; 
  return include_landmark;
}

template<typename Heuristic> 
uint64_t landmark_select_avoid(
      MmapVector<uint64_t>  * seq,
      Btree<uint64_t, Edge> * btree, 
      const std::vector<uint64_t>& landmarks, 
      Heuristic * h
)
{
  uint64_t source = seq_get_random(seq);
  DijkstraHeuristic dh;
  ShortestpathTree<DijkstraHeuristic> spt(btree, source, 0, &dh);

  multimap<uint64_t, VItem<DijkstraHeuristic> > children;
  VItem<DijkstraHeuristic> visit;
  VItem<DijkstraHeuristic> root;
  spt.next(root);
  while( spt.next(visit) )
  {
    children.insert(make_pair(visit.parent, visit));
  }
  pair<uint64_t, double> max_result;
  pickup_avoid_maxnode<Heuristic>(children, source, root, landmarks, h, &max_result);
  cout << "max_weight = " << max_result.second << "  source = " << source <<  endl;
  return max_result.first;
}

template<typename Heuristic>
uint64_t landmark_select_maxvisit(
                           MmapVector<uint64_t> * seq, 
                           Btree<uint64_t, Edge> * f_btree, 
                           Btree<uint64_t, Edge> * b_btree,
                           const std::vector<uint64_t>& landmarks, 
                           Heuristic * fwd_h, 
                           Heuristic * rev_h
                       )
{

  map<uint64_t, int> visit_count;
  for(int i = 0; i < 10; i++)
  {  
    uint64_t s = seq_get_random(seq);
    uint64_t t = seq_get_random(seq);

    ShortestpathTree<Heuristic> spt_fwd(f_btree, s, t, fwd_h);
    ShortestpathTree<Heuristic> spt_rev(b_btree, t, s, rev_h);

    VItem<Heuristic> visit;
    bool meet = false;
    int j = 0;
    while(! meet)
    {
      j++;
      ShortestpathTree<Heuristic> * spt_a = (j % 2) ? &spt_fwd : &spt_rev;
      ShortestpathTree<Heuristic> * spt_b = (j % 2) ? &spt_rev : &spt_fwd;

      if(! spt_a->next(visit) ) break;
      meet = spt_b->scaned(visit.node);
      visit_count[visit.node]++;
    }
  }

  pair<uint64_t, int> max_node = make_pair(0, 0);
  for(map<uint64_t, int>::iterator k = visit_count.begin(); k != visit_count.end(); k++)
  {
    if(k->second > max_node.second){
      if(find(landmarks.begin(), landmarks.end(), k->first) == landmarks.end()){
        max_node = (*k);
      }
    }
  }

  cout << "landmark_select_maxvisit visit = "  << max_node.second << " node = " << max_node.first << endl;

  return max_node.first;
}


#endif
