#include <map>
#include <set>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include "btree.h"
#include "edge.h"
#include "seq.h"
#include "shortestpath_tree.h"
#include "landmark_select.h"
#include "malt_heuristic.h"

using namespace std;
using namespace boost;

int malt_save_need_landmarks(
                  uint64_t node,
                  vector<uint64_t> & landmarks_fwd,
                  vector<uint64_t> & landmarks_rev,
                  map< pair<uint64_t, uint64_t>, double>& landmark_dists,
                  vector<LandmarkDist>& dists,
                  Btree<uint64_t, LandmarkDist> * malt_btree,
                  double& max_l
                )
{
   int ret = 0;

   map<uint64_t, pair<uint64_t, double> > max_potentials_fwd;
   map<uint64_t, pair<uint64_t, double> > max_potentials_rev;
   for(vector<LandmarkDist>::const_iterator i = dists.begin(); i < dists.end(); i++)
   {
     map<uint64_t, pair<uint64_t, double> > * max_potentials;
     vector<uint64_t>      * landmarks;
     if(i->forward)
     {
       max_potentials = & max_potentials_fwd;
       landmarks      = & landmarks_rev;
     }else{
       max_potentials = & max_potentials_rev;
       landmarks      = & landmarks_fwd;
     } 

     if(max_l - i->length < 0) continue;

     for(vector<uint64_t>::const_iterator j = landmarks->begin(); j != landmarks->end(); j++)
     {
       double l;
       if(i->forward)
       {
         l = landmark_dists[ make_pair(i->landmark, (*j))];
       }else{
         l = landmark_dists[ make_pair((*j), i->landmark)];
       }

       double d = l - i->length;
       if(d < 0) continue;

       pair<uint64_t, double> max_p = (* max_potentials)[(*j)];
       if( ( d > max_p.second) || 
          ((d == max_p.second) &&  (i->landmark < max_p.first)  )
          )
       {
         (* max_potentials)[(*j)] = make_pair(i->landmark, d);
       }
     }
   }

   set< pair<bool, uint64_t> > need_landmarks;

   for(map<uint64_t, pair<uint64_t, double> >::const_iterator i = max_potentials_fwd.begin(); i != max_potentials_fwd.end(); i++)
   {
     need_landmarks.insert( make_pair(true, i->second.first) );
   } 
   for(map<uint64_t, pair<uint64_t, double> >::const_iterator i = max_potentials_rev.begin(); i != max_potentials_rev.end(); i++)
   {
     need_landmarks.insert( make_pair(false, i->second.first) );
   }

   for(vector<LandmarkDist>::const_iterator i = dists.begin(); i < dists.end(); i++)
   {
     if(need_landmarks.find( make_pair( i->forward, i->landmark) ) != need_landmarks.end())
     {
       ret++;
       malt_btree->insert((*i));
     }
   }

   return ret;
}

void malt_strip(
            MmapVector<uint64_t>          * seq,
            Btree<uint64_t, LandmarkDist> * alt_btree, 
            Btree<uint64_t, LandmarkDist> * strip_btree,
            vector<uint64_t> & landmarks_fwd,
            vector<uint64_t> & landmarks_rev,
            map< pair<uint64_t, uint64_t>, double>& landmark_dists
)
{
  cout << "start malt strip" << endl; 

  double max_l = 0;
  for(map< pair<uint64_t, uint64_t>, double>::iterator i = landmark_dists.begin(); i != landmark_dists.end(); i++) 
  {
    if( (*i).second > max_l ) max_l = (*i).second;
  }
  cout << "max_ l = " << max_l << endl;

  int save_count = 0;
  for(size_t i = 0; i < seq->size(); i++)
  {
    if(i % 10000 == 0) cout << i << endl;

    uint64_t node = (* seq->at(i) );

    vector<LandmarkDist> dists = alt_btree->find(node);

    save_count += malt_save_need_landmarks(
              node, landmarks_fwd, landmarks_rev, landmark_dists, 
              dists, strip_btree, max_l
             );
  }

  cout << "strip btree size = " << save_count << endl;
  cout << "end malt strip" << endl;
}

Btree<uint64_t, LandmarkDist> * make_landmark_dist(
        const string& pre3_file, Btree<uint64_t, LandmarkDist> * alt_btree,
        vector<uint64_t>& landmarks_fwd, vector<uint64_t>& landmarks_rev)
{
    Btree<uint64_t, LandmarkDist> * landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, true);
    for(vector<uint64_t>::iterator i = landmarks_fwd.begin(); i < landmarks_fwd.end(); i++)
    {
      vector<LandmarkDist> ld = alt_btree->find( (*i) );
      for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
      {
         if(! j->forward)
         {
            LandmarkDist ld;
            ld.forward  = true;
            ld.key      = (*i);
            ld.landmark = j->landmark;
            ld.length   = j->length;
            landmark_btree->insert(ld);
         }
       }
     }

     for(vector<uint64_t>::iterator i = landmarks_rev.begin(); i < landmarks_rev.end(); i++)
     {
       vector<LandmarkDist> ld = alt_btree->find( (*i) );
       for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
       {
         if(j->forward)
         {
            LandmarkDist ld;
            ld.forward  = true;
            ld.key      = j->landmark;
            ld.landmark = (*i);
            ld.length   = j->length;
            landmark_btree->insert(ld);
         }
       }
    }
    
    return landmark_btree;
}

int malt_precompute(const std::string& db_dir, const std::string& strategy, const int& landmark_size)
{
  srand(100);

  if(strategy == "maxcover")
  {
    //return malt_precompute_maxcover(db_dir, strategy, landmark_size);
    return 0;
  }

  cout << "malt precompute start" << endl;

  seq_create(db_dir, "testhash");

  const string pre1_file = db_dir + "tmp_malt_tmp_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string pre2_file = db_dir + "tmp_malt_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string pre3_file = db_dir + "tmp_malt_landmark_" + strategy + "_" + lexical_cast<string>(landmark_size);
  const string seq_file = db_dir + FILE_SEQUENCE + "testhash";

  MmapVector<uint64_t>  * seq     = new MmapVector< uint64_t >(seq_file);
  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);
  Btree<uint64_t, LandmarkDist> * alt_btree   = new Btree<uint64_t, LandmarkDist>(pre1_file, true);
  Btree<uint64_t, LandmarkDist> * strip_btree = new Btree<uint64_t, LandmarkDist>(pre2_file, true);
  Btree<uint64_t, LandmarkDist> * landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, true);

  seq->open(false);

  map<uint64_t, double> total_dist_fwd;
  map<uint64_t, double> total_dist_rev;
  size_t n = seq->size();

  vector<uint64_t> landmarks_fwd;
  vector<uint64_t> landmarks_rev;
  uint64_t landmark_fwd = seq_get_random(seq) ;
  uint64_t landmark_rev = seq_get_random(seq) ;

  for(int i = 0; i < landmark_size; i++)
  {
    Btree<uint64_t, Edge> * btree;
    Btree<uint64_t, Edge> * btree_r;
    vector<uint64_t> * landmarks;
    vector<uint64_t> * next_landmarks;
    map<uint64_t, double> * total_dist;
    map<uint64_t, double> * next_total_dist;
    uint64_t * landmark;
    uint64_t * next_landmark;
    bool is_forward = (i % 2 == 0);
    if(is_forward)
    {
      btree           = f_btree;
      btree_r         = b_btree;
      landmarks       = & landmarks_fwd;
      next_landmarks  = & landmarks_rev;
      total_dist      = & total_dist_fwd;
      next_total_dist = & total_dist_rev;
      landmark        = & landmark_fwd;
      next_landmark   = & landmark_rev;
    }else{
      btree           = b_btree;
      btree_r         = f_btree;
      landmarks       = & landmarks_rev;
      next_landmarks  = & landmarks_fwd;
      total_dist      = & total_dist_rev;
      next_total_dist = & total_dist_fwd;
      landmark        = & landmark_rev;
      next_landmark   = & landmark_fwd;
    } 

    cout << "landmark = " << (* landmark) << endl;

    landmarks->push_back((* landmark));

    uint64_t saved = 0;
    if(strategy == "random"){
      cout << "random add" << endl;
      landmark_add<MaltHeuristic>(seq, is_forward, btree, alt_btree, (* landmark), saved);
      (* next_landmark) = seq_get_random(seq);
    }else if(strategy == "visit") {
      /*
      cout << "max_viist add" << endl;
      landmark_add<MaltHeuristic>(seq, is_forward, btree, alt_btree, (* landmark), saved);

      delete landmark_btree;
      landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, true);
      for(vector<uint64_t>::iterator i = landmarks_fwd.begin(); i < landmarks_fwd.end(); i++)
      {
        vector<LandmarkDist> ld = alt_btree->find( (*i) );
        for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
        {
          if(! j->forward)
          {
             LandmarkDist ld;
             ld.forward  = true;
             ld.key      = (*i);
             ld.landmark = j->landmark;
             ld.length   = j->length;
             landmark_btree->insert(ld);
          }
        }
      }

      for(vector<uint64_t>::iterator i = landmarks_rev.begin(); i < landmarks_rev.end(); i++)
      {
        vector<LandmarkDist> ld = alt_btree->find( (*i) );
        for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
        {
          if(j->forward)
          {
            LandmarkDist ld;
            ld.forward  = true;
            ld.key      = j->landmark;
            ld.landmark = (*i);
            ld.length   = j->length;
            landmark_btree->insert(ld);
          }
        }
      }

      MaltHeuristic fwd_h(true,  true, alt_btree, landmark_btree);
      MaltHeuristic rev_h(false, true, alt_btree, landmark_btree);

      (* next_landmark) = landmark_select_maxvisit(seq, f_btree, b_btree, (* landmarks), &fwd_h, &rev_h);
      */
    }else if(strategy == "fard") {
      landmark_add<MaltHeuristic>(seq, is_forward, btree, alt_btree, (* landmark), saved);
      (*landmark) = landmark_select_far<MaltHeuristic>(seq, btree_r, 1, (* landmark), (*landmarks), total_dist);
    }else if(strategy == "farb") {
      landmark_add<MaltHeuristic>(seq, is_forward, btree, alt_btree, (* landmark), saved);
      (*landmark) = landmark_select_far<MaltHeuristic>(seq, btree_r, 2, (* landmark), (*landmarks), total_dist);
    }else if(strategy == "avoid") {
      landmark_add<MaltHeuristic>(seq, is_forward, btree, alt_btree, (* landmark), saved);

      delete landmark_btree;
      landmark_btree = new Btree<uint64_t, LandmarkDist>(pre3_file, true);
      for(vector<uint64_t>::iterator i = landmarks_fwd.begin(); i < landmarks_fwd.end(); i++)
      {
        vector<LandmarkDist> ld = alt_btree->find( (*i) );
        for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
        {
           if(! j->forward)
           {
              LandmarkDist ld;
              ld.forward  = true;
              ld.key      = (*i);
              ld.landmark = j->landmark;
              ld.length   = j->length;
              landmark_btree->insert(ld);
           }
         }
       }

       for(vector<uint64_t>::iterator i = landmarks_rev.begin(); i < landmarks_rev.end(); i++)
       {
         vector<LandmarkDist> ld = alt_btree->find( (*i) );
         for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
         {
           if(j->forward)
           {
              LandmarkDist ld;
              ld.forward  = true;
              ld.key      = j->landmark;
              ld.landmark = (*i);
              ld.length   = j->length;
              landmark_btree->insert(ld);
           }
         }
      }

      MaltHeuristic malt_h(true, false, alt_btree, landmark_btree,  & landmarks_fwd, & landmarks_rev );
      (* next_landmark) = landmark_select_avoid<MaltHeuristic>(seq, f_btree, (* landmarks) , &malt_h);
    }
  }

  map< pair<uint64_t, uint64_t>, double> landmark_dists;

  for(vector<uint64_t>::iterator i = landmarks_fwd.begin(); i < landmarks_fwd.end(); i++)
  {
    vector<LandmarkDist> ld = alt_btree->find( (*i) );
    for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
    {
      if(! j->forward)
      {
         landmark_dists[ make_pair( (*i), j->landmark ) ] = j->length;
         LandmarkDist ld;
         ld.forward  = true;
         ld.key      = (*i);
         ld.landmark = j->landmark;
         ld.length   = j->length;
         landmark_btree->insert(ld);
      }
    }
  }

  for(vector<uint64_t>::iterator i = landmarks_rev.begin(); i < landmarks_rev.end(); i++)
  {
    vector<LandmarkDist> ld = alt_btree->find( (*i) );
    for( vector<LandmarkDist>::iterator j = ld.begin(); j < ld.end(); j++)
    {
      if(j->forward)
      {
         landmark_dists[ make_pair( j->landmark, (*i) ) ] = j->length;
         LandmarkDist ld;
         ld.forward  = true;
         ld.key      = j->landmark;
         ld.landmark = (*i);
         ld.length   = j->length;
         landmark_btree->insert(ld);
      }
    }
  }

  cout << "landmark fwd  size = " << landmarks_fwd.size() << endl;
  cout << "landmark rev  size = " << landmarks_rev.size() << endl; 
  cout << "landmark dists size = " << landmark_dists.size() << endl;

  malt_strip(seq, alt_btree, strip_btree, landmarks_fwd, landmarks_rev, landmark_dists);

  seq->close();
  delete seq; 
  delete f_btree;
  delete b_btree;
  delete alt_btree;
  delete strip_btree;
  delete landmark_btree;

  cout << "malt precompute end" << endl;

  return 1;
}

