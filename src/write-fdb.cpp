#include "transaction_database.hpp"
#include "mmfile.hpp"
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <sys/mman.h>
using namespace bc;

int main()
{
    mmfile mf("tx.db");
    hashtable_database_writer writer(mf);
    size_t number_txs = 29180978;
    data_chunk buffer;
    buffer.reserve(700);
    // Can be changed to differentiate tests (create different data sets).
    size_t rand_factor = 110;
    timed_section t("fastdb store", "storing...");
    for (size_t i = 0; i < number_txs; ++i)
    {
        // Deterministically generate txs
        srand(i + rand_factor);
        size_t buffer_size = rand() % 400 + 300;
        buffer.resize(buffer_size);
        for(size_t i = 0; i < buffer_size; i++)
            buffer[i] = rand() % 256;
        hash_digest tx_hash = generate_sha256_hash(buffer);
        auto write = [&buffer](uint8_t* it)
        {
            std::copy(buffer.begin(), buffer.end(), it);
        };
        writer.store(tx_hash, buffer.size(), write);
    }
    writer.sync();
    return 0;
}

