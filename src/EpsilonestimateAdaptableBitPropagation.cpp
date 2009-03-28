#include "EpsilonestimateAdaptableBitPropagation.hpp"

#include <iostream>

EpsilonestimateAdaptableBitPropagation::EpsilonestimateAdaptableBitPropagation(const adj_list& outlink, const short width, const uint64_t seed)
    : AdaptableBitPropagation(outlink, width, seed) {}

double EpsilonestimateAdaptableBitPropagation::estimateSupporters(double multiplier) {
    return (2.0*multiplier)/((multiplier+1.0)*epsilon);
}

int EpsilonestimateAdaptableBitPropagation::estimateIfAbove(int minOnes, double multiplier) {
    std::cerr << "Doing Alternative estimation for some nodes (ones >= " << minOnes << "): ";

    double estimated = estimateSupporters(multiplier);
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
            if (ones >= minOnes) {
                estimate[i] = estimated;
                estimated_ok++;
            }
        } else {
            estimated_ok++;
        }
    }
    std::cerr << estimated_ok << " nodes have estimations now." << std::endl;

    return estimated_ok;
}

int EpsilonestimateAdaptableBitPropagation::estimateIfBelow(int maxOnes, double multiplier) {
    std::cerr << "Doing Alternative estimation for some nodes (ones >= " << maxOnes << "): ";

    double estimated = estimateSupporters(multiplier);
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
                estimate[i] = estimated;
                estimated_ok++;
            }
        } else {
            estimated_ok++;
        }
    }
    std::cerr << estimated_ok<< " nodes have estimations now." << std::endl;

    return estimated_ok;
}

int EpsilonestimateAdaptableBitPropagation::estimateAll(double multiplier) {
    std::cerr << "Doing alternative estimations for all nodes : ";

    double estimated = estimateSupporters(multiplier);
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
            estimate[i] = estimated;
            estimated_ok++;
        } else {
            estimated_ok++;
        }
    }

    std::cerr << estimated_ok << " nodes have estimations now." << std::endl;

    return estimated_ok;
}
