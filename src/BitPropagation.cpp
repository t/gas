#include "BitPropagation.hpp"

#include <iostream>
#include <vector>
#include <boost/random.hpp>

BitPropagation::BitPropagation(const adj_list& outlink, const short width, const uint64_t seed)
    : outlink(outlink), num_node(outlink.size()), width(width),
      bitvec_32(num_node), bitvec_32_aux(num_node),
      bitvec_64(num_node), bitvec_64_aux(num_node),
      estimate(num_node, -(width*2)), dout(num_node),
      gen(static_cast<uint64_t>(seed)), dst(0, 1), random(gen, dst) {

    for (int i = 0; i < num_node; i++) dout[i] = outlink[i].size();
    reverse = false;
}

void BitPropagation::init() {
    this->distance = 0;

    if (width == 32) {
        for (int i = 0; i < num_node; i++) {
            bitvec_32[i] = 0;
        }
    } else if (width == 64) {
        for (int i = 0; i < num_node; i++) {
            bitvec_64[i] = 0;
        }
    } else {
        // error processing
    }
}

void BitPropagation::reverseLinks() {
    reverse = true;
}

void BitPropagation::step() {
    distance++;

    std::cerr << "Current distance:" << distance << std::endl;

    if (width == 32) {
        bitvec_32_aux = std::vector<int>(num_node);
    } else if (width == 64) {
        bitvec_64_aux = std::vector<int64_t>(num_node);
    }

    std::cerr << "  Calculation step: ";

    if (width == 32) {
        for (int src = 0; src < num_node; src++) {
            if (bitvec_32[src] != 0) {
                for (int i = 0; i < dout[src]; i++) {
                    int next = outlink[src][i];
                    if (reverse) {
                        bitvec_32_aux[src] |= bitvec_32[next];
                    } else {
                        bitvec_32_aux[next] |= bitvec_32[src];
                    }
                }
            }
        }
    } else if (width == 64) {
        for (int src = 0; src < num_node; src++) {
            if (bitvec_64[src] != 0) {
                for (int i = 0; i < dout[src]; i++) {
                    int next = outlink[src][i];
                    if (reverse) {
                        bitvec_64_aux[src] |= bitvec_64[next];
                    } else {
                        bitvec_64_aux[next] |= bitvec_64[src];
                    }
                }
            }
        }
    }

    if (width == 32) {
        if (distance > 1) {
            for (int i = 0; i < num_node; i++) {
                bitvec_32[i] |= bitvec_32_aux[i];
            }
        } else {
            for (int i = 0; i < num_node; i++) {
                bitvec_32[i] = bitvec_32_aux[i];
            }
        }
    } else if (width == 64) {
        if (distance > 1) {
            for (int i = 0; i < num_node; i++) {
                bitvec_64[i] |= bitvec_64_aux[i];
            }
        } else {
            for (int i = 0; i < num_node; i++) {
                bitvec_64[i] = bitvec_64_aux[i];
            }
        }
    }

    std::cerr << "done." << std::endl;
}

void BitPropagation::estimateAll() {
    std::cerr << "  Doing estimation: ";
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
            estimate[i] = estimateSupporters(ones);
        }
    }

    std::cerr << "done." << std::endl;
}
