#include <bitcoin/bitcoin.hpp>
#include <bitcoin/utility/timed_section.hpp>
#include <bitcoin/blockchain/database/utility.hpp>
#include "hashtable_database.hpp"
using namespace libbitcoin;
using namespace libbitcoin::chain;

constexpr size_t total_txs = 20000;
constexpr size_t tx_size = 200;
constexpr size_t buckets = 100;

data_chunk generate_random_bytes(
    std::default_random_engine& engine, size_t size)
{
    data_chunk result(size);
    for (uint8_t& byte: result)
        byte = engine() % std::numeric_limits<uint8_t>::max();
    return result;
}

void initdb()
{
    touch_file("tx.db");
    mmfile file("tx.db");
    BITCOIN_ASSERT(file.data());
    file.resize(8 + 8 + 8 + 8 * buckets + total_txs * tx_size * 2);
    auto serial = make_serializer(file.data());
    serial.write_8_bytes(1);
    serial.write_8_bytes(buckets);
    serial.write_8_bytes(0);
    constexpr uint64_t record_doesnt_exist =
        std::numeric_limits<uint64_t>::max();
    for (uint64_t i = 0; i < buckets; ++i)
        serial.write_8_bytes(record_doesnt_exist);
}

void write_data()
{
    initdb();

    mmfile file("tx.db");
    BITCOIN_ASSERT(file.data());
    hashtable_database_writer ht(file);

    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);
        auto write = [&value](uint8_t* data)
        {
            std::copy(value.begin(), value.end(), data);
        };
        ht.store(key, value.size(), write);
    }
    ht.sync();
}

void validate_data()
{
    mmfile file("tx.db");
    BITCOIN_ASSERT(file.data());
    const hashtable_database_writer ht_writer(file);
    const hashtable_database_reader ht(file, ht_writer);

    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);

        const uint8_t* slab = ht.get(key);
        BITCOIN_ASSERT(slab);

        BITCOIN_ASSERT(std::equal(value.begin(), value.end(), slab));
    }
}

void read_data()
{
    mmfile file("tx.db");
    BITCOIN_ASSERT(file.data());
    hashtable_database_writer ht_writer(file);
    hashtable_database_reader ht(file, ht_writer);

    std::ostringstream oss;
    oss << "txs = " << total_txs << " size = " << tx_size
        << " buckets = " << buckets << " |  ";

    timed_section t("ht.get()", oss.str());
    std::default_random_engine engine;
    for (size_t i = 0; i < total_txs; ++i)
    {
        data_chunk value = generate_random_bytes(engine, tx_size);
        hash_digest key = bitcoin_hash(value);

        const auto slab = ht.get(key);
    }
}

void show_usage()
{
    std::cerr << "Usage: bench [-w]" << std::endl;
}

int main(int argc, char** argv)
{
    if (argc != 1 && argc != 2)
    {
        show_usage();
        return -1;
    }
    const std::string arg = (argc == 2) ? argv[1] : "";
    if (arg == "-h" || arg == "--help")
    {
        show_usage();
        return 0;
    }
    if (arg == "-w" || arg == "--write")
    {
        std::cout << "Writing..." << std::endl;
        write_data();
        std::cout << "Validating..." << std::endl;
        validate_data();
        std::cout << "Done." << std::endl;
    }
    // Perform benchmark.
    read_data();
    return 0;
}

