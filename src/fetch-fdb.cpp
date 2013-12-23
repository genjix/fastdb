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
    hashtable_database_reader reader(mf, writer);
    size_t number_txs = 29180978;
    srand(time(NULL));
    data_chunk buffer;
    buffer.reserve(700);
    // Can be changed to differentiate tests (create different data sets).
    size_t rand_factor = 110;
    timed_section t("fastdb fetch", "fetching...");
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
        auto res = reader.get(tx_hash);
        bool is_equal = std::equal(buffer.begin(), buffer.end(), res.begin);
        assert(is_equal);
        assert(res.end - res.begin == buffer.size());
    }
    return 0;
}

