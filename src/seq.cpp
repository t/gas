#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <stdint.h>

#include "btree.h"
#include "edge.h"
#include "seq.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

int seq_create(const string& db_dir, const string& hash)
{
  const string seq_file = db_dir + FILE_SEQUENCE + hash;

  if(boost::filesystem::exists(seq_file))
  {
    return 1;
  }

  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(true);

  Btree<uint64_t, Edge> * f_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  Btree<uint64_t, Edge> * b_btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD, false);

  ForwardWalker<uint64_t, Edge> f_walker = f_btree->walkerBegin();
  ForwardWalker<uint64_t, Edge> b_walker = b_btree->walkerBegin();

  time_t start_time = time(0);

  uint64_t min_key = 0;
  while(true)
  {
    uint64_t f_max_key = 0;
    uint64_t b_max_key = 0;
    while(! f_walker.is_end)
    {
      Edge e = f_walker.value();
      if(seq->size() == 0 || e.key > min_key)
      {
        f_max_key = e.key;
        break;
      }
      f_walker.increment();
    }

    while(! b_walker.is_end)
    {
      Edge e = b_walker.value();
      if(seq->size() == 0 || e.key > min_key)
      {
        b_max_key = e.key;
        break;
      }
      b_walker.increment();
    }

    if( (! f_walker.is_end) && (! b_walker.is_end))
    {
      min_key = min(f_max_key, b_max_key);
    }else if(! f_walker.is_end)
    {
      min_key = f_max_key;
    }else if(! b_walker.is_end)
    {
      min_key = b_max_key;
    }else{
      break;
    }

    //cout << "key = " << min_key << endl;
    seq->push_back(min_key);
  }

  cout << "*seq_size =" << seq->size() << endl;
  cout << " seq_create time : " << time(0) - start_time << endl; 

  seq->close();
  delete seq;

  delete f_btree;
  delete b_btree;
}

uint64_t seq_get(MmapVector< uint64_t > * seq, uint64_t hash)
{
  uint64_t * a = seq->at(0);

  uint64_t * res = lower_bound(a, a + seq->size(), hash);

  assert( res != a + seq->size() );
  
  if ( (* res)  != hash) {
    cout << "find = " << (*res) << endl;
    cout << "hash = " << hash << endl;
  }
 
  assert( (* res) == hash );
  uint64_t ret = res - a;  

  return ret;
}

uint64_t seq_get_random(MmapVector<uint64_t> * seq){
  uint64_t r = rand() % seq->size();
  return (* seq->at(r) );
}

size_t seq_size(const string& db_dir, const string& hash)
{
  const string seq_file = db_dir + FILE_SEQUENCE + hash;

  assert(boost::filesystem::exists(seq_file));

  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);
  size_t seq_size = seq->size();
  seq->close();
  delete seq;

  return seq_size;
}

bool seq_32bit(const string& db_dir, const string& hash)
{
  size_t ret = seq_size(db_dir, hash);

  return (ret < 4294967296);
}

 
