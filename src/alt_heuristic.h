#ifndef ALT_HEURISTIC_H_
#define ALT_HEURISTIC_H_

#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include "landmark_dist.h"

using namespace std;
using namespace boost;

class AltHeuristic
{
private:
  Btree<uint64_t, LandmarkDist> * _alt_btree;
  bool                            _is_forward;
  vector<uint64_t>              * _limit_landmarks;

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
    return max;
  }

public:
  AltHeuristic()
  {
  }

  AltHeuristic(bool is_forward, Btree<uint64_t, LandmarkDist> * alt_btree, vector<uint64_t> * limit_landmarks = NULL)
  {
     _is_forward      = is_forward;
     _alt_btree       = alt_btree;
     _limit_landmarks = limit_landmarks;
  }

  double h_star(uint64_t s, uint64_t t)
  {
    double max = 0;

    vector<LandmarkDist> source_dists = _alt_btree->find(s);
    vector<LandmarkDist> target_dists = _alt_btree->find(t);

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
        if(_limit_landmarks)
        {
          if( find(_limit_landmarks->begin(), _limit_landmarks->end(), landmark) != _limit_landmarks->end()) 
          {
            continue;
          }
        }

        if( (*j).second.size() != 2) {
          if((*j).second.size() > 1){
            //cout << "ignore - " << (*j).second.size() << endl;
          }
          continue;
        }

        double d = (*j).second[1] - (*j).second[0];
        if(d > max) max = d;
      }
    }

    //cout << "s = " << s << " t = " << t << " max = " << max << "  is_forward = " << _is_forward << endl;
    return max;
  }

  double operator()(uint64_t source, uint64_t target, uint64_t current)
  {
     double result;

     if(_is_forward)
     {
        result = p_star_f(source, target, current);
     }else{
        result = p_star_r(target, source, current);
     }
     //cout << "result = " << result << endl;

     return result;
  }
};

#endif
