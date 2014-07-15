// Originally taken from:
// http://en.wikibooks.org/wiki/Optimizing_C%2B%2B/General_optimization_techniques/Input/Output#Memory-mapped_file
// To whoever wrote this: thanks for the handy code snippet and cool free book!
#ifndef MEMORY_FILE_HPP
#define MEMORY_FILE_HPP
 
#include <stddef.h> // or <cstddef> and using std::size_t
 
/*
  Read-only memory-mapped file wrapper.
  It handles only files that can be wholly loaded
  into the address space of the process.
  The constructor opens the file, the destructor closes it.
  The "data" function returns a pointer to the beginning of the file,
  if the file has been successfully opened, otherwise it returns 0.
  The "size" function returns the length of the file in bytes,
  if the file has been successfully opened, otherwise it returns 0.
*/
class InputMemoryFile {
public:
    explicit InputMemoryFile(const char *pathname);
    ~InputMemoryFile();
    const char* data() const { return data_; }
    size_t size() const { return size_; }
private:
    const char* data_;
    size_t size_;
#if defined(_WIN32)
    typedef void* HANDLE;
    HANDLE file_handle_;
    HANDLE file_mapping_handle_;
#else
    int file_handle_;
#endif
};
 
/*
  Read/write memory-mapped file wrapper.
  It handles only files that can be wholly loaded
  into the address space of the process.
  The constructor opens the file, the destructor closes it.
  The "data" function returns a pointer to the beginning of the file,
  if the file has been successfully opened, otherwise it returns 0.
  The "size" function returns the initial length of the file in bytes,
  if the file has been successfully opened, otherwise it returns 0.
  Afterwards it returns the size the physical file will get if it is closed now.
  The "resize" function changes the number of bytes of the significant
  part of the file. The resulting size can be retrieved
  using the "size" function.
  The "reserve" grows the physical file to the specified number of bytes.
  The size of the resulting file can be retrieved using "capacity".
  Memory mapped files cannot be shrunk;
  a value smaller than the current capacity is ignored.
  The "capacity()" function return the size the physical file has at this time.
  The "flush" function ensure that the disk is updated
  with the data written in memory.
*/
class MemoryFile {
public:
    enum e_open_mode {
         if_exists_fail_else_create
        ,if_exists_keep_else_fail
        ,if_exists_keep_else_create
        ,if_exists_truncate_else_fail
        ,if_exists_truncate_else_create
    };
    MemoryFile(const char *pathname, e_open_mode open_mode);
    ~MemoryFile();
    char* data() { return data_; }
    void resize(size_t new_size);
    void reserve(size_t new_capacity);
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool flush();
private:
    char* data_;
    size_t size_;
    size_t capacity_;
#if defined(_WIN32)
    typedef void * HANDLE;
    HANDLE file_handle_;
    HANDLE file_mapping_handle_;
#else
    int file_handle_;
#endif
};
#endif // MEMORY_FILE_HPP

