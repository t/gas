#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/dynamic_bitset.hpp>
#include "btree.h"
#include "edge.h"
#include "seq.h"
#include "shortestpath_tree.h"
#include "landmark_select.h"
#include "alt_heuristic.h"

using namespace std;
using namespace boost;

int alt_precompute_maxcover(const std::string& db_dir, const std::string& strategy, const int& landmark_size)
{
  cout << "alt precompute maxcover start" << endl;

  seq_create(db_dir, "testhash");

  const string pre_file = db_dir + "tmp_alt_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string seq_file = db_dir + FILE_SEQUENCE + "testhash";

  MmapVector<uint64_t>  * seq     = new MmapVector< uint64_t >(seq_file);
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD, false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);
  Btree<uint64_t, LandmarkDist> * alt_btree = new Btree<uint64_t, LandmarkDist>(pre_file, true);

  seq->open(false);

  size_t m_len =  btree_count<AltHeuristic>(f_btree);

  vector<uint64_t> candidates;
  vector<uint64_t> landmarks;
  size_t n = seq->size();
  uint64_t landmark = seq_get_random(seq);

  int i = 0;
  while( (i < 5 * landmark_size) && (candidates.size() < 4 * landmark_size) )
  {
    cout << "candidates.size = " << candidates.size() << "  i = "<< i << endl;
    while(landmarks.size() < landmark_size)
    {
      landmarks.push_back(landmark);
      if(find(candidates.begin(), candidates.end(), landmark) == candidates.end()) 
      {
        candidates.push_back(landmark);
        uint64_t saved = 0;
        landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, landmark, saved);
      }
      AltHeuristic fwd_h(true, alt_btree);
      AltHeuristic rev_h(false, alt_btree);
      landmark = landmark_select_avoid<AltHeuristic>(seq, f_btree, landmarks, &fwd_h);
      i++;
    }

    for(int j = 0; j < landmark_size / 2; j++)
    {
      int delete_idx = rand() % landmarks.size();
      landmarks.erase( landmarks.begin() + delete_idx );
    } 
  }

  landmarks.clear();
  for(int i = 0; i < landmark_size ; i++)
  {
    int idx = rand() % candidates.size();
    if(find(landmarks.begin(), landmarks.end(), candidates[idx]) == landmarks.end() )
    {
      landmarks.push_back( candidates[idx] );
    }
  }

  cout << "alt_coverd_arcs start " << endl;

  cout << "size of m  = " << m_len << endl;

  map<uint64_t, dynamic_bitset<> > res =
      alt_coverd_arcs<AltHeuristic>(db_dir, f_btree, alt_btree, candidates, m_len);

  cout << "res size = " <<res.size() << endl;

 cout << "alt_coverd_arcs end" << endl;

  int current_covered = calc_covered_arc<AltHeuristic>(res, landmarks, m_len);

  cout << "hoge" << endl;

  map< pair<uint64_t, uint64_t>, bool > swaped;
  map<string, bool> calced;
  int swap_count = 1;
  while(swap_count > 0)
  {
    swap_count = 0;
    for(vector<uint64_t>::iterator i = landmarks.begin(); i < landmarks.end(); i++)
    {
      dynamic_bitset<> m(m_len);
      for(vector<uint64_t>::iterator j = landmarks.begin(); j < landmarks.end(); j++)
      {
        if( j != i)
        {
          merge_covered_arc<AltHeuristic>(res[(*j)], m);
        }
      }

      for(vector<uint64_t>::iterator j = candidates.begin(); j < candidates.end(); j++)
      {
        uint64_t a = (*i) < (*j) ? (*i) : (*j); 
        uint64_t b = (*i) < (*j) ? (*j) : (*i);

        if(a == b) continue;
        //if(swaped[make_pair(a, b)]) continue;
        swaped[make_pair(a, b)] = true;       

        swap( (*i), (*j) );

        string key = "";
        vector<uint64_t> sorted = landmarks;
        sort(sorted.begin(), sorted.end()); 
        for(vector<uint64_t>::iterator k = sorted.begin(); k < sorted.end(); k++)
        {
          key += lexical_cast<string>( (*k) ) + ",";
        } 

        if(calced[key]){
          cout << "skip calced"<< endl;
          swap( (*i), (*j) );
          continue;
        }
        calced[key] = true;

        cout << "swap" << endl;
        swap_count++;
 
        dynamic_bitset<> m_test = m;
        merge_covered_arc<AltHeuristic>(res[(*i)], m_test);
        int new_covered = m_test.count();
        if(new_covered > current_covered)
        {
          current_covered = new_covered;
          break;
        }else{
          swap( (*i), (*j) );
        }

        cout << "current covered = " << current_covered << endl;
      }
    }
  }

  delete alt_btree;


  cout << "start set landmarks " << endl;

  alt_btree = new Btree<uint64_t, LandmarkDist>(pre_file, true);

  for(vector<uint64_t>::iterator i = landmarks.begin(); i < landmarks.end(); i++)
  {
    uint64_t saved = 0;
    cout << "landmark = " << (*i) << endl;
    landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, (*i), saved);
  }
 
  seq->close();
  delete seq;
  delete f_btree;
  delete b_btree;
  delete alt_btree;

  cout << "alt precompute maxcover end" << endl;

  return 1;
}

int alt_precompute(const std::string& db_dir, const std::string& strategy, const int& landmark_size)
{
  srand(100);

  if(strategy == "maxcover")
  {
    return alt_precompute_maxcover(db_dir, strategy, landmark_size);
  }

  cout << "alt precompute start" << endl;

  seq_create(db_dir, "testhash");

  const string pre_file = db_dir + "tmp_alt_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string seq_file = db_dir + FILE_SEQUENCE + "testhash";

  MmapVector<uint64_t>  * seq     = new MmapVector< uint64_t >(seq_file);
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD, false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);
  Btree<uint64_t, LandmarkDist> * alt_btree = new Btree<uint64_t, LandmarkDist>(pre_file, true);

  seq->open(false);

  map<uint64_t, double> total_dist;

  size_t n = seq->size();
  uint64_t landmark = seq_get_random(seq);
  vector<uint64_t> landmarks;
  uint64_t total_saved = 0;
  for(int i = 0; i < landmark_size; i++)
  {
    uint64_t saved = 0;
    cout << "landmark = " << landmark << endl;
    landmarks.push_back(landmark);
    if(strategy == "random"){
      cout << "random add" << endl;
      landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, landmark, saved);
      landmark = seq_get_random(seq);
    }else if(strategy == "fard") {
      cout << " d = " << total_dist[landmark] << endl;
      landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, landmark, saved);
      landmark = landmark_select_far<AltHeuristic>(seq, b_btree, 1, landmark, landmarks, & total_dist);
    }else if(strategy == "farb") {
      cout << " d = " << total_dist[landmark] << endl;
      landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, landmark, saved);
      landmark = landmark_select_far<AltHeuristic>(seq, b_btree, 2, landmark, landmarks, & total_dist);
    }else if(strategy == "planner") {
      //landmark = landmark_add_planner( seq, f_btree, p_btree);
    }else if(strategy == "avoid") {
      landmark_add<AltHeuristic>(seq, true, f_btree, alt_btree, landmark, saved);
      AltHeuristic fwd_h(true, alt_btree);
      AltHeuristic rev_h(false, alt_btree);
      landmark = landmark_select_avoid<AltHeuristic>(seq, f_btree, landmarks, &fwd_h);
    }else if(strategy == "maxcover") {
      //landmark = landmark_add_maxcover(seq, f_btree, p_btree);
    }
    total_saved += saved;
  }

  seq->close();
  delete seq; 
  delete f_btree;
  delete b_btree;
  delete alt_btree;

  cout << "alt precompute end" << endl;
  cout << "saved = " << total_saved << endl;

  return 1;
}

