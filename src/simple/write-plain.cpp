#include "mmfile.hpp"
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <sys/mman.h>
using namespace bc;

// echo 3 > /proc/sys/vm/drop_caches

int main()
{
    mmfile mf("txdump");
    size_t number_txs = 29180978;
    auto serial = make_serializer(mf.data());
    srand(time(NULL));
    data_chunk buffer;
    buffer.reserve(700);
    madvise(mf.data(), mf.size(), POSIX_MADV_RANDOM);
    timed_section t("fastdb write", "writing data...");
    // Without writes:
    // 262089.278522
    // 293895.541698
    // 260600.351971
    // 260124.726865
    // 259762.253829
    // 260850.058261
    // 259600.363072
    // 260948.359327
    // 261281.748239
    // 259681.244647
    //
    // Results (No flags):
    // 692202.959596
    // 677918.818124
    // 682843.039729
    // 767803.261384
    // 772430.095862
    // 680427.051192
    // 676120.561095
    // 668439.730725
    //
    // Results (RANDOM):
    // 1740683.740056
    // 1738817.607825
    // 1729751.175940
    // 1749846.818516
    // 1690038.096907
    // 1715323.174246
    // 1961341.298289
    //
    // Results (SEQUENTIAL):
    // 674352.305528
    // 667701.693225
    // 667682.826192
    // 676014.604385
    // 670367.890811
    // 672859.941564
    //
    // Results (DONTNEED):
    // 672183.218356
    // 677141.608569
    // 675385.765224
    // 693432.248263
    // 673956.304528
    // 671125.281672
    // 677915.576831
    //
    // Results (WILLNEED):
    // 613442.633918
    // 607372.999572
    // 610250.597906
    // 618272.620437
    // 622475.000974
    // 614790.097457
    // 616835.535024
    // 611000.843473
    // 622685.192777
    for (size_t i = 0; i < number_txs; ++i)
    {
        size_t buffer_size = rand() % 400 + 300;
        buffer.resize(buffer_size);
        for(size_t i = 0; i < buffer_size; i++)
            buffer[i] = rand() % 256;
        serial.write_data(buffer);
    }
    return 0;
}

