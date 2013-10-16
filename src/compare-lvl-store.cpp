#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>

using namespace bc;

int main()
{
    // leveldb likes strings
    std::vector<std::string> keys, values;
    keys.reserve(10000);
    values.reserve(10000);
    mmfile mf("txdump");
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    size_t i = 0;
    while (true)
    {
        uint64_t raw_tx_size = deserial.read_variable_uint();
        if (raw_tx_size == 0)
            break;
        ++i;
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);
        std::string key, value;
        key.resize(32);
        value.resize(satoshi_raw_size(tx));
        auto serial_key = make_serializer(key.begin());
        serial_key.write_hash(hash_transaction(tx));
        satoshi_save(tx, value.begin());
        keys.push_back(key);
        values.push_back(value);
    }
    std::cout << "read " << i << " txs" << std::endl;
    std::cout << "saving to lvldb database" << std::endl;
    BITCOIN_ASSERT(values.size() == keys.size());
    BITCOIN_ASSERT(values.size() == i);

    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "blockchain/tx", &db);
    assert(status.ok());

    timed_section t("leveldb store", "storing...");
    for (size_t j = 0; j < i; ++j)
    {
        const std::string& key = keys[j];
        const std::string& value = values[j];
        leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
    }

    return 0;
}

