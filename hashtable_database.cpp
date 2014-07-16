#include "hashtable_database.hpp"

#include <sys/mman.h>
#include <bitcoin/utility/assert.hpp>
#include <bitcoin/utility/serializer.hpp>

namespace libbitcoin {

constexpr uint64_t record_doesnt_exist = std::numeric_limits<uint64_t>::max();

inline size_t bucket_offset(uint64_t bucket_index)
{
    return 24 + bucket_index * 8;
}

hashtable_database_writer::hashtable_database_writer(mmfile& file)
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
    const size_t header_size = 24 + buckets_ * 8;
    BITCOIN_ASSERT(file_.size() >= header_size + total_records_size_);
    // Advise the kernel that our access patterns for the tx records
    // will be random without pattern.
    //madvise(file_.data(), 24, POSIX_MADV_DONTNEED);
    //madvise(file_.data() + 24, buckets_ * 8,
    //    POSIX_MADV_WILLNEED | POSIX_MADV_RANDOM);
    //madvise(file_.data() + header_size, file_.size() - header_size,
    //    POSIX_MADV_DONTNEED | POSIX_MADV_RANDOM);
}

size_t align_if_crossing_page(
    size_t page_size, size_t records_end_offset, size_t record_size)
{
    size_t record_begin = records_end_offset;
    size_t record_end = record_begin + record_size;
    size_t starting_page = record_begin / page_size;
    size_t ending_page = record_end / page_size;
    if (record_size <= page_size && starting_page != ending_page)
        record_begin = ending_page * page_size;
    return record_begin;
}

void hashtable_database_writer::store(const hash_digest& key_hash,
    size_t value_size, write_value_function write)
{
    // Calculate the end of the last record.
    const uint64_t header_size = 24 + buckets_ * 8;
    const uint64_t records_end_offset = header_size + total_records_size_;
    // [ tx hash ]              32
    // [ varuint value size ]
    // [ ... value data ... ]
    // [ next tx in bucket ]    8
    const size_t record_size = 32 + value_size + 8;
    // If a record crosses a page boundary then we align it with
    // the beginning of the next page.

    /////////////////////////////////////////////////////////
    // DISABLED
    //const size_t record_begin =
    //    align_if_crossing_page(page_size_, records_end_offset, record_size);
    /////////////////////////////////////////////////////////
    const size_t record_begin = records_end_offset;

    BITCOIN_ASSERT(file_.size() >= record_begin + record_size);
    // We will insert new transactions at the beginning of the bucket's list.
    // I assume that more recent transactions in the blockchain are used
    // more often than older ones.
    // We lookup the existing value in the bucket first.
    const uint64_t bucket_index = remainder(key_hash.data(), buckets_);
    BITCOIN_ASSERT(bucket_index < buckets_);
    const uint64_t previous_bucket_value = read_bucket_value(bucket_index);
    // Now begin writing the record itself.
    uint8_t* entry = file_.data() + record_begin;
    std::copy(key_hash.begin(), key_hash.end(), entry);
    auto serial = make_serializer(entry + 32);
    serial.write_8_bytes(previous_bucket_value);
    // Call the supplied callback to serialize the data.
    write(serial.iterator());
    // Change file size value at file start.
    // This must be done first so any subsequent writes don't
    // overwrite this record in case of a crash or interruption.
    BITCOIN_ASSERT(record_begin >= header_size);

    /////////////////////////////////////////////////////////
    // DISABLED
    //const uint64_t alignment_padding =
    //    record_begin - header_size - total_records_size_;
    //BITCOIN_ASSERT(alignment_padding <= page_size_);
    //total_records_size_ += record_size + alignment_padding;
    /////////////////////////////////////////////////////////
    total_records_size_ += record_size;

    // Now add record to bucket.
    const uint64_t record_begin_offset = record_begin - header_size;
    link_record(bucket_index, record_begin_offset);
}

uint64_t hashtable_database_writer::read_bucket_value(uint64_t bucket_index)
{
    BITCOIN_ASSERT(file_.size() > 24 + buckets_ * 8);
    BITCOIN_ASSERT(bucket_index < buckets_);
    uint8_t* bucket_begin = file_.data() + bucket_offset(bucket_index);
    // Read current record stored in the bucket.
    auto deserial = make_deserializer(bucket_begin, bucket_begin + 8);
    return deserial.read_8_bytes();
}
void hashtable_database_writer::link_record(
    uint64_t bucket_index, uint64_t record_begin)
{
    BITCOIN_ASSERT(file_.size() > 24 + buckets_ * 8);
    BITCOIN_ASSERT(bucket_index < buckets_);
    uint8_t* bucket_begin = file_.data() + bucket_offset(bucket_index);
    auto serial = make_serializer(bucket_begin);
    serial.write_8_bytes(record_begin);
}

void hashtable_database_writer::sync()
{
    write_records_size();
}
void hashtable_database_writer::write_records_size()
{
    const size_t header_size = 24 + buckets_ * 8;
    BITCOIN_ASSERT(file_.size() >= header_size + total_records_size_);
    auto serial = make_serializer(file_.data() + 16);
    serial.write_8_bytes(total_records_size_);
}

uint64_t hashtable_database_writer::buckets() const
{
    return buckets_;
}
uint64_t hashtable_database_writer::records_size() const
{
    return total_records_size_;
}

hashtable_database_reader::hashtable_database_reader(
    const mmfile& file,
    const hashtable_database_writer& writer)
  : file_(file), writer_(writer)
{
    //madvise(file_.data(), 24, POSIX_MADV_DONTNEED);
    // buckets + data
    //madvise(file_.data() + 24, file_.size() - 24, POSIX_MADV_RANDOM);
}

uint64_t read_record_offset(const uint8_t* data, uint64_t bucket_index)
{
    const uint8_t* bucket_begin = data + bucket_offset(bucket_index);
    auto deserial = make_deserializer(bucket_begin, bucket_begin + 8);
    return deserial.read_8_bytes();
}

const uint8_t* hashtable_database_reader::get(
    const hash_digest& key_hash) const
{
    uint64_t bucket_index = remainder(key_hash.data(), writer_.buckets());
    BITCOIN_ASSERT(bucket_index < writer_.buckets());
    uint64_t current = read_record_offset(file_.data(), bucket_index);
    const uint64_t header_size = 24 + writer_.buckets() * 8;
    const uint8_t* file_data = file_.data() + header_size;
    // We don't know the end of a record, so we use the end of all records
    // for the deserializer.
    // We will be jumping around the records since it's a chained
    // list per bucket.
    // Begin iterating the list.
    while (current != record_doesnt_exist)
    {
        const uint8_t* data = file_data + current;
        if (std::equal(key_hash.begin(), key_hash.end(), data))
            return data + 32 + 8;
        // Move to next record in bucket.
        current = from_little_endian<uint64_t>(data + 32);
    }
    // Nothing found.
    return nullptr;
}

} // namespace libbitcoin

