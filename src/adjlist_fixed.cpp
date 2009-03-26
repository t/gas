#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <ctime>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "adjlist_fixed.h"
#include "seq.h"
#include "db.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

AbstractAdjlist * adjlist_fixed_create(bool forward)
{
  LOG(INFO) << "adjlist_fixed_create: forward: " << forward;
  seq_create();

  AbstractAdjlist * ret;
  if(seq_32bit()){
    ret = new AdjlistFixed<uint32_t>(forward);
  }else{
    ret = new AdjlistFixed<uint64_t>(forward);
  }

  return ret;
}

