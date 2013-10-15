#ifndef MMFILE_HPP
#define MMFILE_HPP

#include <cstddef>
#include <cstdint>

class mmfile
{
public:
    mmfile(const char* path);
    ~mmfile();
    uint8_t* data();
    const uint8_t* data() const;
    const size_t size() const;
private:
    int file_handle_ = 0;
    uint8_t* data_ = nullptr;
    size_t size_;
};

#endif

