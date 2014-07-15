#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

int main()
{
    int fd = open64("txx", O_CREAT, O_WRONLY, 0644);
    if (fd)
    {
        perror("open");
        return -1;
    }
    off64_t file_size = 30000000000;
    if (ftruncate64(fd, file_size) != 0)
    {
        perror("truncate");
        return -1;
    }
    close(fd);
    return 0;
}

