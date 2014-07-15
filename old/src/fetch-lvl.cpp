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
    srand(time(NULL));
    data_chunk buffer;
    buffer.reserve(700);
    // Can be changed to differentiate tests (create different data sets).
    size_t rand_factor = 110;
    timed_section t("lvldb fetch", "fetching...");
    for (size_t i = 0; i < number_txs; ++i)
    {
        size_t tx_idx = rand() % number_txs;
        // Deterministically generate txs
        srand(tx_idx + rand_factor);
        size_t buffer_size = rand() % 400 + 300;
        buffer.resize(buffer_size);
        for(size_t i = 0; i < buffer_size; i++)
            buffer[i] = rand() % 256;
        hash_digest tx_hash = generate_sha256_hash(buffer);

        leveldb::Slice key1(reinterpret_cast<const char*>(tx_hash.data()), 32);
        std::string value;
        leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);

        const uint8_t* val = reinterpret_cast<const uint8_t*>(value.data());
        //log_debug() << "eq: " << encode_hex(val);

        assert(value.size() == buffer.size());
        bool is_equal = std::equal(buffer.begin(), buffer.end(), val);
        assert(is_equal);
    }
    return 0;
}

