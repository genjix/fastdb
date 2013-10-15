#include <bitcoin/bitcoin.hpp>
#include <leveldb/db.h>
#include "mmfile.hpp"
#include "util.hpp"

using namespace bc;

class transaction_database
{
public:
    transaction_database(mmfile& file);
    void store(const transaction_type& tx);

private:
    uint64_t read_bucket_value(uint64_t bucket_index);
    void write_records_size();
    void link_record(uint64_t bucket_index, uint64_t record_begin);

    mmfile& file_;
    size_t page_size_ = sysconf(_SC_PAGESIZE);
    uint64_t version_, buckets_, total_records_size_;
};

#include <sys/mman.h>

transaction_database::transaction_database(mmfile& file)
  : file_(file)
{
    BITCOIN_ASSERT(file_.data() != nullptr);
    // [ version ]      8
    // [ buckets ]      8
    // [ values size ]  8
    //     ...
    //     ...
    BITCOIN_ASSERT(file_.size() > 24);
    auto deserial = make_deserializer(file_.data(), file_.data() + 24);
    version_ = deserial.read_8_bytes();
    buckets_ = deserial.read_8_bytes();
    total_records_size_ = deserial.read_8_bytes();
    BITCOIN_ASSERT(version_ == 1);
    log_debug() << "buckets: " << buckets_
        << ", values_size: " << total_records_size_;
    const size_t header_size = 24 + buckets_ * 8;
    BITCOIN_ASSERT(file_.size() >= header_size + total_records_size_);
    // Advise the kernel that our access patterns for the tx records
    // will be random without pattern.
    madvise(file_.data() + header_size, file_.size() - header_size,
        POSIX_MADV_RANDOM);
}

size_t align_if_crossing_page(
    size_t page_size, size_t records_end_offset, size_t record_size)
{
    size_t record_begin = records_end_offset;
    size_t record_end = record_begin + record_size;
    size_t starting_page = record_begin / page_size;
    size_t ending_page = record_end / page_size;
    if (starting_page != ending_page)
        record_begin = ending_page * page_size;
    return record_begin;
}

void transaction_database::store(const transaction_type& tx)
{
    // Calculate the end of the last record.
    uint64_t records_end_offset = 24 + buckets_ * 8 + total_records_size_;
    // [ tx hash ]              32
    // [ varuint record size ]
    // [ ... data ... ]
    // [ next tx in bucket ]    8
    size_t raw_tx_size = satoshi_raw_size(tx);
    size_t record_size =
        32 + variable_uint_size(raw_tx_size) + raw_tx_size + 8;
    // If a record crosses a page boundary then we align it with
    // the beginning of the next page.
    size_t record_begin =
        align_if_crossing_page(page_size_, records_end_offset, record_size);
    BITCOIN_ASSERT(file_.size() >= record_begin + record_size);
    const hash_digest tx_hash = hash_transaction(tx);
    // We will insert new transactions at the beginning of the bucket's list.
    // I assume that more recent transactions in the blockchain are used
    // more often than older ones.
    // We lookup the existing value in the bucket first.
    uint64_t bucket_index = remainder(tx_hash.data(), buckets_);
    BITCOIN_ASSERT(bucket_index < buckets_);
    uint64_t previous_bucket_value = read_bucket_value(bucket_index);
    // Now begin writing the record itself.
    uint8_t* entry = file_.data() + record_begin;
    auto serial = make_serializer(entry);
    serial.write_hash(tx_hash);
    serial.write_variable_uint(raw_tx_size);
    auto it = satoshi_save(tx, serial.iterator());
    serial.set_iterator(it);
    serial.write_8_bytes(previous_bucket_value);
    BITCOIN_ASSERT(serial.iterator() == entry + record_size);
    // Change file size value at file start.
    // This must be done first so any subsequent writes don't
    // overwrite this record in case of a crash or interruption.
    total_records_size_ += record_begin + record_size - records_end_offset;
    write_records_size();
    // Now add record to bucket.
    link_record(bucket_index, record_begin);
}

inline size_t bucket_offset(uint64_t bucket_index)
{
    return 24 + bucket_index * 8;
}

uint64_t transaction_database::read_bucket_value(uint64_t bucket_index)
{
    BITCOIN_ASSERT(file_.size() > 24 + buckets_ * 8);
    BITCOIN_ASSERT(bucket_index < buckets_);
    uint8_t* bucket_begin = file_.data() + bucket_offset(bucket_index);
    // Read current record stored in the bucket.
    auto deserial = make_deserializer(bucket_begin, bucket_begin + 8);
    return deserial.read_8_bytes();
}
void transaction_database::write_records_size()
{
    BITCOIN_ASSERT(file_.size() > 24);
    auto serial = make_serializer(file_.data() + 16);
    serial.write_8_bytes(total_records_size_);
}
void transaction_database::link_record(
    uint64_t bucket_index, uint64_t record_begin)
{
    BITCOIN_ASSERT(file_.size() > 24 + buckets_ * 8);
    BITCOIN_ASSERT(bucket_index < buckets_);
    uint8_t* bucket_begin = file_.data() + bucket_offset(bucket_index);
    auto serial = make_serializer(bucket_begin);
    serial.write_8_bytes(record_begin);
}

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    transaction_database txdb(mf);
    transaction_type tx = genesis_block().transactions[0];
    txdb.store(tx);
    return 0;
}

