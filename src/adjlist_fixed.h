#ifndef ADJLIST_FIXED_H_
#define ADJLIST_FIXED_H_

#define FILE_ADJLIST_FIXED  "tmp_adjlist_fixed"

#include <map>
#include <string>

#include "btree.h"
#include "edge.h"
#include "seq.h"
#include "adjlist.h"
#include "mmap_vector.h"
#include "db.h"

template <typename T>
class AdjlistFixedIterator : public AbstractAdjlist::iterator{
public:
  typename MmapVector<T>::iterator itr_;
  AdjlistFixedIterator(typename MmapVector<T>::iterator itr)
  {
    itr_ = itr;
  }
  bool equal(AbstractAdjlist::iterator * other) {
    AdjlistFixedIterator<T> * other_p = dynamic_cast< AdjlistFixedIterator<T> * >(other);
    assert(other_p != 0);
    return (itr_ == other_p->itr_);
  }
  void increment() {
    itr_++;
  }
  uint64_t& dereference() const {
    current_ = (*itr_);
    return current_;
  }
private:
  mutable uint64_t current_;
};

template<typename T>
class AdjlistFixed : public AbstractAdjlist{
public:
  AdjlistFixed(bool forward) : AbstractAdjlist(forward){
  }
  int create()
  {
    LOG(INFO) << "AdjlistFixed::create"; 

    const std::string seq_file = db_path(FILE_SEQUENCE);
    const std::string adj_file = adjlist_fixed_path(seq_32bit(), forward_);

    seq_create();

    if(boost::filesystem::exists(adj_file))
      return 1;
 
    MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
    seq->open(false);

    MmapVector<T> * adj = new MmapVector<T>(adj_file);
    adj->open(true);
    T size = seq->size();
    adj->push_back(size);
 
    Btree<uint64_t, Edge> * btree;
    if(forward_){
      btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_FORWARD), false);
    }else{
      btree = new Btree<uint64_t, Edge>(db_path(FILE_EDGE_BACKWARD),false);
    }

    uint64_t key = 0;
    Btree<uint64_t, Edge>::iterator end = btree->end();
    for(Btree<uint64_t, Edge>::iterator i = btree->begin(); i != end; i++){
      std::vector<uint64_t> links;
      while(i != end)
      {
        if(i->key == key) {
          links.push_back(i->to);
          i++;
        }else{
          key = i->key;
          break;
        }
      }
      adj->push_back(links.size());
      for(typename std::vector<uint64_t>::iterator j = links.begin(); j != links.end(); j++){
        adj->push_back(seq_get(seq, (*j)));
      }
    }

    std::cout << "*adj_size =" << adj->size() << std::endl;

    adj->close();
    delete adj;

    seq->close();
    delete seq;

    delete btree;
  }

  int open()
  {
    LOG(INFO) << "AdjlistFixed::open";

    const std::string adj_file = adjlist_fixed_path(seq_32bit(), forward_);
    adj_ = new MmapVector<T>(adj_file);
    adj_->open(false);
  }

  int close()
  {
    LOG(INFO) << "AdjlistFixed::close";

    adj_->close();
    delete adj_;
  }

  boost::shared_ptr<AbstractAdjlist::iterator> begin_p()
  {
    AbstractAdjlist::iterator * ret = new AdjlistFixedIterator<T>(adj_->begin());
    return boost::shared_ptr<AbstractAdjlist::iterator>(ret);
  }

  boost::shared_ptr<AbstractAdjlist::iterator> end_p()
  {
    AbstractAdjlist::iterator * ret = new AdjlistFixedIterator<T>(adj_->end());
    return boost::shared_ptr<AbstractAdjlist::iterator>(ret);
  }

private:
  MmapVector<T> * adj_;
  const std::string adjlist_fixed_path(bool is32bit, bool forward)
  {
    std::string ret = std::string(FILE_ADJLIST_FIXED)
             + "_" + (is32bit ? "32" : "64")
             + "_" + (forward ? "forward" : "reverse");

    return db_path(ret);
  }
};

AbstractAdjlist * adjlist_fixed_create(bool foward);

#endif
