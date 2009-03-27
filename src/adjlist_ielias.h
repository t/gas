#ifndef ADJLIST_IELIAS_H_
#define ADJLIST_IELIAS_H_

#define FILE_ADJLIST_IELIAS  "tmp_adjlist_ielias"

#include <map>
#include <string>

#include "btree.h"
#include "edge.h"
#include "seq.h"
#include "adjlist.h"
#include "mmap_vector.h"
#include "ielias_vector.h"
#include "db.h"

template <typename T>
class AdjlistIEliasIterator : public AbstractAdjlist::iterator{
public:
  typename iEliasVector<uint64_t>::iterator itr_;

  // begin = true is begin() / false is end()
  AdjlistIEliasIterator(iEliasVector<uint64_t> * parent, bool begin)
  {
    if(begin){
      itr_ = parent->begin();
      state_ = 0;
    }else{
      itr_ = parent->end();
      state_ = 2;
    }
    edge_count_ = 0;
    edge_i_     = 0;
  }
  bool equal(AbstractAdjlist::iterator * other) {
    AdjlistIEliasIterator<T> * other_p = dynamic_cast< AdjlistIEliasIterator<T> * >(other);
    assert(other_p != 0);
    return (itr_ == other_p->itr_);
  }
  void increment() {
    if(state_ == 0){
      state_  = 1;
    }else if(state_ == 1){
      edge_count_ = (*itr_);
      edge_i_     = 0;
      state_      = 2;
    }else if(state_ == 2){
      edge_i_++;
    }
    if(state_ == 2 && edge_i_ >= edge_count_){
      state_ = 1;
    }
    itr_++;
  }
  uint64_t& dereference() const {
    uint64_t next = (*itr_);

    if(state_ == 0 || state_ == 1){
      current_ = next;
    }else if(state_ == 2){
      if(edge_i_ == 0){ 
        current_ = next;
      }else{
        current_ += next;
      }
    }
    return current_;
  }
private:
  mutable uint64_t current_;
  int state_;
  int edge_count_;
  int edge_i_;
};

template<typename T>
class AdjlistIElias : public AbstractAdjlist{
public:
  AdjlistIElias(bool forward) : AbstractAdjlist(forward){
  }
  int create()
  {
    LOG(INFO) << "AdjlistIElias::create"; 

    const std::string seq_file = db_path(FILE_SEQUENCE);
    seq_create();

    const std::string adj_file = this->adjlist_ielias_path(forward_);
    if(boost::filesystem::exists(adj_file))
      return 1;

    LOG(INFO) << "AdjlistIElias create start";
 
    MmapVector< uint64_t > * seq = new MmapVector< uint64_t >(seq_file);
    seq->open(false);

    iEliasVector<uint64_t> * adj = new iEliasVector<uint64_t>(adj_file);
    adj->open(true);
    uint64_t size = seq->size();
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
      uint64_t before = 0;
      for(typename std::vector<uint64_t>::iterator j = links.begin(); j != links.end(); j++){
        uint64_t current = seq_get(seq, (*j));
        assert( current - before >= 0 );
        adj->push_back(current - before);
        before = current;
      }
    }

    adj->close();
    delete adj;

    seq->close();
    delete seq;

    delete btree;
  }

  int open()
  {
    LOG(INFO) << "AdjlistIElias::open";

    const std::string adj_file = this->adjlist_ielias_path(forward_);
    adj_ = new iEliasVector<uint64_t>(adj_file);
    adj_->open(false);
  }

  int close()
  {
    LOG(INFO) << "AdjlistIElias::close";

    adj_->close();
    delete adj_;
  }

  boost::shared_ptr<AbstractAdjlist::iterator> begin_p()
  {
    AbstractAdjlist::iterator * ret = new AdjlistIEliasIterator<uint64_t>(adj_, true);
    return boost::shared_ptr<AbstractAdjlist::iterator>(ret);
  }

  boost::shared_ptr<AbstractAdjlist::iterator> end_p()
  {
    AbstractAdjlist::iterator * ret = new AdjlistIEliasIterator<uint64_t>(adj_, false);
    return boost::shared_ptr<AbstractAdjlist::iterator>(ret);
  }

private:
  iEliasVector<uint64_t> * adj_;
  const std::string adjlist_ielias_path(bool forward)
  {
    std::string ret = std::string(FILE_ADJLIST_IELIAS)
             + "_" + (forward ? "forward" : "reverse");

    return db_path(ret);
  }
};

AbstractAdjlist * adjlist_ielias_create(bool foward);

#endif
