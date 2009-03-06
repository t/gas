#ifndef IELIAS_VECTOR_H_
#define IELIAS_VECTOR_H_

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

#include "mmap_vector.h"

template <typename T> 
class iEliasVector
{
protected:
  MmapVector<char> * _adjlist;

  void set_bit(const bool& bit, const int64_t& pos_byte, const int64_t& pos_bit)
  {
    int add_size = pos_byte - _adjlist->size() + 1;
    for(int i = 0; i < add_size; i++){
      _adjlist->push_back(0);
    }

    if(! bit) return;

    char c = 1 << (7 - pos_bit);
    char * at = _adjlist->at(pos_byte);

    (*at) |= c;
  }

  void push_bit(bool bit)
  {
    int64_t pos_byte_c = (* _adjlist->header(1) );
    int64_t pos_bit_c  = (* _adjlist->header(2) );

    set_bit(bit, pos_byte_c, pos_bit_c);

    int64_t * pos_byte = _adjlist->header(1);
    int64_t * pos_bit  = _adjlist->header(2);

    (* pos_bit)++;
    if( (* pos_bit) == 8){
      (* pos_byte) ++;
      (* pos_bit) = 0;
    }
  }

  void push_bits(char bits, int start)
  {
    for(int i = start; i < 8; i++){
      char c = 1; c << (7 - i);
      push_bit(bits & c);
    }
  }

  bool get_bit(const int64_t& pos_byte, const int64_t& pos_bit)
  {
    char c = 1; c <<= (7 - pos_bit);
    char * at = _adjlist->at(pos_byte);
    return (* at) & c;
  }

public:
  iEliasVector(const std::string& filepath)
  {
    _adjlist = new MmapVector<char>(filepath);
  }

  ~iEliasVector()
  {
    delete _adjlist; 
  }

  int open(bool initialize = false)
  {
    _adjlist->open(initialize);
    if(initialize){
      int64_t * pos_byte = _adjlist->header(1);
      int64_t * pos_bit  = _adjlist->header(2);
      (* pos_byte) = 0;
      (* pos_bit)  = 0;
    }
  }

  int close()
  {
    _adjlist->close();
  } 

  void push_back(const T& num)
  {
    T body = num + 1;
    T c    = 1; c <<= (sizeof(T) * 8 - 1);

    int body_size = 0;
    T   n2        = 1;
   
    while(body >= n2){
      body_size++;
 
      if(n2 == c) break;
      n2 <<= 1;  
    }
    body_size--;

    for(int i = 0; i < body_size; i++){
      push_bit(false);
    }
    push_bit(true);

    for(int i = 0; i < body_size; i++){
      push_bit( (body >> body_size - i - 1) & 1);
    }
  }

  struct iterator : boost::iterator_facade<iterator, T, boost::forward_traversal_tag>
  {
    bool      is_end;
    int64_t   pos_byte;
    int64_t   pos_bit;
    mutable T current;
    iEliasVector<T> * parent;

    iterator(iEliasVector<T> * p)
    {
      if(p){
        is_end   = false;
        pos_byte = 0;
        pos_bit  = 0;
        parent   = p;
        increment();
      }else{
        is_end   = true;
      }
    }

    bool increment_bit()
    {
      pos_bit++;
      if(pos_bit == 8) {
        pos_byte++;
        pos_bit = 0;
      }
    }

    void increment()
    {
      int64_t * last_pos_byte = parent->_adjlist->header(1);
      int64_t * last_pos_bit  = parent->_adjlist->header(2);

      if( (* last_pos_byte) == pos_byte &&
           (* last_pos_bit ) == pos_bit){
        is_end = true;
        return;
      }

      int body_size = 0;
      while(! is_end) {
        bool bit = parent->get_bit(pos_byte, pos_bit);
        increment_bit();
        if(bit) break;
        body_size++;
      }

      T new_current = 0;
      new_current += 1 << body_size; 
      for(int i = 0; i < body_size; i++){
        bool bit = parent->get_bit(pos_byte, pos_bit);
        increment_bit();
        if(bit){
          //std::cout << " old = " << new_current << " plus = " << (1 << body_size - i - 1) << std::endl;
          new_current += 1 << body_size - i - 1;
        }
      } 

      current = new_current - 1;
    }

    bool equal(iterator const& other) const
    {
      if(is_end && other.is_end) return true;
      if(!is_end && !other.is_end && pos_byte == other.pos_byte && pos_bit && other.pos_bit) return true;
      return false;
    }

    T& dereference() const
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
