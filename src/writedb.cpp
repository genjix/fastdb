#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"
#include <gmp.h>
#include <leveldb/db.h>

using namespace bc;

class transaction_database
{
public:
    transaction_database(mmfile& file);
    void write(const transaction_type& tx);

private:
    void write_records_size();

    mmfile& file_;
    size_t page_size_ = sysconf(_SC_PAGESIZE);
    uint64_t version_, buckets_, total_records_size_;
};

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
    BITCOIN_ASSERT(file_.size() >= 24 + buckets_ * 8 + total_records_size_);
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

void transaction_database::write(const transaction_type& tx)
{
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
    // Now begin writing the record itself.
    uint8_t* entry = file_.data() + record_begin;
    auto serial = make_serializer(entry);
    const hash_digest tx_hash = hash_transaction(tx);
    serial.write_hash(tx_hash);
    serial.write_variable_uint(raw_tx_size);
    auto it = satoshi_save(tx, serial.iterator());
    serial.set_iterator(it);
    serial.write_8_bytes(0);
    BITCOIN_ASSERT(serial.iterator() == entry + record_size);
    // Change file size value at file start.
    // This must be done first so any subsequent writes don't
    // overwrite this record in case of a crash or interruption.
    total_records_size_ += record_begin + record_size - records_end_offset;
    write_records_size();
    // 
    // change next value of any colliding tx in the bucket.
}

void transaction_database::write_records_size()
{
    BITCOIN_ASSERT(file_.size() > 24);
    auto serial = make_serializer(file_.data() + 16);
    serial.write_8_bytes(total_records_size_);
}

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    transaction_database txdb(mf);
    transaction_type tx = genesis_block().transactions[0];
    txdb.write(tx);
    return 0;
}

