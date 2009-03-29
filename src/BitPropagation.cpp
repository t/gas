#include "BitPropagation.hpp"

#include <iostream>
#include <vector>
#include <boost/random.hpp>
#include "adjlist.h"

using namespace std;

BitPropagation::BitPropagation(Adjlist& outlink, const short width, const uint64_t seed)
  : gen(static_cast<uint64_t>(seed)), dst(0, 1), random(gen, dst) {

  LOG(INFO) << "BitPropagation::BitPropagation started";

  num_node = (* outlink.begin());
  this->outlink = outlink;
  this->width   = width;
  this->bitvec_32 = vector<int32_t>(num_node);
  this->bitvec_32_aux = vector<int32_t>(num_node);
  this->bitvec_64     = vector<int64_t>(num_node);
  this->bitvec_64_aux = vector<int64_t>(num_node);
  this->estimate = vector<double>(num_node, -(width*2));
  reverse = false;

  LOG(INFO) << "BitPropagation::BitPropagation finished";
}

void BitPropagation::init() {
    this->distance = 0;

    if (width == 32) {
        for (int i = 0; i < num_node; i++) {
            bitvec_32[i] = 0;
        }
    } else if (width == 64) {
        for (int i = 0; i < num_node; i++) {
            bitvec_64[i] = 0;
        }
    } else {
        // error processing
    }
}

void BitPropagation::reverseLinks() {
    reverse = true;
}

void BitPropagation::step() {
    distance++;

    LOG(INFO) << "Current distance:" << distance;

    if (width == 32) {
        bitvec_32_aux = std::vector<int>(num_node);
    } else if (width == 64) {
        bitvec_64_aux = std::vector<int64_t>(num_node);
    }

    LOG(INFO) << "  Calculation step: ";

    Adjlist::iterator end = outlink.end(); 
    uint64_t src = 0;
    for(Adjlist::iterator adj_i = outlink.begin() + 1; adj_i != end; ++adj_i){
      uint64_t edge_count = (*adj_i);
      for(uint64_t i = 0; i < edge_count; ++i){
        ++adj_i;
        if(width == 32){
          int32_t next = (*adj_i);
          if(reverse){
            bitvec_32_aux[src]  |= bitvec_32[next];
          }else{
            bitvec_32_aux[next] |= bitvec_32[src]; 
          }
        }else if(width == 64){
          int64_t next = (*adj_i);
          if(reverse){
            bitvec_64_aux[src]  |= bitvec_64[next];
          }else{
            bitvec_64_aux[next] |= bitvec_64[src]; 
          }
        }else{
          assert(false);
        }
      }

    }

    if (width == 32) {
        if (distance > 1) {
            for (int i = 0; i < num_node; i++) {
                bitvec_32[i] |= bitvec_32_aux[i];
            }
        } else {
            for (int i = 0; i < num_node; i++) {
                bitvec_32[i] = bitvec_32_aux[i];
            }
        }
    } else if (width == 64) {
        if (distance > 1) {
            for (int i = 0; i < num_node; i++) {
                bitvec_64[i] |= bitvec_64_aux[i];
            }
        } else {
            for (int i = 0; i < num_node; i++) {
                bitvec_64[i] = bitvec_64_aux[i];
            }
        }
    }

    LOG(INFO) << "done.";
}

void BitPropagation::estimateAll() {
    LOG(INFO) << "  Doing estimation: ";
    for (int i = 0; i < num_node; i++) {
        int ones = 0;
        if (estimate[i] < 0) {
            if (width == 32) {
                int v = bitvec_32[i];
                for (; v != 0; ones++) {
                    v &= v-1;
                }
            } else if (width == 64) {
                int64_t v = bitvec_64[i];
                for (; v != 0; ones++) {
                    v &= v-1;
                }
            }
            estimate[i] = estimateSupporters(ones);
        }
    }

    LOG(INFO) << "done.";
}

