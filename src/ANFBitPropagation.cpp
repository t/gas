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

#include "ANFBitPropagation.hpp"

#include <iostream>
#include <cmath>
// for round function
#include <math.h>
#include "adjlist.h"

ANFBitPropagation::ANFBitPropagation(Adjlist& outlink, const short width, const uint64_t seed)
    :BitPropagation(outlink, width, seed) {
    estimate = std::vector<double>(num_node);
}

void ANFBitPropagation::init() {
    BitPropagation::init();

    LOG(INFO) << "Ones per bit position : ";

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

        LOG(INFO) << set_bits << " ";

        if (set_bits == 0 && last_was_zero && next_last_was_zero) {
            LOG(INFO) << "(Last 3 were zero, stopping at bit " << bitpos << ")";
            break;
        } else {
            next_last_was_zero = last_was_zero;
            last_was_zero = (set_bits == 0);
        }
    }
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
    LOG(INFO) << "  Updating average position of first zero: ";

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

    LOG(INFO) << "(average " << avg_position << ") done.";
}

void ANFBitPropagation::estimateAll(short nrounds) {
    LOG(INFO) << "  Doing ANF estimation after " << nrounds << " rounds: ";
    for (int i = 0; i < num_node; i++) {
        double first_zero = (double)estimate[i] / (double)nrounds;
        estimate[i] = estimateSupporters(first_zero);
    }
    LOG(INFO) << "done.";
}
