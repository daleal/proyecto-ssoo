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
    DirectoryEntry *directories[32];
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
    unsigned char data[BLOCK_SIZE];
};


Disk *open_disk(char *diskname)
{
    FILE *raw;
    raw = fopen(diskname, "rb");
    if (raw == NULL) {
        return NULL;
    }

    Disk *disk = malloc(sizeof(Disk));
    Block *index = malloc(sizeof(Block));

    // Get raw index block
    fread(index->data, sizeof(unsigned char), BLOCK_SIZE, raw);

    // Get bitmap blocks
    fread(disk->bitmap, sizeof(unsigned char), BITMAP_BYTES, raw);

    // Get index block
    disk->index = get_directory_block(index);

    for (int i = 0; i < DISK_BLOCKS; i++) {
        disk->blocks[i] = malloc(sizeof(Block));
        fread(disk->blocks[i]->data, sizeof(unsigned char), BLOCK_SIZE, raw);
    }

    fclose(raw);
    free(index);

    return disk;
}


int close_disk(Disk *disk)
{
    if (disk == NULL) {
        return 0;
    }
    for (int i = 0; i < DISK_BLOCKS; i++) {
        free(disk->blocks[i]);
    }
    for (int i = 0; i < 32; i++) {
        free(disk->index->directories[i]);
    }
    free(disk->index);
    free(disk);
    return 1;
}



/* BLOCK CONVERSIONS */

DirectoryBlock *get_directory_block(Block *block)
{
    DirectoryBlock *directory_block = malloc(sizeof(DirectoryBlock));
    unsigned char *buffer = malloc(sizeof(unsigned char) * 4);
    for (int offset = 0; offset < 32; offset++) {
        directory_block->directories[offset] = malloc(sizeof(DirectoryEntry));
        // Get entry status
        directory_block->directories[offset]->status = block->data[32 * offset];
        // Get entry name
        for (int i = 0; i < 28; i++) {
            directory_block->directories[offset]->name[i] = block->data[(32 * offset) + i + 1];
        }
        // Get index block pointer
        for (int i = 0; i < 4; i++) {
            buffer[i] = block->data[(32 * offset) + 28 + i];
        }
        int_from_chars(buffer, &directory_block->directories[offset]->file_pointer);
    }
    free(buffer);
    return directory_block;
}


IndexBlock *get_index_block(Block *block)
{
    IndexBlock *index_block = malloc(sizeof(IndexBlock));
    unsigned char *buffer = malloc(sizeof(unsigned char) * 4);
    for (int i = 0; i < 4; i++) {
        buffer[i] = block->data[i];
    }
    int_from_chars(buffer, &index_block->size);
    free(buffer);
    return index_block;
}



/* BITMAP MANAGEMENT */

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



/* BIT FIDDELING */

void int_from_chars(unsigned char *bytes, unsigned int *integer)
{
    *integer = (unsigned int) bytes[3] | ( (int)bytes[2] << 8 ) | ( (int)bytes[1] << 16 ) | ( (int)bytes[0] << 24 );
}


void chars_from_int(unsigned char *bytes, unsigned int *integer)
{
    bytes[0] = (*integer >> 24) & 0xFF;
    bytes[1] = (*integer >> 16) & 0xFF;
    bytes[2] = (*integer >> 8) & 0xFF;
    bytes[3] = *integer & 0xFF;
}
