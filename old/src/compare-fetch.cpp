#include "transaction_database.hpp"
#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <algorithm>
#include <sys/mman.h>

using namespace bc;

int main()
{
    // leveldb likes strings
    std::vector<hash_digest> txs;
    mmfile mf("txdump");
    madvise(mf.data(), mf.size(), POSIX_MADV_SEQUENTIAL);
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    size_t i = 0, total_size = 0;
    while (true)
    {
        uint64_t raw_tx_size = deserial.read_variable_uint();
        total_size += raw_tx_size;
        if (raw_tx_size == 0)
            break;
        ++i;
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);
        txs.push_back(hash_transaction(tx));
    }
    std::random_shuffle(txs.begin(), txs.end());
    std::cout << "reading " << i << " txs (" << total_size << " bytes)" << std::endl;
    std::cout << "testing fetch of databases" << std::endl;

    transaction_database* txdb = new transaction_database("../tx.db");
    timed_section* t = new timed_section("fastdb get", "getting...");
    for (const auto& tx_hash: txs)
    {
        transaction_type tx;
        bool success = txdb->get(tx_hash, tx);
    }
    delete t;
    delete txdb;

    leveldb::DB* db;
    leveldb::Options options;
    leveldb::Status status = leveldb::DB::Open(options, "blockchain/tx", &db);
    assert(status.ok());

    t = new timed_section("leveldb get", "getting...");
    for (const auto& tx_hash: txs)
    {
        leveldb::Slice key1(reinterpret_cast<const char*>(tx_hash.data()), 32);
        std::string value;
        leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
        transaction_type tx;
        satoshi_load(value.begin() + 8, value.end(), tx);
    }
    delete t;
    delete db;

    return 0;
}

