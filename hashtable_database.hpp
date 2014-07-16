#ifndef HASHTABLE_DATABASE_HPP
#define HASHTABLE_DATABASE_HPP

#include <functional>
#include <bitcoin/types.hpp>
#include <bitcoin/utility/mmfile.hpp>
#include "util.hpp"

namespace libbitcoin {

class hashtable_database_writer
{
public:
    typedef std::function<void (uint8_t*)> write_value_function;

    hashtable_database_writer(mmfile& file);
    void store(const hash_digest& key_hash,
        size_t value_size, write_value_function write);
    void sync();

    uint64_t buckets() const;
    uint64_t records_size() const;

private:
    uint64_t read_bucket_value(uint64_t bucket_index);
    void write_records_size();
    void link_record(uint64_t bucket_index, uint64_t record_begin);

    mmfile& file_;
    size_t page_size_ = sysconf(_SC_PAGESIZE);
    uint64_t version_, buckets_, total_records_size_;
};

class hashtable_database_reader
{
public:
    hashtable_database_reader(
        const mmfile& file,
        const hashtable_database_writer& writer);
    const uint8_t* get(const hash_digest& key_hash) const;

private:
    const mmfile& file_;
    const hashtable_database_writer& writer_;
};

} // namespace libbitcoin

#endif

