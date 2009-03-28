#ifndef HOMOGENEOUS_BIT_PROPAGATION_H_
#define HOMOGENEOUS_BIT_PROPAGATION_H_

#include <vector>
#include <cstdlib>
#include <boost/random.hpp>
#include <stdint.h>

#include "adjlist.h"

template<typename T>
class HomogeneousBitPropagation : BitPropagation<T> {
public:
  HomogeneousBitPropagation(const Adjlist& adj, const uint64_t seed) {
    BitPropagation(adj, seed);
    estimations_cache = std::vector<double>(sizeof(T) * 8 + 1);
    epsilon = -1;
  }

  void init() {
    BitPropagation::init();

    if (epsilon < 1 && epsilon > 0) {
        this->epsilon = epsilon;
    } else {
        // error processing
    }

    LOG(INFO) << "Init with epsilon     : " << epsilon;
    LOG(INFO) << "Ones per bit position : ";

    int i = nextGeometric() + 1;
    double average_bits_set = 0;

    for (int bitpos = 0; bitpos < width; bitpos++) {
        int bitmask_32 = 1 << bitpos;
        int64_t bitmask_64 = 1LL << bitpos;
        int set_bits = 0;

        while (i < num_node) {
            if (width == 32) {
                bitvec_32[i] |= bitmask_32;
            } else if (width == 64) {
                bitvec_64[i] |= bitmask_64;
            }
            set_bits++;
            i = i + nextGeometric() + 1;
        }

        if (i >= num_node) {
            i = i - num_node;
        }

        std::cerr << set_bits << " ";
        average_bits_set += set_bits;
    }
    average_bits_set /= width;
    std::cerr << " (effective epsilon:" << average_bits_set/num_node << ")" << std::endl;

    for (int bit = 0; bit <= width; bit++) {
        estimations_cache[bit] = estimateSupporters((double)bit);
    }
  }

  int nextGeometric() {
    double x = random();
    int k = 0;
    double y = epsilon;
    double p = 1.0 - epsilon;

    while (y < x) {
        y += (epsilon)*p;
        p *= (1.0 - epsilon);
        k++;
    }

    return k;
  }

  double estimateSupporters(double ones) {
    if (ones == 0) {
        return 0;
    } else if (ones >= width) {
        return estimateSupporters(width-1) + (estimateSupporters(width-1) - estimateSupporters(width-2));
    } else {
        double fraction_bits_zero = (width - ones) / width;
        return std::log(fraction_bits_zero)/(std::log((double)num_node - (epsilon * (double)num_node)) - std::log(num_node));
    }
  }

  double estimateSupporters(int ones) {
    return estimations_cache[ones];
  }

  double averageOnes() {
    double average = 0;
    int count = 0;

    for (int i = 0; i < num_node; i++) {
        count = 0;
        if (width == 32) {
            int v = bitvec_32[i];
            for (; v != 0; count++) {
                v &= v-1;
            }
        } else if (width == 64) {
            int64_t v = bitvec_64[i];
            for (; v != 0; count++) {
                v &= v-1;
            }
        }
        average += count;
    }
    average /= num_node;

    return average;
  }

protected:
};

#endif
