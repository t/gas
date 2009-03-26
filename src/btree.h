#ifndef BTREE_H_
#define BTREE_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/iterator_adaptors.hpp>
#include <google/gflags.h>
#include <glog/logging.h>

#include "mmap_vector.h"

template <typename KEY, typename VALUE>
struct BtreeNode
{
public:
  static const int B_BLOCK_SIZE = 4096;
  static const int B_STEM_POINT = (B_BLOCK_SIZE - sizeof(uint64_t) * 1) / ( sizeof(KEY) + sizeof(uint64_t));
  static const int B_STEM_CHILD = B_STEM_POINT + 1;
  static const int B_STEM_DUMMY = ( B_BLOCK_SIZE - (sizeof(KEY) * B_STEM_POINT + sizeof(uint64_t) * B_STEM_CHILD) / sizeof(uint8_t));
  //static const int B_BLOCK_SIZE = sizeof(KEY) * B_STEM_POINT + sizeof(uint64_t) * B_STEM_CHILD;
  static const int B_LEAF_VALUE = B_BLOCK_SIZE / sizeof(VALUE); 
  static const int B_LEAF_DUMMY = B_BLOCK_SIZE % sizeof(VALUE);
 
  bool is_leaf;
  int  count;
  union {
    struct{
      boost::array<KEY,      B_STEM_POINT> points;
      boost::array<uint64_t, B_STEM_CHILD> children;
      boost::array<uint8_t,  B_STEM_DUMMY> dummy;
     } stem;
    struct{
      boost::array<VALUE,    B_LEAF_VALUE> points;
      //boost::array<uint8_t, B_LEAF_DUMMY> dummy;
    } leaf;
  };

  BtreeNode(bool is_leaf_)
  {
    is_leaf = is_leaf_;
    count   = 0;
    for(int i = 0; i < B_STEM_POINT; i++)
      stem.points[i] = 0;
    for(int i = 0; i < B_STEM_CHILD; i++)
      stem.children[i] = 0;
  }
};

template <typename KEY, typename VALUE>
class Btree;

template <typename KEY, typename VALUE>
class ForwardWalker
{
public:
  Btree<KEY, VALUE> *  btree;
  std::stack<uint64_t> stack;
  int                  pos;
  bool                 is_begin;
  bool                 is_end;

  ForwardWalker<KEY, VALUE>(){}

  ForwardWalker<KEY, VALUE>(Btree<KEY, VALUE> * btree, std::stack<uint64_t> stack, const int pos, const bool is_begin = false, const bool is_end = false )
   : btree(btree), stack(stack), pos(pos), is_begin(is_begin), is_end(is_end) {}

  VALUE value()
  {
    return btree->walkerValue(this);
  }

  void increment()
  {
    return btree->walkerIncrement(this);
  }
};

template <typename KEY, typename VALUE>
class Btree 
{
  protected:
  MmapVector< BtreeNode<KEY, VALUE> > * _tree;
  int                                   _height;

  int low_in_stem(KEY s, uint64_t node_id)
  {

    BtreeNode<KEY, VALUE> * node = _tree->at(node_id);

    typename boost::array<KEY, BtreeNode<KEY, VALUE>::B_STEM_POINT>::iterator p
      = lower_bound(node->stem.points.begin(), 
                    node->stem.points.begin() + node->count,
                    s
                   );
    return p - node->stem.points.begin();
  }

  void insert_nonfull( const VALUE& e, uint64_t node_id, int kd)
  {

    BtreeNode<KEY, VALUE> * node;
    uint64_t next;
    int64_t * root = _tree->header(0);
    if(kd != 0 && node_id == (* root)) return;
    node = _tree->at(node_id);

    if(! node->is_leaf)
    {		
      int move = this->low_in_stem(e.key, node_id);
 
      next = node->stem.children[move];
      if(is_full(next))
      {
        //   cout << "stem is full" << endl;
        split_child(node_id, move, next);
        insert_nonfull(e, node_id, kd);
      }else{
        insert_nonfull(e, next, kd + 1);
      }
    }else{
 
      int lower = node->count;

      //  cout << "insert_nonfull (leaf) : node_count " << node->count << endl;
      for(int i = 0; i < node->count; i++)
      {
        VALUE item = node->leaf.points[i];
        KEY   source = item.key;
        if( source > e.key)
        {
          lower = i;
          break;
        }
      }

      for(int i = 0; i < node->count; i++)
      {
        int move_from = lower + (node->count - 1 - i);
        int move_to   = move_from + 1;
        if(move_to < BtreeNode<KEY, VALUE>::B_LEAF_VALUE){
          //cout << " - " << move_to << "=" << move_from << endl;
          node->leaf.points[move_to] = node->leaf.points[move_from];
        }
      }
	
      node->leaf.points[lower] = e;
      node->count += 1;
    }
  }
                      
  int find_sub(std::vector<VALUE> * result, KEY s, uint64_t node_id, int kd)
  {
    BtreeNode<KEY, VALUE> * node;
    uint64_t   next;
    int64_t * r = _tree->header(0);

    if(kd != 0 && node_id == (* r)) return 0;
    node = _tree->at(node_id);
      
    if(! node->is_leaf)
    {
      int move = this->low_in_stem(s, node_id);
                                       
      //cout << "  :move =" << node->count << endl;
      for(int i = move; i <= node->count; i++)
      {
        int res = 0;
        next = node->stem.children[i];
        res = find_sub(result, s, next, kd + 1);
        if(node->stem.points[i] != s)
        {
          break;
        }
      }
    }else{
      for(int i = 0; i < node->count; i++)
      {
        // cout << "finding... : " << i << endl;
        VALUE item = node->leaf.points[i];
        KEY source = item.key;
        if(source > s)
        { 
          break;
        }else if (source == s){
          //std::cout << "find : " << node_id << " - " << i << std::endl;
          result->push_back(item);
        }
      }
    }
  }

  int remove_sub(const VALUE& e, uint64_t node_id, int kd)
  {
    BtreeNode<KEY, VALUE> * node;
    uint64_t   next;
    int64_t * r = _tree->header(0);

    if(kd != 0 && node_id == (* r)) return 0;
    node = _tree->at(node_id);

    if(! node->is_leaf)
    {
      int move = this->low_in_stem(e.key, node_id);

      //cout << "  :move =" << node->count << endl;
      for(int i = move; i <= node->count; i++)
      {
        int res = 0;
        next = node->stem.children[i];
        remove_sub(e, next, kd + 1);
        if(node->stem.points[i] != e.key)
        {
          break;
        }
      }
    }else{
      // remove
      int delete_start = node->count;
      int delete_end   = node->count;
      for(int i = 0; i < node->count; i++)
      {
        // cout << "finding... : " << i << endl;
        VALUE item = node->leaf.points[i];
        KEY source = item.key;
        if(source > e.key)
        {
          delete_end = i;
          break;
        }else if (item == e){
          if(i < delete_start) delete_start = i; 
        }
      }
      int delete_count = delete_end - delete_start;
      for(int i = delete_end; i < node->count; i++)
      {
        node->leaf.points[i - delete_count] = node->leaf.points[i];
      }
      node->count -= delete_count;
    }
  }

  void write_allkeys_sub(MmapVector<KEY> * result, uint64_t node_id, int kd, KEY& max_key)
  {
    BtreeNode<KEY, VALUE> * node;
    uint64_t   next;
    int64_t * r = _tree->header(0);

    if(kd != 0 && node_id == (* r)) return ;
    node = _tree->at(node_id);

    if(! node->is_leaf)
    {
      for(int i = 0; i <= node->count; i++)
      {
        next = node->stem.children[i];
        _tree->will_need(next);
      }

      for(int i = 0; i <= node->count; i++)
      {
        next = node->stem.children[i];
        write_allkeys_sub(result, next, kd + 1, max_key);
      }
    }else{
      for(int i = 0; i < node->count; i++)
      {
        VALUE item = node->leaf.points[i];
        if( result->size() == 0 || item.key > max_key) 
        {
          result->push_back(item.key);
          max_key = item.key;
        }

        result->push_back(item.to);
      }
    }
  }

  bool is_full(uint64_t node_id)
  {
    BtreeNode<KEY, VALUE> * n;
    n = _tree->at(node_id);
    if(n->is_leaf)
    {
      return (n->count >= BtreeNode<KEY, VALUE>::B_LEAF_VALUE - 2) ;
    }else{
      return (n->count >= BtreeNode<KEY, VALUE>::B_STEM_POINT - 1);
    }
  }			
			
  void split_child(uint64_t parent_id, int parent_index, uint64_t split_id)
  {
    BtreeNode<KEY, VALUE> * split;
    BtreeNode<KEY, VALUE> * new_node;
    BtreeNode<KEY, VALUE> * parent;
    uint64_t    new_node_id;
    int    point_min;
    int    point_max;

    // create a new node

    split = _tree->at(split_id);

    _tree->push_back(BtreeNode<KEY, VALUE>(split->is_leaf));
    new_node    = _tree->back();
    new_node_id = _tree->size() - 1;

    // split the node

    split = _tree->at(split_id);

    point_min = split->count / 2;
    point_max = split->count;

    if(split->is_leaf)
    {
      new_node->count = point_max - point_min;
    }else{
      new_node->count = point_max - point_min - 1;
    }
                           
    split->count    = point_min;

    for(unsigned int i = 0; i < new_node->count; i++)
    {
      if(split->is_leaf)
      {
        new_node->leaf.points[i] = split->leaf.points[point_min + i];
      }else{
        new_node->stem.points[i] = split->stem.points[point_min + i + 1];
      }
    }

    if(! split->is_leaf)
    {
      for(unsigned int i = 0; i < new_node->count + 1; i++)
        new_node->stem.children[i] = split->stem.children[point_min + i + 1];
    }

    // update parent

    parent = _tree->at(parent_id);

    for(int i = parent->count; i > parent_index; i--)
    {
      parent->stem.points[i]       = parent->stem.points[i - 1];
      parent->stem.children[i + 1] = parent->stem.children[i];
    }

    KEY p;
    if(split->is_leaf)
    {
      p = split->leaf.points[point_min].key;
    }else{
      p = split->stem.points[point_min];
    }
    parent->stem.points[parent_index] = p;
    parent->stem.children[parent_index + 1] = new_node_id;
    parent->count++;
  }

public:
  Btree(std::string filename, bool init)
  {
    LOG(INFO) << "btree is opening [" << filename << "].";

    _tree = new MmapVector< BtreeNode<KEY, VALUE> >(filename);
    _tree->open(init);

    int64_t * r = _tree->header(0);
    if(init)
    {
      BtreeNode<KEY, VALUE> root(true);
      _tree->push_back(root);
      _height = 1;
      r = _tree->header(0);
      (* r) = 0;
    }
  }

  ~Btree()
  {
    _tree->close();
    delete _tree;
  }

  std::vector<VALUE> find(KEY s)
  {
    std::vector<VALUE> result;
    int64_t * r = _tree->header(0);

    find_sub(& result, s, (* r), 0);
 
    return result;
  }

  void remove(const VALUE& e)
  {
    int64_t * r = _tree->header(0);

    remove_sub(e, (* r), 0);
  }

  void insert(const VALUE& e)
  {
    int64_t * r = _tree->header(0);
    if(is_full((* r)))
    {
      _tree->push_back(BtreeNode<KEY, VALUE>(false));
      BtreeNode<KEY, VALUE> * s = _tree->back();
      uint64_t  s_id = _tree->size() - 1;         

      r = _tree->header(0);
      s->stem.children[0] = (* r);
      split_child(s_id, 0, (* r));

      r = _tree->header(0);
      (* r) = s_id;
      _height ++;
    }

    insert_nonfull(e, (* r), 0);
  } 
  
  void write_allkeys(MmapVector<KEY> * result)
  {
    int64_t * r = _tree->header(0);
    KEY max_key = 0;

    write_allkeys_sub(result, (* r), 0, max_key);
  }

  int height() const
  {
    return _height;
  }

  uint64_t size() const
  {
    return sizeof(BtreeNode<KEY, VALUE>) * _tree->size();
  }

  VALUE walkerValue( ForwardWalker<KEY, VALUE> * walker)
  {
    uint64_t node_id = walker->stack.top();
    BtreeNode<KEY, VALUE> * node = _tree->at(node_id);

    assert(node->is_leaf);
    assert(walker->pos < node->count);

    return node->leaf.points[walker->pos];
  }

  void walkerIncrement(ForwardWalker<KEY, VALUE> * walker)
  {
    bool started_node = ! walker->is_begin;
    walker->is_begin = false;
  
    while(walker->stack.size() > 0)
    {
      uint64_t node_id = walker->stack.top();
      BtreeNode<KEY, VALUE> * node = _tree->at(node_id);

      if(node->is_leaf)
      {
        if(walker->pos < node->count - 1)
        {
          if(started_node) 
          {
            walker->pos++;
          }
          return;
        }else{
          started_node = false;
          walker->pos = 0;
          walker->stack.pop();
        } 
      }else if(! node->is_leaf)
      {
        walker->stack.pop();
        if(node->count > 0)
        {
          int i = node->count;
          while(true)
          {
            uint64_t next = node->stem.children[i]; 
            walker->stack.push(next);
            _tree->madv_willneed(next);
            if(i == 0) break;
            i--;
          } 
        }
      }
    }
    walker->is_end = true;
  }
  
  ForwardWalker<KEY, VALUE> walkerBegin()
  {
    int64_t * r = _tree->header(0);
    std::stack<uint64_t> stack;
    stack.push((* r));

    ForwardWalker<KEY, VALUE> walker (this, stack,  0, true, false);
    walker.increment();

    return walker;
  }

  struct iterator : boost::iterator_facade<iterator, VALUE, boost::forward_traversal_tag>
  {
    ForwardWalker<KEY, VALUE> walker;
    bool                      is_end ;
    size_t                    pos ;
    mutable VALUE             current;

    iterator(Btree<KEY, VALUE> * p) 
    {
      if(p){
        walker  = p->walkerBegin();
        is_end  = walker.is_end;
        pos     = 0;
        current = walker.value();
      }else{
        is_end = true;
      }
    }

    void increment()
    { 
      if(! walker.is_end)
      {
        walker.increment();
        if(walker.is_end){
          is_end = true;
        }else{
          current = walker.value();
          pos++;
        }
      }else{
        is_end = true;
      }
    }

    bool equal(iterator const& other) const
    {
      if(is_end && other.is_end) return true;
      if(!is_end && !other.is_end &&  pos == other.pos) return true;
      return false;
    }

    VALUE& dereference() const
    { 
      return current;
    }
  };
  iterator begin()
  {
    return iterator(this);
  }
  iterator end()
  {
    return iterator(NULL);
  }
};


#endif

