#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"

using namespace bc;

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    BITCOIN_ASSERT(mf.data() != nullptr);
    // [ version ]      8
    // [ buckets ]      8
    // [ values size ]  8
    //     ...
    //     ...
    uint64_t buckets = 725000000;
    BITCOIN_ASSERT(mf.size() > 24);
    auto serial = make_serializer(mf.data());
    serial.write_8_bytes(1);
    serial.write_8_bytes(buckets);
    serial.write_8_bytes(0);
    return 0;
}

