#ifndef _ADAPTABLEBITPROPAGATION_HPP_
#define _ADAPTABLEBITPROPAGATION_HPP_

#include "adjlist.h"
#include "HomogeneousBitPropagation.hpp"

class AdaptableBitPropagation : public HomogeneousBitPropagation {
public:
    AdaptableBitPropagation(Adjlist& outlink, const short width, const uint64_t seed);
    virtual double estimateSupporters(double ones, double epsilon);
    virtual double estimateSupporters(int ones, int last_ones, double multiplier);
    virtual int estimateIfAbove(int minOnes, double multiplier);
    virtual int estimateIfBelow(int maxOnes, double multiplier);
    virtual int estimateAll(double multiplier);
};

#endif
