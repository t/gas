#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>

#include "btree.h"
#include "edge.h"
#include "dijkstra.h"
#include "seq.h"

using namespace std;
using namespace boost;

int alt_experiment(const std::string& db_dir)
{
  const string seq_file = db_dir + FILE_SEQUENCE + "testhash";

  MmapVector<uint64_t> * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  int total = 0;
  int c = 0;
  for(int i = 0; i < 100; i++)
  {
    uint64_t source = seq_get_random(seq);
    uint64_t target = seq_get_random(seq);
    cout << "test source = " << source << " target = " << target << endl;
    //bidirectional_dijkstra(db_dir, source, target);
    int result = bidirectional_alt(db_dir, source, target, "maxcover", 16);
    if(result > 0)
    {
      total += result;
      c++;
    } 
  }
  cout << "avg = " << total / c << endl;

  seq->close();
  delete seq;
}



