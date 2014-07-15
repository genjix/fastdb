// Originally taken from:
// http://en.wikibooks.org/wiki/Optimizing_C%2B%2B/General_optimization_techniques/Input/Output#Memory-mapped_file
// To whoever wrote this: thanks for the handy code snippet and cool free book!
#include "memory_file.hpp"
#if defined(_WIN32)
// our typedefs MemoryFile::HANDLE and InputMemoryFile::HANDLE play nice with typedef HANDLE
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
 
InputMemoryFile::InputMemoryFile(const char *pathname):
    data_(0),
    size_(0),
#if defined(_WIN32)
    file_handle_(INVALID_HANDLE_VALUE),
    file_mapping_handle_(INVALID_HANDLE_VALUE)
#else
    file_handle_(-1)
#endif
{
#if defined(_WIN32)
    file_handle_ = ::CreateFile(pathname, GENERIC_READ,
        FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle_ == INVALID_HANDLE_VALUE) return;
    file_mapping_handle_ = ::CreateFileMapping(
        file_handle_, 0, PAGE_READONLY, 0, 0, 0);
    if (file_mapping_handle_ == INVALID_HANDLE_VALUE) return;
    data_ = static_cast<char*>(::MapViewOfFile(
        file_mapping_handle_, FILE_MAP_READ, 0, 0, 0));
    if (data_) size_ = ::GetFileSize(file_handle_, 0);
#else
    file_handle_ = ::open(pathname, O_RDONLY);
    if (file_handle_ == -1) return;
    struct stat sbuf;
    if (::fstat(file_handle_, &sbuf) == -1) return;
    data_ = static_cast<const char*>(::mmap(
        0, sbuf.st_size, PROT_READ, MAP_SHARED, file_handle_, 0));
    if (data_ == MAP_FAILED) data_ = 0;
    else size_ = sbuf.st_size;
#endif
}
 
InputMemoryFile::~InputMemoryFile() {
#if defined(_WIN32)
    ::UnmapViewOfFile(data_);
    ::CloseHandle(file_mapping_handle_);
    ::CloseHandle(file_handle_);
#else
    ::munmap(const_cast<char*>(data_), size_);
    ::close(file_handle_);
#endif
}
 
MemoryFile::MemoryFile(const char *pathname, e_open_mode open_mode):
    data_(0),
    size_(0),
#if defined(_WIN32)
    file_handle_(INVALID_HANDLE_VALUE),
    file_mapping_handle_(INVALID_HANDLE_VALUE)
#else
    file_handle_(-1)
#endif
{
#if defined(_WIN32)
    int windows_open_mode;
    switch (open_mode)
    {
    case if_exists_fail_else_create:
        windows_open_mode = CREATE_NEW;
        break;
    case if_exists_keep_else_fail:
        windows_open_mode = OPEN_EXISTING;
        break;
    case if_exists_keep_else_create:
        windows_open_mode = OPEN_ALWAYS;
        break;
    case if_exists_truncate_else_fail:
        windows_open_mode = TRUNCATE_EXISTING;
        break;
    case if_exists_truncate_else_create:
        windows_open_mode = CREATE_ALWAYS;
        break;
    default: return;
    }
    const size_t min_file_size = 4096;
    file_handle_ = ::CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
        0, 0, windows_open_mode, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle_ == INVALID_HANDLE_VALUE) return;
    size_t initial_file_size = ::GetFileSize(file_handle_, 0);
    size_t adjusted_file_size = initial_file_size == 0 ? min_file_size : initial_file_size;
    file_mapping_handle_ = ::CreateFileMapping(
        file_handle_, 0, PAGE_READWRITE, 0, adjusted_file_size, 0);
    if (file_mapping_handle_ == INVALID_HANDLE_VALUE) return;
    data_ = static_cast<char*>(::MapViewOfFile(
        file_mapping_handle_, FILE_MAP_WRITE, 0, 0, 0));
    if (data_) {
        size_ = initial_file_size;
        capacity_ = adjusted_file_size;
    }
#else
    int posix_open_mode = O_RDWR;
    switch (open_mode)
    {
    case if_exists_fail_else_create:
        posix_open_mode |= O_EXCL | O_CREAT;
        break;
    case if_exists_keep_else_fail:
        break;
    case if_exists_keep_else_create:
        posix_open_mode |= O_CREAT;
        break;
    case if_exists_truncate_else_fail:
        posix_open_mode |= O_TRUNC;
        break;
    case if_exists_truncate_else_create:
        posix_open_mode |= O_TRUNC | O_CREAT;
        break;
    default: return;
    }
    const size_t min_file_size = 4096;
    file_handle_ = ::open(pathname, posix_open_mode, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (file_handle_ == -1) return;
    struct stat sbuf;
    if (::fstat(file_handle_, &sbuf) == -1) return;
    size_t initial_file_size = sbuf.st_size;
    size_t adjusted_file_size = (initial_file_size == 0) ? min_file_size : initial_file_size;
    ::ftruncate(file_handle_, adjusted_file_size);
    data_ = static_cast<char*>(::mmap(
        0, adjusted_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_handle_, 0));
    if (data_ == MAP_FAILED) data_ = 0;
    else {
        size_ = initial_file_size;
        capacity_ = adjusted_file_size;
    }
#endif
}
 
void MemoryFile::resize(size_t new_size) {
    if (new_size > capacity_) reserve(new_size);
    size_ = new_size;
}
 
void MemoryFile::reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) return;
#if defined(_WIN32)
    ::UnmapViewOfFile(data_);
    ::CloseHandle(file_mapping_handle_);
    file_mapping_handle_ = ::CreateFileMapping(
        file_handle_, 0, PAGE_READWRITE, 0, new_capacity, 0);
    capacity_ = new_capacity;
    data_ = static_cast<char*>(::MapViewOfFile(
        file_mapping_handle_, FILE_MAP_WRITE, 0, 0, 0));
#else
    ::munmap(data_, size_);
    ::ftruncate(file_handle_, new_capacity);
    data_ = static_cast<char*>(::mmap(
        0, new_capacity, PROT_READ | PROT_WRITE, MAP_SHARED, file_handle_, 0));
    if (data_ == MAP_FAILED) data_ = 0;
    capacity_ = new_capacity;
#endif
}
 
MemoryFile::~MemoryFile() {
#if defined(_WIN32)
    ::UnmapViewOfFile(data_);
    ::CloseHandle(file_mapping_handle_);
    if (size_ != capacity_)
    {
        ::SetFilePointer(file_handle_, size_, 0, FILE_BEGIN);
        ::SetEndOfFile(file_handle_);
    }
    ::CloseHandle(file_handle_);
#else
    ::munmap(data_, size_);
    if (size_ != capacity_)
    {
        ::ftruncate(file_handle_, size_);
    }
    ::close(file_handle_);
#endif
}
 
bool MemoryFile::flush() {
#if defined(_WIN32)
    return ::FlushViewOfFile(data_, size_) != 0;
#else
    return ::msync(data_, size_, MS_SYNC) == 0;
#endif
}

