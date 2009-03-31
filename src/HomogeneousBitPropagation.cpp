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

#include "HomogeneousBitPropagation.hpp"

#include <cmath>

HomogeneousBitPropagation::HomogeneousBitPropagation(Adjlist& outlink, const short width, const uint64_t seed)
    : BitPropagation(outlink, width, seed) {

    LOG(INFO) << "HomogeneousBitPropagation::HomogeneousBitPropagation started";

    estimations_cache = std::vector<double>(width + 1);
    epsilon = -1;

    LOG(INFO) << "HomogeneousBitPropagation::HomogeneousBitPropagation finished";
}

void HomogeneousBitPropagation::setReverse() {
    reverse = true;
}

void HomogeneousBitPropagation::init(double epsilon) {
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

        LOG(INFO) << set_bits;
        average_bits_set += set_bits;
    }
    average_bits_set /= width;
    LOG(INFO) << " (effective epsilon:" << average_bits_set/num_node << ")" ;

    for (int bit = 0; bit <= width; bit++) {
        estimations_cache[bit] = estimateSupporters((double)bit);
    }
}

int HomogeneousBitPropagation::nextGeometric() {
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

double HomogeneousBitPropagation::estimateSupporters(double ones) {
    if (ones == 0) {
        return 0;
    } else if (ones >= width) {
        return estimateSupporters(width-1) + (estimateSupporters(width-1) - estimateSupporters(width-2));
    } else {
        double fraction_bits_zero = (width - ones) / width;
        return std::log(fraction_bits_zero)/(std::log((double)num_node - (epsilon * (double)num_node)) - std::log(num_node));
    }
}

double HomogeneousBitPropagation::estimateSupporters(int ones) {
    return estimations_cache[ones];
}

double HomogeneousBitPropagation::averageOnes() {
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
