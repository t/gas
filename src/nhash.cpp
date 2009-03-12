#include <vector>
#include <queue>
#include <algorithm>
#include <map>
#include <string>
#include <iostream>
#include <stdbool.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <boost/program_options.hpp>

using namespace std;
using namespace boost;
using namespace boost::program_options;

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
		      +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash(const char* data, int len)
{
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

uint32_t MurmurHash2 ( const void * key, int len, unsigned int seed )
{
  const uint32_t m = 0x5bd1e995;
  const int r = 24;

  uint32_t h = seed ^ len;
  const unsigned char * data = (const unsigned char *)key;

  while(len >= 4)
  {
    unsigned int k = *(unsigned int *)data;

    k *= m; 
    k ^= k >> r; 
    k *= m; 
		
    h *= m; 
    h ^= k;

    data += 4;
    len -= 4;
  }
	
  switch(len)
  {
  case 3: h ^= data[2] << 16;
  case 2: h ^= data[1] << 8;
  case 1: h ^= data[0];
          h *= m;
  };

  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
} 

uint64_t nhash(const void* buf, size_t len)
{
    uint64_t h = MurmurHash2(buf, len, 0);
    return (h << 32) | SuperFastHash(static_cast<const char*>(buf), len);
}

int main(int argc, char *argv[] )
{
  options_description opt("オプション");
  opt.add_options()
    ("help,h",                                          "ヘルプを表示")
    ("n,n",        value<int>()->default_value(1),      "n")
    ("add,a",      value<string>()->default_value(""),  "add")
    ("output,o",   value<string>()->default_value(""),  "output")
  ;

  variables_map vm;
  store(parse_command_line(argc, argv, opt), vm);
  notify(vm);

  if(vm.count("help") ) {
    cout << opt << endl;
    return 0;
  }

  const int    n       = vm["n"].as<int>();
  const string add     = vm["add"].as<string>();
  const string output  = vm["output"].as<string>();

  ssize_t read_size;
  size_t buffer_size = 0;
  char* buf;
  const char * const delimiter = "\t";
  while (-1 != (read_size = getline(&buf, &buffer_size, stdin)))
  {
    buf[read_size - 1] = '\0';  // remove 'newline'
    char *s;
    s = strtok(buf, delimiter);
    int i=0;
    while (s != NULL)
    {
      if (i > 0) printf("\t");
      if (i < n)
      {
        uint64_t hash = nhash(s, strlen(s));
        if (output == "hex")
          printf("%llx", hash);
        else
          printf("%llu", hash);
      }else{
        printf("%s", s);
      }
      s = strtok(NULL, delimiter);
      ++i;
    }
    if (add != "") printf("\t%s\n", add.c_str());
    else printf("\n");
  }

  if (buf != NULL) free(buf);

  return EXIT_SUCCESS;
}

