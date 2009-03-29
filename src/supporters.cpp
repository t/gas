#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <google/gflags.h>
#include <glog/logging.h>
#include "run_bit_propagation_org.h"

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"
#include "hits.h"
#include "db.h"
#include "arg.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

const string supporters_path()
{
  string ret = "";//string(FILE_SUPPORTERS);
       //      + "_eps_"   + lexical_cast<string>(FLAGS_eps)
       //      + "_maxi_"  + lexical_cast<string>(FLAGS_max_iteration);

  return db_path(ret);
}

int supporters_calc()
{
  const vector<string> argvs = get_argvs();
  const string supporters_file = supporters_path();

  run_bit_propagation_org();

  return 1;
}

int supporters()
{
  const vector<string> argvs = get_argvs();
  assert(argvs.size() >= 2);

  supporters_calc();

  return 1;
}

