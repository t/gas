#ifndef _HOMOGENEOUSBITPROPAGATION_HPP_
#define _HOMOGENEOUSBITPROPAGATION_HPP_

#include "adjlist.h"
#include "BitPropagation.hpp"

#include <vector>

class HomogeneousBitPropagation : public BitPropagation {
private:
    bool reverse;
protected:
    std::vector<double> estimations_cache;
    double epsilon;
public:
    HomogeneousBitPropagation(Adjlist& outlink, const short width, const uint64_t seed);
    ~HomogeneousBitPropagation() {}
    virtual void setReverse();
    virtual void init(double epsilon);
    virtual int nextGeometric();
    virtual double estimateSupporters(double ones);
    virtual double estimateSupporters(int ones);
    virtual double averageOnes();
};

#endif
