#include "transaction_database.hpp"
#include <bitcoin/bitcoin.hpp>

using namespace bc;

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    transaction_database_writer txdb(mf);
    transaction_type tx = genesis_block().transactions[0];
    txdb.store(tx);
    return 0;
}

