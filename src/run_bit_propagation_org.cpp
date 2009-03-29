#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <boost/lexical_cast.hpp>
#include <google/gflags.h>
#include <glog/logging.h>

#include "db.h"
#include "adjlist.h"
#include "BitPropagation.hpp"
#include "ANFBitPropagation.hpp"
#include "HomogeneousBitPropagation.hpp"
#include "AdaptableBitPropagation.hpp"
#include "EpsilonestimateAdaptableBitPropagation.hpp"

DEFINE_int32( width,                        64,    "bit vector width 32 or 64");
DEFINE_int32( distance,                     25,    "distance to explore");
DEFINE_double(epsilon_factor,               1.0,   "probability of initializing a bit as 1, in terms of 1/N");
DEFINE_bool  (reverse,                      false, "reverse links, counting, out-neighbors");
DEFINE_int32 (anf,                          false, "use ANF method for arg times iteration");
DEFINE_uint64(seed,                         0,     "seed of random number generator [default: std::time(0)]");
DEFINE_bool(  adaptive,                     false, "adjust epsilon adaptively");
DEFINE_double(adaptive_multiplier,          2.0,   "factor for increasing epsilon");
DEFINE_double(adaptive_epsilon_max,         0.5,   "maximum epsilon");
DEFINE_double(adaptive_epsilon_min_factor,  1.0,   "minimum epsilon factor");
DEFINE_double(adaptive_use_epsilonestimate, false, "use alternative estimation of 2/(M+1)e, disregarding bits");
DEFINE_double(adaptive_stop_criteria,       0.99,  "fraction of nodes with good estimations required to stop");

using namespace std;
using namespace boost;

int run_bit_propagation_org() 
{
  if(! FLAGS_seed) FLAGS_seed = std::time(0);

  // create graph as adjacency list
  Adjlist graph = adjlist_create(true);
  graph.open();
  uint64_t node_count = (* graph.begin() );

  LOG(INFO) << "Number of nodes : " << node_count;
  LOG(INFO) << "Number of bits : "  << FLAGS_width;
  LOG(INFO) << "Distance : "        << FLAGS_distance;
  LOG(INFO) << "Random Seed : "     << FLAGS_seed;

  double epsilon;
  double adaptive_epsilon_min = 0;
  if (FLAGS_adaptive) {
    LOG(INFO) << "Stop Criteria : " << FLAGS_adaptive_stop_criteria << " of nodes with good estimations";
    if (FLAGS_adaptive_use_epsilonestimate) {
      LOG(INFO) << "Using eplison-only estimate ";
    }
    adaptive_epsilon_min = FLAGS_adaptive_epsilon_min_factor / node_count;
    LOG(INFO) << "ADAPTIVE strategy enabled : multiplier=" << FLAGS_adaptive_multiplier <<  " maximum=" << FLAGS_adaptive_epsilon_max << " minimum=" << adaptive_epsilon_min << endl;
    if (FLAGS_adaptive_multiplier > 1) {
      epsilon = adaptive_epsilon_min;
    } else {
      epsilon = FLAGS_adaptive_epsilon_max;
    }
  } else {
    if (FLAGS_anf) {
      LOG(INFO) << "Using ANF iterations : " << FLAGS_anf;
    }
    if (FLAGS_epsilon_factor != 1.0) {
      LOG(INFO) << "Epsilon factor provided   : " << FLAGS_epsilon_factor;
    }
    epsilon = FLAGS_epsilon_factor / node_count;
  }
  LOG(INFO) << "initializing algorithm";  
  
  BitPropagation* algorithm = NULL;
  if (FLAGS_adaptive) {
    if (FLAGS_adaptive_use_epsilonestimate) {
      LOG(INFO) << "initializing EpsilonestimateAdaptableBitPropagation algorithm";
      algorithm = new EpsilonestimateAdaptableBitPropagation(graph, FLAGS_width, FLAGS_seed);
    } else {
      LOG(INFO) << "initializing AdaptableBitPropagation algorithm";
      algorithm = new AdaptableBitPropagation(graph, FLAGS_width, FLAGS_seed);
    }
  } else {
     if (FLAGS_anf) {
       LOG(INFO) << "initializing ANFBitPropagation algorithm";
       algorithm = new ANFBitPropagation(graph, FLAGS_width, FLAGS_seed);
     } else {
       LOG(INFO) << "initializing HomogeneousBitPropagation algorithm";
       algorithm = new HomogeneousBitPropagation(graph, FLAGS_width, FLAGS_seed);
     }
  }
  LOG(INFO) << "initialized algorithm"; 
  
  if (FLAGS_reverse) {
    algorithm->reverseLinks();
  }
    
  if (FLAGS_anf) {
    dynamic_cast<ANFBitPropagation*>(algorithm)->init();
  } else {
    dynamic_cast<HomogeneousBitPropagation*>(algorithm)->init(epsilon);
  }
    
  int estimated_ok = 0;
  bool done = false;
  int runs = 0;
    
  while (!done) {
    LOG(INFO) << "Iteration " << (runs+1) << ", target distance " << FLAGS_distance;
    for (int it = 1; it <= FLAGS_distance; it++) {
      algorithm->step();
     }
        
     if (FLAGS_adaptive) {
       if (FLAGS_adaptive_multiplier > 1) {
         estimated_ok = dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateIfAbove(static_cast<int>(0.629629 * FLAGS_width), FLAGS_adaptive_multiplier);
       } else {
         estimated_ok = dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateIfBelow(static_cast<int>(0.629629 * FLAGS_width), FLAGS_adaptive_multiplier);
     }
            
     LOG(INFO) << "Fraction of nodes with good estimations: " << estimated_ok << "/" << algorithm->get_num_node() << "=" << static_cast<double>(estimated_ok) / algorithm->get_num_node();
            
     if (estimated_ok >= algorithm->get_num_node() * FLAGS_adaptive_stop_criteria) {
       LOG(INFO) << "Fraction of nodes with good estimates (" << FLAGS_adaptive_stop_criteria << ") reached for adaptive strategy" << endl;
       done = true;
     } else if (epsilon * FLAGS_adaptive_multiplier < adaptive_epsilon_min) {
       LOG(INFO) << "Minimum (" << adaptive_epsilon_min << ") reached for adaptive strategy" << endl;
       done = true;
     } else if (epsilon * FLAGS_adaptive_multiplier > FLAGS_adaptive_epsilon_max) {
       LOG(INFO) << "Maximum (" << FLAGS_adaptive_epsilon_max << ") reached for adaptive strategy" << endl;
       done = true;
     } else {
       epsilon *= FLAGS_adaptive_multiplier;
       LOG(INFO) << "Adaptive strategy enabled, continuing eith epsilon=" << epsilon << endl;
       dynamic_cast<AdaptableBitPropagation*>(algorithm)->init(epsilon);
     }
            
     runs++;
   } else {
     if (FLAGS_anf) {
       dynamic_cast<ANFBitPropagation*>(algorithm)->updateAverageFirstZero();
       runs++;
                if (runs == FLAGS_anf) {
                    done = true;
                } else {
                    dynamic_cast<ANFBitPropagation*>(algorithm)->init();
                }
            } else {
                done = true;
            }
        }
    }
    
    LOG(INFO) << "Estimating supporters for all nodes";

    if (FLAGS_adaptive) {
        dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateAll(FLAGS_adaptive_multiplier);
    } else {
        if (FLAGS_anf) {
            dynamic_cast<ANFBitPropagation*>(algorithm)->estimateAll(FLAGS_anf);
        } else {
            algorithm->estimateAll();
        }
    }

    if (FLAGS_adaptive || FLAGS_anf) {
      LOG(INFO) << "Done after " << runs << " runs";
    }

    vector<double> estimate = algorithm->get_estimate();
    uint64_t num_node = algorithm->get_num_node();

    const string seq_file  = db_path(FILE_SEQUENCE);
    MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
    seq->open(false);

    cout << "result:" << endl;
    cout << "  supporters:" << endl;
    for (int i = 0; i < num_node; i++) {
      cout << "    " << (* seq->at(i) ) << ": " << round(estimate[i]) << endl;
    }
    LOG(INFO) << "done.";

    seq->close();
    graph.close();

    return 0;
}
