#ifndef _ANFBITPROPAGATION_HPP_
#define _ANFBITPROPAGATION_HPP_

#include "adjlist.h"
#include "BitPropagation.hpp"

class ANFBitPropagation : public BitPropagation {
public:
    ANFBitPropagation( Adjlist& outlink, const short width, const uint64_t seed);
    void init();
    int nextGeometric(double probability);
    double estimateSupporters(double first_zero);
    double estimateSupporters(int first_zero);
    void updateAverageFirstZero();
    void estimateAll(short nrounds);
};

#endif
