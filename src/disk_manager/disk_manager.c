#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "disk_manager.h"


/*
 * Opens a file named :diskname and returns a pointer
 * to a Disk struct containing an interpreted directory
 * block, an array of bytes representing the bitmap and
 * an array of pointers to raw blocks to be interpreted
 * when required.
 * If the :diskname file does not exist
 * or fails to be opened, the function returns a pointer
 * to NULL. Otherwise, the function returns a poitner
 * to a Disk struct.
 */
Disk *open_disk(char *diskname)
{
    FILE *raw;
    raw = fopen(diskname, "rb");
    if (raw == NULL) {
        return NULL;
    }

    Disk *disk = malloc(sizeof(Disk));
    disk->index = malloc(sizeof(Block));

    // Get raw index block
    fread(disk->index, sizeof(unsigned char), BLOCK_SIZE, raw);

    // Get bitmap blocks
    fread(disk->bitmap, sizeof(unsigned char), BITMAP_BYTES, raw);

    for (int i = 0; i < DISK_BLOCKS; i++) {
        disk->blocks[i] = malloc(sizeof(Block));
        fread(disk->blocks[i]->data, sizeof(unsigned char), BLOCK_SIZE, raw);
    }

    fclose(raw);

    return disk;
}


/*
 * Ideally, this method rewrites the disk file with
 * the new disk content (YET TO BE DONE) and then
 * frees the memory of the disk.
 * If the :disk struct poitner points to NULL, the
 * method returns 0. Otherwise, it returns 1.
 */
int close_disk(Disk *disk)
{
    if (disk == NULL) {
        return 0;
    }
    for (int i = 0; i < DISK_BLOCKS; i++) {
        free(disk->blocks[i]);
    }
    free(disk->index);
    free(disk);
    return 1;
}



/* NAVIGATION */

/*
 * The method recieves a Disk struct :disk and an
 * unsigned int :pointer. If :pointer is 0, returns
 * the index of the disk. If :pointer is between
 * 129 (included) and TOTAL_BLOCKS (not included),
 * it offsets the pointer by 129 and returns the
 * block in that place from the array of blocks
 * of :disk. In any other case, the function
 * returns NULLS.
 */
Block *go_to_block(Disk *disk, unsigned int pointer)
{
    if (pointer == 0) {
        // Get the index of the disk
        return disk->index;
    }
    if ((pointer < (unsigned int)129) | (pointer >= (unsigned int)TOTAL_BLOCKS)) {
        // Pointer points to a bitmap block (invalid navigation) or to
        // a block outside of the disk
        return NULL;
    }
    pointer -= 129;  // index block and bitmap blocks
    return disk->blocks[pointer];
}



/* BLOCK MANAGEMENT */

/*
 * The method recieves a raw Block struct :block,
 * interprets it as a DirectoryBlock struct and
 * returns a pointer to it.
 */
DirectoryBlock *get_directory_block(Block *block)
{
    DirectoryBlock *directory_block = malloc(sizeof(DirectoryBlock));
    unsigned char buffer[4];
    // Get entries
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
    return directory_block;
}


/*
 * The method recieves a DirectoryBlock struct
 * :block and frees its memory usage.
 */
void free_directory_block(DirectoryBlock *block)
{
    for (int offset = 0; offset < 32; offset++) {
        free(block->directories[offset]);
    }
    free(block);
}


/*
 * The method recieves a raw Block struct :block,
 * interprets it as an IndexBlock struct and
 * returns a pointer to it.
 */
IndexBlock *get_index_block(Block *block)
{
    IndexBlock *index_block = malloc(sizeof(IndexBlock));
    unsigned char size[4];
    for (int i = 0; i < 4; i++) {
        size[i] = block->data[i];
    }
    int_from_chars(size, &index_block->size);

    unsigned char buffer[4];
    for (int i = 0; i < 252; i++){
        for (int j = 0; j < 4; j++){
            buffer[j] = block->data[4 + (i * 4) +  j];
        }
        int_from_chars(buffer, &index_block->data_blocks[i]);
    }

    unsigned char simple[4];
    for (int i = 1012; i < 1016; i++){
        simple[i%4] = block->data[i];
    }
    int_from_chars(simple, &index_block->simple_directioning_block);

    unsigned char doublex[4];
    for (int i = 1016; i < 1020; i++){
        doublex[i%4] = block->data[i];
    }
    int_from_chars(doublex, &index_block->double_directioning_block);

    unsigned char triple[4];
    for (int i = 1020; i < 1024; i++){
        triple[i%4] = block->data[i];
    }
    int_from_chars(triple, &index_block->triple_directioning_block);

    return index_block;
}


/*
 * The method recieves an IndexBlock struct
 * :block and frees its memory usage.
 */
void free_index_block(IndexBlock *block)
{
    free(block);
}


/*
 * The method recieves a raw Block struct :block,
 * interprets it as a DirectioningBlock struct and
 * returns a pointer to it.
 */
DirectioningBlock *get_directioning_block(Block *block)
{
    DirectioningBlock *directioning_block = malloc(sizeof(DirectioningBlock));
    unsigned char buffer[4];
    for (int pointer = 0; pointer < 256; pointer++) {
        for (int offset = 0; offset < 4; offset++) {
            buffer[offset] = block->data[(pointer * 4) + offset];
        }
        int_from_chars(buffer, &directioning_block->pointers[pointer]);
    }
    return directioning_block;
}


/*
 * The method recieves a DirectioningBlock struct
 * :block and frees its memory usage.
 */
void free_directioning_block(DirectioningBlock *block)
{
    free(block);
}



/* BITMAP MANAGEMENT */

/*
 * The method recieves a pointer to a Disk
 * struct :disk and an integer :position
 * representing the desired bit position
 * inside the bitmap and returns its
 * value (0 or 1).
 */
int bit_from_bitmap(Disk *disk, int position)
{
    int chunk = position / 8;
    int offset = 7 - (position % 8);
    return bit_from_byte(disk->bitmap[chunk], offset);
}


/*
 * The method recieves an unsigned char :byte
 * and an integer :position which represents
 * a bit's position inside :byte (position 0
 * is the least significant bit of the byte).
 * The method returns the bit in the position
 * :position and can be a 0 or a 1.
 */
int bit_from_byte(unsigned char byte, int position)
{
    int bit;
    bit = (byte % (int) (pow(2, position + 1))) / (int) pow(2, position);
    return bit;
}


/*
 * The method recieves a pointer to a Disk
 * struct :disk and an integer :position
 * representing the desired bit position
 * inside the bitmap. If the bit in :position
 * is already a 1, the method returns 0 and
 * changes nothing. If the bit is a 0, the
 * method changes it to a 1 and returns 1.
 */
int turn_bitmap_bit_to_one(Disk *disk, int position)
{
    int chunk = position / 8;
    int offset = 7 - (position % 8);
    if (bit_from_bitmap(disk, position)) {
        return 0;  // bit is already 1, error
    }
    disk->bitmap[chunk] += pow(2, offset);
    return 1;
}


/*
 * The method recieves a pointer to a Disk
 * struct :disk and an integer :position
 * representing the desired bit position
 * inside the bitmap. If the bit in :position
 * is already a 0, the method returns 0 and
 * changes nothing. If the bit is a 1, the
 * method changes it to a 0 and returns 1.
 */
int turn_bitmap_bit_to_zero(Disk *disk, int position)
{
    int chunk = position / 8;
    int offset = 7 - (position % 8);
    if (!bit_from_bitmap(disk, position)) {
        return 0;  // bit is already 0, error
    }
    disk->bitmap[chunk] -= pow(2, offset);
    return 1;
}


/*
 * The method recieves a pointer to a Disk
 * struct :disk and returns the amount of
 * blocks that are marked as used in the
 * bitmap.
 */
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


/*
 * The method recieves a pointer to a Disk
 * struct :disk and returns the amount of
 * blocks that are marked as free in the
 * bitmap.
 */
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



/* BLOCK REVERSE TRANSLATE */

/*
 * The method recieves a pointer to a raw Block
 * and a pointer to a DirectoryBlock. This give
 * the information of the DirectoryBlock to
 * the raw Block.
 */
void reverse_translate_block_directory(DirectoryBlock *interpreted_block, Block *raw_block){ 
    
    int n_byte_raw_block = 0;
    unsigned char index_block_pointer[4];

    for (int n_dir = 0; n_dir < 32; n_dir++)
    {
        // Reverse status
        raw_block->data[n_byte_raw_block] = interpreted_block -> directories[n_dir]->status;
        n_byte_raw_block++;

        // Reverse name
        for (int n_byte_len_name = 0; n_byte_len_name < 27; n_byte_len_name++)
        {
            raw_block->data[n_byte_raw_block] = interpreted_block -> directories[n_dir]->name[n_byte_len_name];
            n_byte_raw_block++;
        }

        // Get index block
        chars_from_int(index_block_pointer, &interpreted_block -> directories[n_dir]->file_pointer);
        
        // Reverse index block pointer
        for (int n_byte_block_pointer = 0; n_byte_block_pointer < 4; n_byte_block_pointer++)
        {
            raw_block->data[n_byte_raw_block] = index_block_pointer[n_byte_block_pointer];
            n_byte_raw_block++;
        }

        // This increase set cero for the next directory
        n_byte_raw_block++;

    }
    
}



/* BIT FIDDELING */

/*
 * The method recieves a pointer to an array
 * of unsigned chars :bytes and a pointer to
 * an unsigned int :integer and reads the first
 * 4 chars of :bytes as an integer into :integer
 * using big endianness.
 */
void int_from_chars(unsigned char *bytes, unsigned int *integer)
{
    *integer = (unsigned int) bytes[3] | ( (int)bytes[2] << 8 ) | ( (int)bytes[1] << 16 ) | ( (int)bytes[0] << 24 );
}


/*
 * The method recieves a pointer to an array
 * of unsigned chars :bytes and a pointer to
 * an unsigned int :integer and reads :integer
 * into the first 4 chars of :bytes using big
 * endianness.
 */
void chars_from_int(unsigned char *bytes, unsigned int *integer)
{
    bytes[0] = (*integer >> 24) & 0xFF;
    bytes[1] = (*integer >> 16) & 0xFF;
    bytes[2] = (*integer >> 8) & 0xFF;
    bytes[3] = *integer & 0xFF;
}
