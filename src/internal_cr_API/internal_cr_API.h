#include "../disk_manager/disk_manager.h"


// File structs
typedef struct crfile {
    IndexBlock *index;
    Block *raw_index;
    unsigned long reader;
    int reading;  // 0: writing, 1: reading, 2: already written
} crFILE;


Block *cr_cd(Disk *disk, char *path);
Block *cr_folder_cd(Disk *disk, char *path);
void split_path(char *path, char *new_path, char *filename);
unsigned char *get_file_byte(Disk *disk, crFILE *file_desc, unsigned long position);
int expand_file(Disk *disk, crFILE *file, unsigned long size);
unsigned long new_file_block_amount(unsigned long size);
