#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

struct diffChunk
{
    int pos;
    int length;
    char* diffFile1;
    char* diffFile2;
};

int compare_files(char *ptr1, char *ptr2, int padding);
void get_all_file_paths(const char *base_path, char ***file_paths, int *count, int *capacity);
void concatHex(unsigned char source, char *destination);
