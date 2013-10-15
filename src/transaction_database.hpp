#ifndef TRANSACTION_DATABASE_HPP
#define TRANSACTION_DATABASE_HPP

#include <bitcoin/primitives.hpp>
#include "mmfile.hpp"
#include "util.hpp"

namespace libbitcoin {

class transaction_database_writer
{
public:
    transaction_database_writer(mmfile& file);
    void store(const transaction_type& tx);

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

class transaction_database_reader
{
public:
    transaction_database_reader(
        const mmfile& file,
        const transaction_database_writer& writer);
    bool get(const hash_digest& tx_hash, transaction_type& tx) const;

private:
    const mmfile& file_;
    const transaction_database_writer& writer_;
};

} // namespace libbitcoin

#endif

