#ifndef _BITPROPAGATION_HPP_
#define _BITPROPAGATION_HPP_

#include <vector>
#include <cstdlib>
#include <boost/random.hpp>
#include <stdint.h>

#include "adjlist.h"

class BitPropagation {
private:
    bool reverse;
protected:
    Adjlist outlink;
    int num_node;
    short width;

    std::vector<int> bitvec_32;
    std::vector<int> bitvec_32_aux;
    std::vector<int64_t> bitvec_64;
    std::vector<int64_t> bitvec_64_aux;

    std::vector<double> estimate;

    int distance;

    // for random genetator
    boost::mt19937 gen;
    boost::uniform_real<> dst;
    boost::variate_generator<boost::mt19937, boost::uniform_real<> > random;

    std::vector<int> leader_docid;
public:
    BitPropagation(Adjlist& outlink, const short width, const uint64_t seed);
    ~BitPropagation() {}

    int get_num_node() const { return num_node; }
    std::vector<double> get_estimate() const { return estimate; }

    void init();
    virtual void reverseLinks();
    virtual void step();

    //virtual double estimateSupporters(int ones);
    //virtual double estimateSupporters(double ones);

    virtual double estimateSupporters(int ones)    = 0;
    virtual double estimateSupporters(double ones) = 0;

    virtual void estimateAll();

    //void prepareSiteMap(std::string& fileSite);
    //void doSiteMap();
};

#endif
