#ifndef TRANSACTION_DATABASE_HPP
#define TRANSACTION_DATABASE_HPP

#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"
#include "util.hpp"

namespace libbitcoin {

class transaction_database
{
public:
    transaction_database(mmfile& file);
    void store(const bc::transaction_type& tx);

private:
    uint64_t read_bucket_value(uint64_t bucket_index);
    void write_records_size();
    void link_record(uint64_t bucket_index, uint64_t record_begin);

    mmfile& file_;
    size_t page_size_ = sysconf(_SC_PAGESIZE);
    uint64_t version_, buckets_, total_records_size_;
};

} // namespace libbitcoin

#endif

