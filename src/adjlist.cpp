#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/array.hpp>
#include <ctime>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "seq.h"
#include "ielias_vector.h"

using namespace std;
using namespace boost;
using namespace boost::filesystem;

template<typename T> int adjlist_create_t(const std::string& db_dir, bool is_forward, const std::string& hash)
{
  const std::string seq_file = db_dir + FILE_SEQUENCE + hash;
  const string adj_file = is_forward ? db_dir + FILE_ADJLIST_FORWARD + hash : db_dir + FILE_ADJLIST_BACKWARD;

  if(! exists(seq_file))
  {
      std::cout << "failed: not found " + seq_file << std::endl;
      return 0;
  }

  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);

  MmapVector<T> * adj = new MmapVector<T>(adj_file);
  adj->open(true);

  Btree<uint64_t, Edge> * btree;
  if(is_forward)
  {
    btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  }else{
    btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD,  false);
  }

  ForwardWalker<uint64_t, Edge> walker = btree->walkerBegin();

  time_t   start_time = time(0);
  uint64_t key = 0;
  while(! walker.is_end)
  {
    std::vector<uint64_t> links;
    while(! walker.is_end)
    {
      Edge e = walker.value();

      if(e.key == key)
      {
        links.push_back(e.to);
        walker.increment();
      }else{
        key = e.key;
        break;
      }
    }

    adj->push_back(links.size());

    for(typename std::vector<uint64_t>::iterator i = links.begin(); i < links.end(); i++)
    {
      adj->push_back(seq_get(seq, (*i)));
    }
  }

  std::cout << "*adj_size =" << adj->size() << std::endl;
  std::cout << " adj_create time : " << time(0) - start_time << std::endl;

  adj->close();
  delete adj;

  seq->close();
  delete seq;

  delete btree;
}

int adjlist_create(const std::string& db_dir,  bool is_forward, const std::string& hash)
{
  const string adj_file = is_forward ? db_dir + FILE_ADJLIST_FORWARD + hash : db_dir + FILE_ADJLIST_BACKWARD;
  const string seq_file = db_dir + FILE_SEQUENCE        + hash;

  if(boost::filesystem::exists(adj_file))
  {
    return 1;
  }

  seq_create(db_dir, hash);

  MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
  seq->open(false);
  size_t seq_size = seq->size();
  seq->close();
  delete seq;

  if(seq_size < 4294967296)
  {
    adjlist_create_t<uint32_t>(db_dir, is_forward, hash);
  }else{
    adjlist_create_t<uint64_t>(db_dir, is_forward, hash);
  }
}

int adjlist_ielias_create(const std::string& db_dir,  bool is_forward, const std::string& hash)
{
  const string adj_file = is_forward ? db_dir + FILE_ADJLIST_IELIAS_FORWARD + hash : db_dir + FILE_ADJLIST_IELIAS_BACKWARD;
  const string seq_file = db_dir + FILE_SEQUENCE + hash;

  if(boost::filesystem::exists(adj_file)) {
    return 1;
  }
  seq_create(db_dir, hash);

  MmapVector<uint64_t> * seq = new MmapVector<uint64_t>(seq_file);
  seq->open(false);

  iEliasVector<uint64_t> * adj = new iEliasVector<uint64_t>(adj_file);
  adj->open(true);

  Btree<uint64_t, Edge> * btree;
  if(is_forward)
  {
    btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_FORWARD,  false);
  }else{
    btree = new Btree<uint64_t, Edge>(db_dir + FILE_EDGE_BACKWARD,  false);
  }

  ForwardWalker<uint64_t, Edge> walker = btree->walkerBegin();

  time_t start_time = time(0);

  uint64_t key = 0;
  while(! walker.is_end)
  {
    cout << "." << endl;
    std::vector<uint64_t> links;
    while(! walker.is_end)
    {
      Edge e = walker.value();

      if(e.key == key)
      {
        links.push_back(e.to);
        walker.increment();
      }else{
        key = e.key;
        break;
      }
    }

    adj->push_back(links.size());

    uint64_t before = 0;
    for(vector<uint64_t>::iterator i = links.begin(); i < links.end(); i++)
    {
      uint64_t current = seq_get(seq, (*i));
      adj->push_back(current - before);
      before = current;
    }
  }

  std::cout << " ielias_adj_create time : " << time(0) - start_time << std::endl;

  adj->close();
  delete adj;

  seq->close();
  delete seq;

  delete btree;
}

int adjlist_ielias_test(const std::string& db_dir)
{
  const string test_file = db_dir + "elias_test";

  iEliasVector<uint64_t> * ielias = new iEliasVector<uint64_t>(test_file);
  ielias->open(true);

  for(int i = 0; i < 100000; i++){
    cout << "i = " << i << endl;
    ielias->push_back(i);

    int before = -1;
    for(iEliasVector<uint64_t>::iterator j = ielias->begin(); j != ielias->end(); j++){
      //cout << " j = " <<  (*j) << endl;
      assert( (*j) == before + 1 );
      before = (*j);
    }
    assert( before == i );
  }
  ielias->close();
  delete ielias;
}


