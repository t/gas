bin_PROGRAMS = gas nhash gr2edge
gas_SOURCES  = src/gas.cpp \
               src/db.cpp \
               src/edge.cpp \
               src/seq.cpp \
               src/adjlist.cpp \
               src/adjlist_fixed.cpp \
               src/adjlist_ielias.cpp \
               src/spp.cpp \
               src/pagerank.cpp \
               src/hits.cpp \
               src/modularityq.cpp \
               src/arg.cpp \
               src/run_bit_propagation_org.cpp \
               src/supporters.cpp \
               src/HomogeneousBitPropagation.cpp \
               src/EpsilonestimateAdaptableBitPropagation.cpp \
               src/ANFBitPropagation.cpp \
               src/AdaptableBitPropagation.cpp \
               src/BitPropagation.cpp
gas_LDADD=-lboost_filesystem -lpthread -lgflags -lglog
nhash_SOURCES=src/nhash.cpp
nhash_LDADD=-lboost_program_options
gr2edge_SOURCES=src/gr2edge.cpp
gr2edge_LDADD=-lboost_program_options

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

