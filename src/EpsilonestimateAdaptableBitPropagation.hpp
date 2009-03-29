#ifndef _EPSILONESTIMATEADAPTABLEBITPROPAGATION_HPP_
#define _EPSILONESTIMATEADAPTABLEBITPROPAGATION_HPP_

#include "adjlist.h"
#include "AdaptableBitPropagation.hpp"

class EpsilonestimateAdaptableBitPropagation : public AdaptableBitPropagation {
public:
    EpsilonestimateAdaptableBitPropagation(Adjlist& outlink, const short width, const uint64_t seed);
    virtual double estimateSupporters(double multiplier);
    virtual int estimateIfAbove(int minOnes, double multiplier);
    virtual int estimateIfBelow(int maxOnes, double multiplier);
    virtual int estimateAll(double multiplier);
};

#endif
