#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"
#include "util.hpp"

using namespace bc;

int main()
{
    log_debug() << "REMEMBER THAT TRANSACTIONS DO NOT CROSS PAGE FAULTS";
    log_debug() << "PARSING RECORDS MIGHT CRASH CLOSE TO THE NEXT PAGE";
    // create and alloc file
    mmfile mf("../tx.db");
    BITCOIN_ASSERT(mf.data() != nullptr);
    BITCOIN_ASSERT(mf.size() > 24);
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    uint64_t version = deserial.read_8_bytes();
    uint64_t buckets = deserial.read_8_bytes();
    uint64_t records_size = deserial.read_8_bytes();
    BITCOIN_ASSERT(version == 1);
    log_debug() << "buckets: " << buckets
        << ", records_size: " << records_size;
    log_debug() << "filesize: " << mf.size();
    const size_t header_size = 24 + buckets * 8;
    log_debug() << "header_size: " << header_size;
    BITCOIN_ASSERT(mf.size() >= 24 + buckets * 8 + records_size);
    // Start reading buckets.
    for (size_t i = 0; i < buckets; ++i)
    {
        uint64_t offset = deserial.read_8_bytes();
        log_debug() << "Bucket " << i << ": " << offset;
    }
    return 0;
}

