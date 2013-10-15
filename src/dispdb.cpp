#include <bitcoin/bitcoin.hpp>
#include <gmp.h>
#include "mmfile.hpp"

using namespace bc;

uint64_t remainder(const uint8_t* hash_data, const uint64_t divisor)
{
    mpz_t integ;
    mpz_init(integ);
    mpz_import(integ, 32, 1, 1, 1, 0, hash_data);
    uint64_t remainder = mpz_fdiv_ui(integ, divisor);
    mpz_clear(integ);
    return remainder;
}

int main()
{
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
    BITCOIN_ASSERT(mf.size() >= 24 + buckets * 8 + records_size);
    // Start reading records.
    uint8_t* records_begin = mf.data() + 24 + buckets * 8;
    uint8_t* current_record = records_begin;
    deserial.set_iterator(current_record);
    while (current_record - records_begin < records_size)
    {
        const hash_digest tx_hash = deserial.read_hash();
        uint64_t raw_tx_size = deserial.read_variable_uint();
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);
        uint64_t next = deserial.read_8_bytes();
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
        uint64_t bucket_index = remainder(tx_hash.data(), buckets);
        log_debug() << tx_hash << " [" << bucket_index << "] -> " << next;
        current_record = deserial.iterator();
    }
    BITCOIN_ASSERT(current_record - records_begin == records_size);
    return 0;
}

