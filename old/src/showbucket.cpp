#include <boost/lexical_cast.hpp>
#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"
#include "util.hpp"

using namespace bc;

int main(int argc, char** argv)
{
    uint64_t bucket_index = 698648699;
    if (argc == 2)
        bucket_index = boost::lexical_cast<uint64_t>(argv[1]);
    // create and alloc file
    mmfile mf("../tx.db");
    BITCOIN_ASSERT(mf.data() != nullptr);
    BITCOIN_ASSERT(mf.size() > 24);
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    uint64_t version = deserial.read_8_bytes();
    uint64_t buckets = deserial.read_8_bytes();
    uint64_t records_size = deserial.read_8_bytes();
    BITCOIN_ASSERT(version == 1);
    BITCOIN_ASSERT(mf.size() >= 24 + buckets * 8 + records_size);
    BITCOIN_ASSERT(bucket_index < buckets);
    // Read bucket value.
    BITCOIN_ASSERT(deserial.iterator() == mf.data() + 24);
    deserial.set_iterator(deserial.iterator() + bucket_index * 8);
    uint64_t record_offset = deserial.read_8_bytes();
    log_debug() << "record offset: " << record_offset;
    // Start reading records.
    uint8_t* records_begin = mf.data() + 24 + buckets * 8;
    uint8_t* current_record = records_begin + record_offset;
    deserial.set_iterator(current_record);
    while (true)
    {
        BITCOIN_ASSERT(current_record - records_begin < records_size);
        const hash_digest tx_hash = deserial.read_hash();
        uint64_t raw_tx_size = deserial.read_variable_uint();
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);
        uint64_t next = deserial.read_8_bytes();
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
        uint64_t bucket_index = remainder(tx_hash.data(), buckets);
        log_debug() << tx_hash << " [" << bucket_index << "] -> " << next;
        constexpr uint64_t record_doesnt_exist =
            std::numeric_limits<uint64_t>::max();
        if (next == record_doesnt_exist)
            break;
        current_record = records_begin + next;
        deserial.set_iterator(current_record);
    }
    return 0;
}

