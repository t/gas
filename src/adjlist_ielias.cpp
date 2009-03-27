#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <ctime>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "adjlist_ielias.h"
#include "seq.h"
#include "db.h"

AbstractAdjlist * adjlist_ielias_create(bool forward)
{
  LOG(INFO) << "adjlist_ielias_create: forward: " << forward;
  seq_create();

  AbstractAdjlist * ret;
  ret = new AdjlistIElias<uint64_t>(forward);

  return ret;
}

