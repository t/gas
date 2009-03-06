#ifndef MALT_HEURISTIC_H_
#define MALT_HEURISTIC_H_

#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "landmark_dist.h"

using namespace std;
using namespace boost;

class MaltHeuristic 
{
private:
  Btree<uint64_t, LandmarkDist> * _malt_btree;
  Btree<uint64_t, LandmarkDist> * _malt_landmark_btree;
  vector<uint64_t>              * _limit_landmarks_fwd;
  vector<uint64_t>              * _limit_landmarks_rev;
  bool                            _is_forward;
  bool                            _is_bi;

  double p_star_f(uint64_t source, uint64_t target, uint64_t current)
  {
    double max;

    max  = (h_star(current, target) - h_star(source, current)) / 2;
    max += h_star(source, target)  / 2;

    return max;
  }

  double p_star_r(uint64_t source, uint64_t target, uint64_t current)
  {
    double max;

    max  = (h_star(source, current) - h_star(current, target)) / 2;
    max += h_star(source, target)  / 2;
  }

public:
  MaltHeuristic()
  {
  }

  MaltHeuristic(
      bool                            is_forward,
      bool                            is_bi,
      Btree<uint64_t, LandmarkDist> * malt_btree,
      Btree<uint64_t, LandmarkDist> * malt_landmark_btree,
      vector<uint64_t>              * limit_landmarks_fwd = NULL,
      vector<uint64_t>              * limit_landmarks_rev = NULL
    )
  {
    _malt_btree          = malt_btree;
    _malt_landmark_btree = malt_landmark_btree;
    _limit_landmarks_fwd = limit_landmarks_fwd;
    _limit_landmarks_rev = limit_landmarks_rev;
    _is_forward          = is_forward;
    _is_bi               = is_bi;
  }

  double h_star(uint64_t source, uint64_t target)
  {
    double max = 0;

    vector<LandmarkDist> source_dists = _malt_btree->find(source);
    vector<LandmarkDist> target_dists = _malt_btree->find(target);

    vector< map<uint64_t, vector<double> > > bi(2);

    vector< pair<uint64_t, double> > available_fwd;
    vector< pair<uint64_t, double> > available_rev;
    for(vector<LandmarkDist>::iterator i = source_dists.begin(); i != source_dists.end(); i++)
    {
      if(_limit_landmarks_fwd){
        if( find(_limit_landmarks_fwd->begin(), _limit_landmarks_fwd->end(), i->landmark) != _limit_landmarks_fwd->end()){
            continue;
        }
      }

      if(i->forward)
      {
         available_fwd.push_back( make_pair( i->landmark, i->length ) );
      }
    }

    for(vector<LandmarkDist>::iterator i = target_dists.begin(); i != target_dists.end(); i++)
    {
      if(_limit_landmarks_rev){
        if( find(_limit_landmarks_rev->begin(), _limit_landmarks_rev->end(), i->landmark) != _limit_landmarks_rev->end()){
            continue;
        }
      }

      if(! i->forward)
      {
         available_rev.push_back( make_pair( i->landmark, i->length ) );
      }
    }

    /*
    pair<uint64_t, double> min_fwd = make_pair(0, UINT_MAX);
    pair<uint64_t, double> min_rev = make_pair(0, UINT_MAX);

    for(vector< pair<uint64_t, double> >::iterator i = available_fwd.begin(); i < available_fwd.end(); i++)
    {
      if((*i).second < min_fwd.second) min_fwd = (*i);
    }
   
    for(vector< pair<uint64_t, double> >::iterator i = available_rev.begin(); i < available_rev.end(); i++)
    {
      if((*i).second < min_rev.second) min_rev = (*i);
    } 
    available_fwd.clear(); 
    available_fwd.push_back( min_fwd );
    available_rev.clear();
    available_rev.push_back( min_rev );
    */

    //cout << "available: fwd = " << available_fwd.size() <<   "  rev = " << available_rev.size() << endl;

    for(vector< pair<uint64_t, double> >::iterator i = available_fwd.begin(); i < available_fwd.end(); i++)
    {
       double org_p = i->second;

       vector<LandmarkDist> landmark_dists = _malt_landmark_btree->find(i->first);
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
         if(d2 > max)
         {
            max = d2;
         }
      }
    }

    return max;
  }

  double operator()(uint64_t source, uint64_t target, uint64_t current)
  {
     if(! _is_bi) return h_star(current, target);
     if(! _is_forward) swap(source, target);

     double result;
     if(_is_forward)
     {
        result = p_star_f(source, target, current);
     }else{
        result = p_star_r(source, target, current);
     }
    
     //cout << "result = " << result << endl;

     return result;
  }

};

#endif
