#include "transaction_database.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>

using namespace bc;

int main()
{
    constexpr size_t total_number = 10000;
    log_debug() << "Storing " << total_number << " txs";
    std::vector<hash_digest> tx_hashes;
    tx_hashes.reserve(total_number);
    // create and alloc file
    transaction_database txdb("../tx.db");

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, "blockchain/tx", &db);
    assert(status.ok());

    timed_section* t = new timed_section("benchmark", "storing");
    size_t i = 0;
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        ++i;
        if (i > total_number)
            break;
        const uint8_t* hash_data =
            reinterpret_cast<const uint8_t*>(it->key().data());
        assert(it->key().size() == 32);
        hash_digest tx_hash;
        std::copy(hash_data, hash_data + 32, tx_hash.begin());
        tx_hashes.push_back(tx_hash);
        const uint8_t* value =
            reinterpret_cast<const uint8_t*>(it->value().data()) + 8;
        transaction_type tx;
        satoshi_load(value, value + it->value().size(), tx);
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
        txdb.store(tx);
    }
    delete t;
    assert(it->status().ok());
    delete it;
    delete db;

    log_debug() << "Reading " << total_number << " txs";

    t = new timed_section("benchmark", "storing");
    for (const hash_digest& tx_hash: tx_hashes)
    {
        transaction_type tx;
        bool success = txdb.get(tx_hash, tx);
        BITCOIN_ASSERT(success);
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
    }
    delete t;

    return 0;
}

