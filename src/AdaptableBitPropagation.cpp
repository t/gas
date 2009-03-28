#include "AdaptableBitPropagation.hpp"

#include <iostream>
#include <cmath>
// for round function
#include <math.h>

AdaptableBitPropagation::AdaptableBitPropagation(const adj_list& outlink, const short width, const uint64_t seed)
    : HomogeneousBitPropagation(outlink, width, seed) {}

double AdaptableBitPropagation::estimateSupporters(double ones, double epsilon) {
    if (ones == 0) {
        return 0;
    } else if (ones >= width) {
        return estimateSupporters(width-1, epsilon) + (estimateSupporters(width-1, epsilon) - estimateSupporters(width-2, epsilon));
    } else {
        double fraction_bits_zero = (width - ones) / width;
        return std::log(fraction_bits_zero) / (std::log((double)num_node - (epsilon*(double)num_node)) - std::log(num_node));
    }
}

double AdaptableBitPropagation::estimateSupporters(int ones, int last_ones, double multiplier) {
    double last_epsilon = epsilon / multiplier;

    if (last_ones == 0 || last_ones >= width) {
        return estimateSupporters(ones, epsilon);
    } else if (ones == 0 || ones >= width) {
        return estimateSupporters(last_ones, last_epsilon);
    } else {
        return (estimateSupporters(ones, epsilon) + estimateSupporters(last_ones, last_epsilon)) / 2.0;
    }
}

int AdaptableBitPropagation::estimateIfAbove(int minOnes, double multiplier) {
    std::cerr << "Doing estimation for some nodes (ones >=" << minOnes << "): ";

    int estimated_ok = 0;
    for (int i = 0; i < num_node; i++) {
        int ones = 0;
        if (estimate[i] < 0) {
            if (width == 32) {
                int v = bitvec_32[i];
                for (; v != 0; ones++) {
                    v &= v-1;
                }
            } else if (width == 64) {
                long long v = bitvec_64[i];
                for (; v != 0; ones++) {
                    v &= v-1;
                }
            }
            if (ones >= minOnes) {
                estimate[i] = estimateSupporters(ones, (int)(round(-estimate[i])), multiplier);
                estimated_ok++;
            } else {
                estimate[i] = -ones;
            }
        } else {
            estimated_ok++;
        }
    }
    std::cerr << estimated_ok << " nodes have estimations now." << std::endl;;

    return estimated_ok;
}

int AdaptableBitPropagation::estimateIfBelow(int maxOnes, double multiplier) {
    std::cerr << "Doing estimation for some nodes (ones <=" << maxOnes << "): ";

    int estimated_ok = 0;
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
            if (ones <= maxOnes) {
                estimate[i] = estimateSupporters(ones, (int)(round(-estimate[i])), multiplier);
                estimated_ok++;
            } else {
                estimate[i] = -ones;
            }
        } else {
            estimated_ok++;
        }
    }
    std::cerr << estimated_ok << " nodes have estimations now." << std::endl;

    return estimated_ok;
}

int AdaptableBitPropagation::estimateAll(double multiplier) {
    std::cerr << "Doing estimation for all nodes: ";

    int estimated_ok = 0;
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
            estimate[i] = estimateSupporters(ones, (int)(round(-estimate[i])), multiplier);
            estimated_ok++;
        } else {
            estimated_ok++;
        }
    }
    std::cerr << estimated_ok << " nodes have estimations now." << std::endl;

    return estimated_ok;
}
