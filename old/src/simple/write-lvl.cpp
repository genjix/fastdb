#include "mmfile.hpp"
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
using namespace bc;

int main()
{
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, "txlvl.db", &db);
    assert(status.ok());

    mmfile mf("txdump");
    size_t number_txs = 29180978;
    auto serial = make_serializer(mf.data());
    srand(time(NULL));
    data_chunk buffer;
    buffer.reserve(700);
    timed_section t("fastdb write", "writing data...");
    // Results:
    // 11560587.436745
    // 11498751.139852
    // 11476653.582037
    // 11537438.511248
    // 11608247.779545
    for (size_t i = 0; i < number_txs; ++i)
    {
        //if (i % 1000 == 0)
        //    log_debug() << i << " of " << number_txs;
        size_t buffer_size = rand() % 400 + 300;
        buffer.resize(buffer_size);
        for(size_t i = 0; i < buffer_size; i++)
            buffer[i] = rand() % 256;
        hash_digest tx_hash = generate_sha256_hash(buffer);
        leveldb::Slice key_slice(reinterpret_cast<const char*>(tx_hash.data()), tx_hash.size());
        leveldb::Slice value_slice(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        leveldb::Status s = db->Put(leveldb::WriteOptions(), key_slice, value_slice);
    }
    return 0;
}

