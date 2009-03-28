#include "ANFBitPropagation.hpp"

#include <iostream>
#include <cmath>
// for round function
#include <math.h>

ANFBitPropagation::ANFBitPropagation(const adj_list& outlink, const short width, const uint64_t seed)
    :BitPropagation(outlink, width, seed) {
    estimate = std::vector<double>(num_node);
}

void ANFBitPropagation::init() {
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
            if (width == 32) {
                bitvec_32[i] |= bitmask_32;
            } else if (width == 64) {
                bitvec_64[i] |= bitmask_64;
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

int ANFBitPropagation::nextGeometric(double probability) {
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

double ANFBitPropagation::estimateSupporters(double first_zero) {
    return std::pow(2.0, first_zero) / 0.77351;
}

double ANFBitPropagation::estimateSupporters(int first_zero) {
    return estimateSupporters(round(first_zero));
}

void ANFBitPropagation::updateAverageFirstZero() {
    std::cerr << "  Updating average position of first zero: ";

    double avg_position = 0;

    for (int i = 0; i < num_node; i++) {
        int first_zero = 0;
        if (width == 32) {
            int bitmask_32 = 1;
            int v = bitvec_32[i];
            while (((v & bitmask_32) != 0) && (first_zero < width)) {
                bitmask_32 = bitmask_32 << 1;
                first_zero++;
            }
        } else if (width == 64) {
            int64_t bitmask_64 = 1LL;
            int64_t v = bitvec_64[i];
            while (((v & bitmask_64) != 0) && (first_zero < width)) {
                bitmask_64 = bitmask_64 << 1;
                first_zero++;
            }
        }

        estimate[i] += first_zero;
        avg_position += first_zero;
    }
    avg_position /= num_node;

    std::cerr << "(average " << avg_position << ") done." << std::endl;
}

void ANFBitPropagation::estimateAll(short nrounds) {
    std::cerr << "  Doing ANF estimation after " << nrounds << " rounds: ";
    for (int i = 0; i < num_node; i++) {
        double first_zero = (double)estimate[i] / (double)nrounds;
        estimate[i] = estimateSupporters(first_zero);
    }
    std::cerr << "done." << std::endl;
}
