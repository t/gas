#ifndef SEQ_H_
#define SEQ_H_

#define FILE_SEQUENCE "tmp_seq"

int      seq_create();
uint64_t seq_get(MmapVector< uint64_t > * seq, uint64_t hash);
uint64_t seq_get_random(MmapVector<uint64_t> * seq);
size_t   seq_size();
bool     seq_32bit();

#endif
