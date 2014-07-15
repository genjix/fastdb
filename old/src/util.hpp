#ifndef UTIL_HPP
#define UTIL_HPP

#include <cstdint>
#include <gmp.h>

namespace libbitcoin {

inline uint64_t remainder(const uint8_t* hash_data, const uint64_t divisor)
{
    mpz_t integ;
    mpz_init(integ);
    mpz_import(integ, 32, 1, 1, 1, 0, hash_data);
    uint64_t remainder = mpz_fdiv_ui(integ, divisor);
    mpz_clear(integ);
    return remainder;
}

} // namespace libbitcoin

#endif

