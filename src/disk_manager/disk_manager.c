#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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


int bit_from_bitmap(Disk *disk, int position)
{
    int chunk = position / 8;
    int offset = 7 - (position % 8);
    return bit_from_byte(disk->bitmap[chunk], offset);
}


int bit_from_byte(unsigned char byte, int position)
{
    int bit;
    bit = (byte % (int) (pow(2, position + 1))) / (int) pow(2, position);
    return bit;
}


int used_blocks(Disk *disk)
{
    int used = 0;
    for (int i = 0; i < BITMAP_BYTES; i++) {
        for (int j = 0; j < 8; j++) {
            if (bit_from_bitmap(disk, (i * 8) + j)) {
                ++used;
            }
        }
    }
    return used;
}


int free_blocks(Disk *disk)
{
    int free = 0;
    for (int i = 0; i < BITMAP_BYTES; i++) {
        for (int j = 0; j < 8; j++) {
            if (!bit_from_bitmap(disk, (i * 8) + j)) {
                ++free;
            }
        }
    }
    return free;
}
