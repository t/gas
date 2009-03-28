#ifndef BIT_PROPAGATION_H_
#define BIT_PROPAGATION_H_

#include <vector>
#include <cstdlib>
#include <boost/random.hpp>
#include <stdint.h>

#include "adjlist.h"

template<typename T>
class BitPropagation {
public:
  BitPropagation(const Adjlist& adj, const uint64_t seed){
    adj_ = adj;
    node_count = (* adj.begin());

       /*
      bitvec_32(num_node), bitvec_32_aux(num_node),
      bitvec_64(num_node), bitvec_64_aux(num_node),
      estimate(num_node, -(width*2)), dout(num_node),
      gen(static_cast<uint64_t>(seed)), dst(0, 1), random(gen, dst) {
        for (int i = 0; i < num_node; i++) dout[i] = outlink[i].size();
        reverse = false;
      */
  }

  ~BitPropagation() {}

  int get_num_node() const { return num_node; }
  std::vector<double> get_estimate() const { return estimate; }

  void init(){
    distance_ = 0;
    bitvec_   = vector<T>(node_count_, 0);
  }

  void step(){
    distance_ ++;
    LOG(INFO) << "Current distance:" << distance_;

    bitvec_aux_ = std::vector<int>(node_count_);

    Adjlist::iterator end = adj_.end();
    Adjlist::iterator i = adj_.begin(); i != end; ++i){
    for(uint64_t i = 0; i < node_count_; ++i) {
      if (bitvec_[i] != 0) {
        for (uint64_t j = 0; j < dout[i]; ++j) {
          int next = outlink[i][j];
          bitvec_aux_[i] |= bitvec_[next];
        }
      }
    }
    if (distance == 1) {
      bitvec_ = bitvec_aux_;
    }else{
      for(uint64_t i = 0; i < num_node_; ++i) {
        bitvec_[i] |= bitvec_aux_[i];
      }
    }

    std::cerr << "done." << std::endl;
  }
 
  void propagation(){
    int  iteration = 0;
    while(! this->done()) {
      ++iteration;
      LOG(INFO) << "Iteration " << iteration << ", target distance " << distance << endl;
      for(int it = 0; it < distance; it++) {
        this->step();
      }
      this->step_after();
    }
    LOG(INFO) << "Estimating supporters for all nodes";
  }

  void estimate_all() {
    LOG(INFO) << "  Doing estimation: ";
    for (uint64_t i = 0; i < num_node_; ++i) {
      uint64_t ones = 0;
      T v = bitvec_[i];
      for (; v != 0; ones++) {
         v &= v - 1;
      }
      estimate[i] = this>estimate(ones);
    }

    LOG(INFO) << "done.";
  }

  virtual double estimate(int ones)    = 0;
  virtual double estimate(double ones) = 0;

protected:
  Adjlist adj_;
  std::vector<T> bitvec_;
  std::vector<T> bitvec_aux_;
  std::vector<double> estimate_;
  uint64_t distance_;
  uint64_t node_count_;

  std::vector<int> dout;

  // for random genetator
  boost::mt19937 gen;
  boost::uniform_real<> dst;
  boost::variate_generator<boost::mt19937, boost::uniform_real<> > random;
  std::vector<int> leader_docid;
};

#endif
