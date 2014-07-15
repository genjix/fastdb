#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"
#include "util.hpp"

using namespace bc;

int main(int argc, char** argv)
{
    BITCOIN_ASSERT(argc == 2);
    hash_digest tx_hash = decode_hex_digest<hash_digest>(argv[1]);
    mmfile mf("../tx.db");
    BITCOIN_ASSERT(mf.data() != nullptr);
    BITCOIN_ASSERT(mf.size() > 24);
    auto deserial = make_deserializer(mf.data() + 8, mf.data() + mf.size());
    uint64_t buckets = deserial.read_8_bytes();
    log_debug() << remainder(tx_hash.data(), buckets);
    return 0;
}

