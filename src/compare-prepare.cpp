#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>

using namespace bc;

int main()
{
    size_t total_number = 1000000;
    mmfile mf("txdump");

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, "blockchain/tx", &db);
    assert(status.ok());

    auto serial = make_serializer(mf.data());
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    size_t i = 0;
    size_t total_data = 0;
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        ++i;
        if (i > total_number)
        {
            serial.write_variable_uint(0);
            break;
        }
        const uint8_t* hash_data =
            reinterpret_cast<const uint8_t*>(it->key().data());
        assert(it->key().size() == 32);
        hash_digest tx_hash;
        std::copy(hash_data, hash_data + 32, tx_hash.begin());
        const uint8_t* value =
            reinterpret_cast<const uint8_t*>(it->value().data()) + 8;
        transaction_type tx;
        satoshi_load(value, value + it->value().size(), tx);
        BITCOIN_ASSERT(hash_transaction(tx) == tx_hash);
        // Display every 1000th tx.
        if (rand() % 1000 == 0)
            log_debug() << "Saving: " << tx_hash;
        // Save it
        serial.write_variable_uint(satoshi_raw_size(tx));
        auto it = satoshi_save(tx, serial.iterator());
        serial.set_iterator(it);
        total_data += satoshi_raw_size(tx);
    }
    assert(it->status().ok());
    delete it;
    delete db;

    log_debug() << "Saved total " << total_data << " bytes";

    return 0;
}

