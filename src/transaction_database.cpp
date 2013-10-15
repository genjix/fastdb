#include "transaction_database.hpp"

#include <bitcoin/satoshi_serialize.hpp>
#include <bitcoin/transaction.hpp>
#include <bitcoin/utility/assert.hpp>

#define LOG_TXDB    "transaction_database"

namespace libbitcoin {

transaction_database::transaction_database(const std::string& filename)
  : file_(filename), writer_(file_), reader_(file_, writer_)
{
}

void transaction_database::store(const transaction_type& tx)
{
    auto save_tx = [&tx](uint8_t* it)
    {
        satoshi_save(tx, it);
    };
    writer_.store(hash_transaction(tx), satoshi_raw_size(tx), save_tx);
}
bool transaction_database::get(
    const hash_digest& tx_hash, transaction_type& tx) const
{
    auto result = reader_.get(tx_hash);
    BITCOIN_ASSERT(result.begin != nullptr || (result.begin == result.end));
    if (result.begin == nullptr)
        return false;
    satoshi_load(result.begin, result.end, tx);
    return true;
}

} // namespace libbitcoin

