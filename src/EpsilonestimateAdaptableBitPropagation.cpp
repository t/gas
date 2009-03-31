/*
 This code is based on the following Java Code by Ricardo Baeza-Yates, et al. 
 http://www.yr-bcn.es/webspam/code/ 
 The Original code is distributed under GPL. Below text is Original Code's licence.

 -----------------

 Copyright (C) 2006 - Ricardo Baeza-Yates, Luca Becchetti, Carlos Castillo
    Debora Donato, Stefano Leonardi :: http://www.dis.uniroma1.it/~ae/
 This work was partially supported by EU IST-FET project 001907 (DELIS).

 Portions derived from the WebGraph framework.
 Copyright (C) 2003, 2004, 2005 - Paolo Boldi, Massimo Santini
                                     and Sebastiano Vigna

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "EpsilonestimateAdaptableBitPropagation.hpp"

#include <iostream>

EpsilonestimateAdaptableBitPropagation::EpsilonestimateAdaptableBitPropagation(Adjlist& outlink, const short width, const uint64_t seed)
    : AdaptableBitPropagation(outlink, width, seed) {}

double EpsilonestimateAdaptableBitPropagation::estimateSupporters(double multiplier) {
    return (2.0*multiplier)/((multiplier+1.0)*epsilon);
}

int EpsilonestimateAdaptableBitPropagation::estimateIfAbove(int minOnes, double multiplier) {
    LOG(INFO) << "Doing Alternative estimation for some nodes (ones >= " << minOnes << "): ";

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
    LOG(INFO) << estimated_ok << " nodes have estimations now.";

    return estimated_ok;
}

int EpsilonestimateAdaptableBitPropagation::estimateIfBelow(int maxOnes, double multiplier) {
    LOG(INFO) << "Doing Alternative estimation for some nodes (ones >= " << maxOnes << "): ";

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
    LOG(INFO) << estimated_ok<< " nodes have estimations now.";

    return estimated_ok;
}

int EpsilonestimateAdaptableBitPropagation::estimateAll(double multiplier) {
    LOG(INFO) << "Doing alternative estimations for all nodes : ";

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

    LOG(INFO) << estimated_ok << " nodes have estimations now.";

    return estimated_ok;
}
