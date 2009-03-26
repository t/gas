#ifndef MMAP_VECTOR_H_
#define MMAP_VECTOR_H_

#include <assert.h>
#include <map>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <strstream>
	
#define HEADER_SIZE 4096

template <typename T>
class MmapVector
{
  protected:
    int          _fd;
    bool         _opened;
    char      *  _mmap_start;
    size_t       _mmap_length;
    char      *  _body_start;
    int64_t   *  _count;
    int          _item_count;
    int          _page_size;
    std::string  _filepath;

    void remap(size_t require)
    {
      size_t new_length = (((require + HEADER_SIZE) / _page_size) + 1) * _page_size;
      ftruncate(_fd, new_length);
      _mmap_start  = (char*) mremap(_mmap_start, _mmap_length, new_length, MREMAP_MAYMOVE);
      assert(_mmap_start != MAP_FAILED);

     //madvise(_mmap_start, new_length, MADV_RANDOM);
      _mmap_length = new_length;
      _count       = (int64_t*) _mmap_start;
      _body_start  = _mmap_start + HEADER_SIZE;
    }

  public:
    MmapVector(const std::string& filepath)
    {
      _filepath = filepath;
      _opened   = false;
    }

    ~MmapVector()
    {
    }
  
    int open(bool initialize = false)
    {
      if((_fd = ::open(_filepath.c_str(), O_RDWR|O_CREAT, 0666)) == -1)
      {
        perror("ftruncate");
        exit(-1);
      }

      _page_size  = getpagesize();
      //std::cout << "_page_size: " << _page_size << std::endl;
      char * start = (char *) mmap(0, HEADER_SIZE, PROT_READ | PROT_WRITE,  MAP_SHARED, _fd, 0);
      if(start < 0){
        perror("mmaperror");
        exit(-1);
      }
      _mmap_start  = start;
      _mmap_length = HEADER_SIZE;
      _count       = (int64_t*) start;

      if(initialize)
      {
        remap(0); 
        _count[0] = 0;
      }else{
        remap((_count[0]) * sizeof(T));
      }

      _opened = true;
    }

    int close()
    {
      assert(_opened);

      int64_t count = (*_count);
      munmap(_mmap_start, _mmap_length);
      ftruncate(_fd, count * sizeof(T) + HEADER_SIZE);
      ::close(_fd);
   
      _opened = false;
    }

    int refresh()
    {
      assert(_opened);

      munmap(_mmap_start, _mmap_length);

      char * start = (char *) mmap(0, HEADER_SIZE, PROT_READ | PROT_WRITE,  MAP_SHARED, _fd, 0);
      if(start < 0){
        perror("mmaperror");
        exit(-1);
      }
      _mmap_start  = start;
      _mmap_length = HEADER_SIZE;
      _count       = (int64_t*) start;

      remap((_count[0]) * sizeof(T));
    }


    T * at(int64_t index)
    {
      assert(_opened);
      assert(index >= 0);

      if(index >= _count[0])
      {
        std::cout << "error: size = " <<  _count[0] << " index = " << index << ", " << _filepath << std::endl;
        assert(0);
      }
      return ((T *)_body_start) + index;
    }
  
    int64_t * header(int index)
    {
      assert(_opened);
      assert(index >= 0);
      return (int64_t *) (_mmap_start +sizeof(int64_t)  ) + index;
    }

    T * back()
    {
      assert(_opened);
      assert(_count[0] > 0);
      return at(_count[0] - 1);
    }

    void push_back(const T& add)
    {
      assert(_opened);

      _count[0]++;

      while(_mmap_length < (HEADER_SIZE +  _count[0] * sizeof(T)))
      {
        remap(_mmap_length * 2);
      }
	
      T * p = at( _count[0] - 1);
      * p = add;
    }

    void resize(size_t size)
    {
      (*_count) = size;
    }

    size_t size()
    {
      assert(_opened);

      return  _count[0];
    }

    void madv_willneed(size_t index)
    {
      assert(_opened);

      madvise(((T *)_body_start) + index, sizeof(T), MADV_WILLNEED);
    }

    void madv_sequential(size_t index, size_t count)
    {
      assert(_opened);

      madvise(((T *)_body_start) + index, sizeof(T) * count, MADV_SEQUENTIAL);
    }

    void madv_dontneed(size_t index, size_t count)
    {
      assert(_opened);

      madvise(((T *)_body_start) + index, sizeof(T) * count, MADV_DONTNEED);
      //madvise(_body_start + index * sizeof(T), count * sizeof(T), MADV_DONTNEED);
    }

    struct iterator : boost::iterator_facade<iterator, T, boost::forward_traversal_tag>
    {
      MmapVector<T> * parent;
      size_t          pos;

      iterator(){}
      iterator(MmapVector<T> * p)
      {
        parent = p;
        pos    = 0;
      }

      bool is_end() const
      {
        if(parent == NULL) return true;
        return (pos >= parent->size());
      }

      void increment()
      {
        if(! is_end()) pos++;
      }

      bool equal(iterator const& other) const
      {
        if(is_end() && other.is_end()) return true;
        if(!is_end() && !other.is_end() && pos == other.pos) return true;
        return false;
      }

      T& dereference() const
      {
        (* parent->at(pos));
        return (* parent->at(pos));
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
