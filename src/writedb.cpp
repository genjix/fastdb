#include <cassert>
#include <stdio.h>
#include <bitcoin/bitcoin.hpp>
#include "mmfile.hpp"

using namespace bc;

int main()
{
    // create and alloc file
    mmfile mf("../tx.db");
    log_debug() << "filesize: " << mf.size();
    return 0;
    assert(mf.data() != nullptr);
    // [ version ]      8
    // [ buckets ]      8
    // [ data size ]    8
    //     ...
    //     ...
    log_debug() << "filesize: " << mf.size();
    assert(mf.size() > 24);
    auto deserial = make_deserializer(mf.data(), mf.data() + 24);
    uint64_t version = deserial.read_8_bytes();
    uint64_t buckets = deserial.read_8_bytes();
    uint64_t data_size = deserial.read_8_bytes();
    assert(version == 1);
    log_debug() << "buckets: " << buckets << ", data_size: " << data_size;
    assert(mf.size() >= 24 + buckets * 8 + data_size);
    return 0;
}

