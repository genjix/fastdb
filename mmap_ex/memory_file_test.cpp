// Originally taken from:
// http://en.wikibooks.org/wiki/Optimizing_C%2B%2B/General_optimization_techniques/Input/Output#Memory-mapped_file
// To whoever wrote this: thanks for the handy code snippet and cool free book!
#include "memory_file.hpp"
#include <iostream> // for std::cerr
 
// TODO review interface, reader cannot tell what CopyFile(backupfile, preciousfile, false) will do
bool CopyFile(const char* source, const char* dest, bool overwrite)
{
    InputMemoryFile source_mf(source);
    if (! source_mf.data()) return false;
    MemoryFile dest_mf(dest, overwrite ?
        MemoryFile::if_exists_truncate_else_create :
        MemoryFile::if_exists_fail_else_create);
    if (! dest_mf.data()) return false;
    dest_mf.resize(source_mf.size());
    if (source_mf.size() != dest_mf.size()) return false;
    std::copy(source_mf.data(), source_mf.data() + source_mf.size(),
        dest_mf.data());
    return true;
}
 
int main() {
    if (! CopyFile("memory_file_test.cpp", "copy.tmp", true)) { 
        std::cerr << "Copy failed" << std::endl;
        return 1;
    }
}

