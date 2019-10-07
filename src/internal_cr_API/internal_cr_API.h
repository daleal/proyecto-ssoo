#include "../disk_manager/disk_manager.h"


// File structs
typedef struct crfile {
    IndexBlock *index;
    unsigned long reader;
    int reading ;
} crFILE;


Block *cr_cd(Disk *disk, char *path);
Block *cr_folder_cd(Disk *disk, char *path);
void split_path(char *path, char *new_path, char *filename);
unsigned char *get_file_byte(Disk *disk, crFILE *file_desc, unsigned long position);
