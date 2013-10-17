#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <sys/mman.h>

using namespace bc;

int main()
{
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "txlvl.db", &db);
    assert(status.ok());

    mmfile mf("txdump");
    madvise(mf.data(), mf.size(), POSIX_MADV_SEQUENTIAL);
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    size_t i = 0;
    timed_section t("leveldb store", "storing...");
    while (true)
    {
        uint64_t raw_tx_size = deserial.read_variable_uint();
        if (raw_tx_size == 0)
            break;
        ++i;
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);

        hash_digest tx_hash = hash_transaction(tx);
        leveldb::Slice key_slice(reinterpret_cast<const char*>(tx_hash.data()), tx_hash.size());
        std::string raw_tx;
        raw_tx.resize(satoshi_raw_size(tx));
        satoshi_save(tx, raw_tx.begin());
        leveldb::Slice value_slice(raw_tx.data(), raw_tx.size());
        leveldb::Status s = db->Put(leveldb::WriteOptions(), key_slice, value_slice);
    }
    std::cout << "read " << i << " txs" << std::endl;
    std::cout << "saving to lvldb database" << std::endl;

    return 0;
}

