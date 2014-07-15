#include "transaction_database.hpp"
#include "mmfile.hpp"
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

    size_t number_txs = 29180978;
    data_chunk buffer;
    buffer.reserve(700);
    // Can be changed to differentiate tests (create different data sets).
    size_t rand_factor = 110;
    timed_section t("lvldb store", "storing...");
    for (size_t i = 0; i < number_txs; ++i)
    {
        // Deterministically generate txs
        srand(i + rand_factor);
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

