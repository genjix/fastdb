#ifndef TRANSACTION_DATABASE_HPP
#define TRANSACTION_DATABASE_HPP

#include <bitcoin/primitives.hpp>
#include "hashtable_database.hpp"

namespace libbitcoin {

class transaction_database
{
public:
    transaction_database(const std::string& filename);
    void store(const transaction_type& tx);
    bool get(const hash_digest& tx_hash, transaction_type& tx) const;

private:
    mmfile file_;
    hashtable_database_writer writer_;
    hashtable_database_reader reader_;
};

} // namespace libbitcoin

#endif

