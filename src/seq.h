#ifndef SEQ_H_
#define SEQ_H_

#define FILE_SEQUENCE "tmp_seq_"

int      seq_create(const std::string& db_dir, const std::string& hash);
uint64_t seq_get(MmapVector< uint64_t > * seq, uint64_t hash);
uint64_t seq_get_random(MmapVector<uint64_t> * seq);
size_t   seq_size(const std::string& db_dir, const std::string& hash);
bool     seq_32bit(const std::string& db_dir, const std::string& hash);

#endif
