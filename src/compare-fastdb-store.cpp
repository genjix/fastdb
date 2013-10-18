#include "transaction_database.hpp"
#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <sys/mman.h>

using namespace bc;

int main()
{
    transaction_database txdb("../tx.db");
    // leveldb likes strings
    mmfile mf("txdump");
    madvise(mf.data(), mf.size(), POSIX_MADV_SEQUENTIAL);
    auto deserial = make_deserializer(mf.data(), mf.data() + mf.size());
    size_t i = 0;
    timed_section t("fastdb store", "storing...");
    while (true)
    {
        uint64_t raw_tx_size = deserial.read_variable_uint();
        if (raw_tx_size == 0)
            break;
        ++i;
        transaction_type tx;
        satoshi_load(deserial.iterator(), deserial.iterator() + raw_tx_size, tx);
        deserial.set_iterator(deserial.iterator() + raw_tx_size);
        txdb.store(tx);
    }
    txdb.sync();
    std::cout << "read " << i << " txs" << std::endl;
    std::cout << "saving to fastdb database" << std::endl;

    return 0;
}

