#include "transaction_database.hpp"
#include "mmfile.hpp"
#include <leveldb/db.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>

using namespace bc;

int main()
{
    // leveldb likes strings
    std::vector<transaction_type> txs;
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
        txs.push_back(tx);
    }
    std::cout << "read " << i << " txs" << std::endl;
    std::cout << "saving to fastdb database" << std::endl;

    transaction_database txdb("../tx.db");
    timed_section t("fastdb store", "storing...");
    for (const auto& tx: txs)
        txdb.store(tx);

    return 0;
}

