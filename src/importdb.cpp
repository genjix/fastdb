#include "transaction_database.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>

using namespace bc;

int main()
{
    // create and alloc file
    transaction_database txdb("../tx.db");

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, "blockchain/tx", &db);
    assert(status.ok());

    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        const uint8_t* hash_data =
            reinterpret_cast<const uint8_t*>(it->key().data());
        assert(it->key().size() == 32);
        hash_digest tx_hash;
        std::copy(hash_data, hash_data + 32, tx_hash.begin());
        const uint8_t* value =
            reinterpret_cast<const uint8_t*>(it->value().data());
        transaction_type tx;
        satoshi_load(value + 8, value + it->value().size(), tx);
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
        // Display every 1000th tx.
        if (rand() % 1000 == 0)
            log_debug() << "Importing: " << tx_hash;
        txdb.store(tx);
    }
    assert(it->status().ok());
    delete it;
    delete db;

    return 0;
}

