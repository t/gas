#include <boost/iterator_adaptors.hpp>
#include <boost/shared_ptr.hpp>

#include "btree.h"
#include "edge.h"
#include "adjlist.h"
#include "adjlist_fixed.h"
#include "seq.h"
#include "db.h"

using namespace std;

AbstractAdjlist::AbstractAdjlist(bool forward)
{
  forward_ = forward;
}

// ----------

Adjlist::iterator::iterator(boost::shared_ptr<AbstractAdjlist::iterator> subject)
{
  subject_ = subject;
}

void Adjlist::iterator::increment()
{
  subject_->increment();
}

bool Adjlist::iterator::equal(Adjlist::iterator const& other) const
{
  return subject_->equal(other.subject_.get());
}

uint64_t& Adjlist::iterator::dereference() const
{
  return subject_->dereference();
}

// ---------

Adjlist::Adjlist(bool forward){
  forward_ = forward;
  subject_ = NULL;
}

Adjlist::~Adjlist(){
  if(subject_ != NULL) delete subject_;
}

int Adjlist::create(){
  LOG(INFO) << "Adjlist::create()";
  assert(subject_ == NULL);
  subject_ = adjlist_fixed_create(forward_); 
  subject_->create();
  subject_->open();
}

Adjlist::iterator Adjlist::begin()
{
  return Adjlist::iterator(subject_->begin_p());
}

Adjlist::iterator Adjlist::end()
{
  return Adjlist::iterator(subject_->end_p());
}

// ---------

Adjlist adjlist_create(bool forward)
{
  Adjlist adj(forward);
  adj.create();
  return adj;
}

int adjlist_all()
{
  Adjlist adj = adjlist_create(true);

  Adjlist::iterator end = adj.end();
  cout << "result:" << endl;
  cout << "  adjlist:" << endl;
  size_t count = 0;
  for(Adjlist::iterator i = adj.begin(); i != end; i++){
    cout << "    " << count << ": " << (*i) << endl;
    count++;
  }
  cout << "  adjlist_count: " << count << endl;
}

int adjlist()
{ 
  const vector<string> argvs = google::GetArgvs();
  assert(argvs.size() >= 3);
  
  if(argvs[2] == "all"){
    return adjlist_all(); 
  }

  return 0;
}

