#include <stdio.h>
#include <stdlib.h>
#include "disk_manager.h"


// Directory Entry struct
struct directory_entry {
    unsigned char status;
    // 1:  invalid
    // 2:  valid, corresponds to a directory
    // 4:  valid, corresponds to a file
    // 8:  valid, the same directory
    // 16: invalid, father directory
    // 32: next (same) directory

    char name[27];
    // Empty spaces must be 0

    unsigned int file_pointer;
    // From 0 to 1048575
};


// Directory Block struct
struct directory_block {
    DirectoryEntry *directories[31];
    DirectoryEntry *next_block;
};


// Index Block struct
struct index_block {
    unsigned int size;
    unsigned int data_blocks[252];
    unsigned int simple_directioning_block;
    unsigned int double_directioning_block;
    unsigned int triple_directioning_block;
};


// Vainilla Block struct
struct block {
    unsigned char data[1024];
};


Disk *open_disk(char *diskname)
{
    FILE *raw;
    Disk *disk = malloc(sizeof(Disk));
    unsigned char index_block[1024];
    unsigned char block_buffer[1024];
    raw = fopen(diskname,"rb");
    if (raw == NULL) {
        printf("Unable to find %s disk. Exiting.\n", diskname);
        exit(1);
    }

    // Get index block
    fread(index_block, sizeof(unsigned char), sizeof(index_block), raw);

    // Get bitmap blocks
    fread(disk->bitmap, sizeof(unsigned char), sizeof(disk->bitmap), raw);



    fclose(raw);

    return disk;
}
