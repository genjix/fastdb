#include <leveldb/db.h>
#include "transaction_database.hpp"

using namespace bc;

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    transaction_database txdb(mf);
    transaction_type tx = genesis_block().transactions[0];
    txdb.store(tx);
    return 0;
}

