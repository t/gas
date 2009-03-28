#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <boost/lexical_cast.hpp>

#include "BitPropagation.hpp"
#include "ANFBitPropagation.hpp"
#include "HomogeneousBitPropagation.hpp"
#include "AdaptableBitPropagation.hpp"
#include "EpsilonestimateAdaptableBitPropagation.hpp"

using namespace std;
using namespace boost;

int run_bit_propagation_org() {
    const short Default_Width = 64;
    const int Default_Distance = 25;
//    const bool Default_Adaptive = false;
    const double Default_Adaptive_Epsilon_Multiplier = 2.0;
    const double Default_Adaptive_Epsilon_Max = 0.5;
    const double Default_Adaptive_Epsilon_Min_Factor = 1.0;
//    const bool Default_Adaptive_Use_Epsilonestimate = false;
    const double Default_Adaptive_Stop_Criteria = 0.99;
    const double Default_Epsilon = 1.0;
//    const bool Default_Reverse = false;
//    const bool Default_Use_Anf = false;
    const uint64_t Default_Seed = std::time(0);

    options_description opt("Options");
    opt.add_options()
        ("input-file",                     value<string>(),                                                        "input file name")
        ("width,w",                        value<short>()->default_value(Default_Width),                           "bit vector width 32 or 64")
        ("distance,d",                     value<int>()->default_value(Default_Distance),                          "distance to explore")
        ("epsilon,e",                      value<double>()->default_value(Default_Epsilon),                        "probability of initializing a bit as 1, in terms of 1/N")
        ("reverse,r",                                                                                              "reverse links, counting, out-neighbors")
        ("anf,f",                          value<short>(),                                                         "use ANF method for arg times iteration")
        ("seed,s",                         value<uint64_t>(),                                                      "seed of random number generator [default: std::time(0)]")
        ("adaptive,a",                                                                                             "adjust epsilon adaptively")
        ("adaptive-multiplier,m",          value<double>()->default_value(Default_Adaptive_Epsilon_Multiplier),    "factor for increasing epsilon")
        ("adaptive-maximum,x",             value<double>()->default_value(Default_Adaptive_Epsilon_Max),           "maximum epsilon")
        ("adaptive-minimum,n",             value<double>()->default_value(Default_Adaptive_Epsilon_Min_Factor),    "minimum epsilon factor")
        ("adaptive-use-epsilonestimate,t",                                                                         "use alternative estimation of 2/(M+1)e, disregarding bits")
        ("adaptive-stop-criteria,c",       value<double>()->default_value(Default_Adaptive_Stop_Criteria),         "fraction of nodes with good estimations required to stop")
        ("help,h",                                                                                                 "show help message")
        ;

    positional_options_description p;
    p.add("input-file", -1);

    variables_map vm;
    store(command_line_parser(argc, argv).options(opt).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
        cout << opt << endl;
        return 1;
    }
    if (!vm.count("input-file")) {
        cout << "no input graph file!" << endl;
        return 1;
    }
    
    // create graph as adjacency list
    adj_list graph = make_adj_list(input_file_name);
    cerr << "Number of nodes           : " << graph.size() << endl;

    string output_file_name = "WIDTH_" + lexical_cast<string>(width) + "_DIST_" + lexical_cast<string>(distance);
    cerr << "Number of bits            : " << width  << endl;
    cerr << "Distance                  : " << distance << endl;
    if (seed != Default_Seed) {
        output_file_name += "_SEED_" + lexical_cast<string>(seed);
    }
    cerr << "Random Seed               : " << seed << endl;

    double epsilon;
    double adaptive_epsilon_min = 0;
    if (adaptive) {
        output_file_name += "_ADAPTIVE";
        if (adaptive_stop_criteria != Default_Adaptive_Stop_Criteria) {
            output_file_name += "_STOPCRITERIA_" + lexical_cast<string>(adaptive_stop_criteria);
        }
        cerr << "Stop Criteria             : " << adaptive_stop_criteria << " of nodes with good estimations" << endl;

        if (adaptive_use_epsilonestimate) {
            output_file_name += "_EPSILONESTIMATE";
            cerr << "Using eplison-only estimate " << endl;
        }

        if (adaptive_epsilon_min_factor != Default_Adaptive_Epsilon_Min_Factor) {
            output_file_name += "_EPSILONMINIMUM_" + lexical_cast<string>(adaptive_epsilon_min_factor);
        }
        adaptive_epsilon_min = adaptive_epsilon_min_factor / graph.size();

        if (adaptive_epsilon_max != Default_Adaptive_Epsilon_Max) {
            output_file_name += "_EPSILONMAX_" + lexical_cast<string>(adaptive_epsilon_max);
        }

        if (adaptive_multiplier != Default_Adaptive_Epsilon_Multiplier) {
            output_file_name += "_EPSILONMULTIPLIER_" + lexical_cast<string>(adaptive_multiplier);
        }
        cerr << "ADAPTIVE strategy enabled : multiplier=" << adaptive_multiplier <<  " maximum=" << adaptive_epsilon_max << " minimum=" << adaptive_epsilon_min << endl;

        if (adaptive_multiplier > 1) {
            epsilon = adaptive_epsilon_min;
        } else {
            epsilon = adaptive_epsilon_max;
        }
    } else {
        if (use_anf) {
            output_file_name += "_ANF_" + lexical_cast<string>(anf_iterations);
            cerr << "Using ANF iterations      : " << anf_iterations << endl;
        }
        if (epsilon_factor != Default_Epsilon) {
            output_file_name += "_EPSILON_" + lexical_cast<string>(epsilon_factor);
            cerr << "Epsilon factor provided   : " << epsilon_factor << endl;
        }
        epsilon = epsilon_factor / graph.size();
    }
    
    BitPropagation* algorithm = NULL;
    if (adaptive) {
        if (adaptive_use_epsilonestimate) {
            algorithm = new EpsilonestimateAdaptableBitPropagation(graph, width, seed);
        } else {
            algorithm = new AdaptableBitPropagation(graph, width, seed);
        }
    } else {
        if (use_anf) {
            algorithm = new ANFBitPropagation(graph, width, seed);
        } else {
            algorithm = new HomogeneousBitPropagation(graph, width, seed);
        }
    }
    
    if (reverse) {
        output_file_name += "_REVERSE";
        algorithm->reverseLinks();
    }
    
    if (use_anf) {
        dynamic_cast<ANFBitPropagation*>(algorithm)->init();
    } else {
        dynamic_cast<HomogeneousBitPropagation*>(algorithm)->init(epsilon);
    }
    
    int estimated_ok = 0;
    bool done = false;
    int runs = 0;
    
    while (!done) {
        cerr << "Iteration " << (runs+1) << ", target distance " << distance << endl;
        for (int it = 1; it <= distance; it++) {
            cerr << endl;
            algorithm->step();
        }
        cerr << endl;
        
        if (adaptive) {
            if (adaptive_multiplier > 1) {
                estimated_ok = dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateIfAbove(static_cast<int>(0.629629 * width), adaptive_multiplier);
            } else {
                estimated_ok = dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateIfBelow(static_cast<int>(0.629629 * width), adaptive_multiplier);
            }
            
            cerr << "Fraction of nodes with good estimations: " << estimated_ok << "/" << algorithm->get_num_node() << "=" << static_cast<double>(estimated_ok) / algorithm->get_num_node() << endl;
            
            if (estimated_ok >= algorithm->get_num_node() * adaptive_stop_criteria) {
                cerr << "Fraction of nodes with good estimates (" << adaptive_stop_criteria << ") reached for adaptive strategy" << endl;
                done = true;
            } else if (epsilon * adaptive_multiplier < adaptive_epsilon_min) {
                cerr << "Minimum (" << adaptive_epsilon_min << ") reached for adaptive strategy" << endl;
                done = true;
            } else if (epsilon * adaptive_multiplier > adaptive_epsilon_max) {
                cerr << "Maximum (" << adaptive_epsilon_max << ") reached for adaptive strategy" << endl;
                done = true;
            } else {
                cerr << '\n';
                epsilon *= adaptive_multiplier;
                cerr << "Adaptive strategy enabled, continuing eith epsilon=" << epsilon << endl;
                dynamic_cast<AdaptableBitPropagation*>(algorithm)->init(epsilon);
            }
            
            runs++;
        } else {
            if (use_anf) {
                dynamic_cast<ANFBitPropagation*>(algorithm)->updateAverageFirstZero();
                runs++;
                if (runs == anf_iterations) {
                    done = true;
                } else {
                    dynamic_cast<ANFBitPropagation*>(algorithm)->init();
                }
            } else {
                done = true;
            }
        }
    }
    
    cerr << "Estimating supporters for all nodes" << endl;

    if (adaptive) {
        dynamic_cast<AdaptableBitPropagation*>(algorithm)->estimateAll(adaptive_multiplier);
    } else {
        if (use_anf) {
            dynamic_cast<ANFBitPropagation*>(algorithm)->estimateAll(anf_iterations);
        } else {
            algorithm->estimateAll();
        }
    }

    if (adaptive || use_anf) {
        cerr << "Done after " << runs << " runs" << endl;
    }

    output_file_name += ".csv";
    cerr << "Writing estimations to " << output_file_name << " : ";
    ofstream ofs(output_file_name.c_str());
    vector<double> estimate = algorithm->get_estimate();
    int num_node = algorithm->get_num_node();
    for (int i = 0; i < num_node; i++) {
        ofs << round(estimate[i]) << endl;
    }
    cerr << "done." << endl;

    return 0;
}
