#ifndef ADJLIST_H_
#define ADJLIST_H_

#include <map>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/shared_ptr.hpp>

#include "btree.h"
#include "edge.h"
#include "seq.h"

class AbstractAdjlist{
public:
  class iterator : public boost::iterator_facade<iterator, uint64_t, boost::forward_traversal_tag>{
  public:
    virtual bool equal(iterator * other) = 0;
    virtual void increment()                        = 0;
    virtual uint64_t& dereference() const           = 0;
  };
  AbstractAdjlist(bool forward);
  virtual int create() = 0;
  virtual int open()   = 0;
  virtual int close()  = 0;
  virtual boost::shared_ptr<iterator> begin_p() = 0;
  virtual boost::shared_ptr<iterator> end_p()   = 0;
protected:
  bool forward_;
};

class Adjlist{
public: 
  class iterator : public boost::iterator_facade<iterator, uint64_t, boost::forward_traversal_tag> {
  public:
    iterator();
    iterator(boost::shared_ptr<AbstractAdjlist::iterator> subject);
    bool equal(Adjlist::iterator const& other) const;
    void increment();
    uint64_t& dereference() const;
  private:
    boost::shared_ptr<AbstractAdjlist::iterator> subject_;
  };
  Adjlist(bool forward);
  ~Adjlist();
  int create();
  int open();
  int close();
  iterator begin();
  iterator end();
  AbstractAdjlist * subject_;
private:
  bool              forward_;
};

Adjlist adjlist_create(bool is_foward);
int adjlist();

#endif

