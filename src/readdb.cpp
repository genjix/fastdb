#include "transaction_database.hpp"
#include <bitcoin/bitcoin.hpp>

using namespace bc;

int main(int argc, char** argv)
{
    std::string tx_hash_str =
        "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b";
    if (argc == 2)
        tx_hash_str = argv[1];
    // create and alloc file
    mmfile mf("../tx.db");
    transaction_database_writer txdb_write(mf);
    transaction_database_reader txdb(mf, txdb_write);
    hash_digest tx_hash = decode_hex_digest<hash_digest>(tx_hash_str);
    transaction_type tx;
    bool success = txdb.get(tx_hash, tx);
    BITCOIN_ASSERT(success);
    log_debug() << "Fetched " << tx_hash;
    return 0;
}

