#ifndef ANF_BIT_PROPAGATION_H_
#define ANF_BIT_PROPAGATION_H_

#include <vector>
#include <cstdlib>
#include <boost/random.hpp>
#include <stdint.h>

#include "adjlist.h"

template<typename T>
class AnfBitPropagation : BitPropagation<T> {
public:
  AnfBitPropagation(const Adjlist& adj, const uint64_t seed) {
    BitPropagation(adj, seed);
  }

  void init() {
    BitPropagation::init();

    std::cerr << "Ones per bit position : ";

    bool last_was_zero = false;
    bool next_last_was_zero = false;

    for (int bitpos = 0; bitpos < width; bitpos++) {
        int bitmask_32 = 1 << bitpos;
        int64_t bitmask_64 = 1LL << bitpos;
        int set_bits = 0;
        double probability = std::pow(0.5, bitpos+1);

        int i = nextGeometric(probability) + 1;
        while (i < num_node) {
          bitvec_32[i] |= bitmask_32;
         }
         set_bits++;
         i = i + nextGeometric(probability) + 1;
        }

        std::cerr << set_bits << " ";

        if (set_bits == 0 && last_was_zero && next_last_was_zero) {
            std::cerr << "(Last 3 were zero, stopping at bit " << bitpos << ")";
            break;
        } else {
            next_last_was_zero = last_was_zero;
            last_was_zero = (set_bits == 0);
        }
    }
    std::cerr << std::endl;
  }

  int nextGeometric(double probability) {
    double x = random();
    int k = 0;
    double y = probability;
    double p = 1.0 - probability;

    while (y < x) {
        y += (probability) * p;
        p *= (1.0 - probability);
        k++;
    }

    return k;
  }

  double estimate(double first_zero) {
    return std::pow(2.0, first_zero) / 0.77351;
  }

  void update_average_first_zero() {
    LOG(INFO) << "  Updating average position of first zero: ";

    double avg_position = 0;

    for (int i = 0; i < num_node; i++) {
        int first_zero = 0;
        int bitmask_32 = 1;
        int v = bitvec_32[i];
        while (((v & bitmask_32) != 0) && (first_zero < width)) {
            bitmask_32 = bitmask_32 << 1;
            first_zero++;
        }

        estimate[i]  += first_zero;
        avg_position += first_zero;
    }
    avg_position /= num_node;

    LOG(INFO) << "(average " << avg_position << ") done.";
  }

  void estimateAll(short nrounds) {
    std::cerr << "  Doing ANF estimation after " << nrounds << " rounds: ";
    for (int i = 0; i < num_node; i++) {
        double first_zero = (double)estimate[i] / (double)nrounds;
        estimate[i] = estimateSupporters(first_zero);
    }
    std::cerr << "done." << std::endl;
  }

  ~AnfBitPropagation() {}

protected:
};

#endif
