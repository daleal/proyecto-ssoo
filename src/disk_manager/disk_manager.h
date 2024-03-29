#include "../constants/constants.h"


/***************************************************
 *                     STRUCTS                     *
 ***************************************************/

// Directory Entry struct
typedef struct directory_entry {
    unsigned char status;
    // 1:  invalid
    // 2:  valid, corresponds to a directory
    // 4:  valid, corresponds to a file
    // 8:  same directory
    // 16: father directory
    // 32: next (same) directory

    char name[27];
    // Empty spaces must be 0

    unsigned int file_pointer;
    // From 0 to 1048575
} DirectoryEntry;


// Vainilla Block struct
typedef struct block {
    unsigned char data[BLOCK_SIZE];
} Block;


// Directory Block struct
typedef struct directory_block {
    DirectoryEntry *directories[32];
} DirectoryBlock;


// Index Block struct
typedef struct index_block {
    unsigned int size;
    unsigned int data_blocks[252];
    unsigned int simple_directioning_block;
    unsigned int double_directioning_block;
    unsigned int triple_directioning_block;
} IndexBlock;


// Directioning Block struct
typedef struct directioning_block {
    unsigned int pointers[256];
} DirectioningBlock;


// Disk struct
typedef struct disk {
    char diskname[MAX_DISKNAME_LENGTH];
    Block *index;
    unsigned char bitmap[BITMAP_BYTES];
    Block *blocks[DISK_BLOCKS];
} Disk;

/****************************************************
 *                   END OF STRUCTS                 *
 ****************************************************/


Disk *open_disk(char *diskname);
int close_disk(Disk *disk);
void save_disk(Disk *disk);
void free_disk(Disk *disk);

/* NAVIGATION */
Block *go_to_block(Disk *disk, unsigned int pointer);

/* BLOCK MANAGEMENT */
DirectoryBlock *get_directory_block(Block *block);
void free_directory_block(DirectoryBlock *block);
IndexBlock *get_index_block(Block *block);
void free_index_block(IndexBlock *block);
DirectioningBlock *get_directioning_block(Block *block);
void free_directioning_block(DirectioningBlock *block);

/* BLOCK REVERSE TRANSLATE */
void reverse_translate_directory_block(DirectoryBlock *interpreted_block, Block *raw_block);
void reverse_translate_index_block(IndexBlock *interpreted_block, Block *raw_block);
void reverse_translate_directioning_block(DirectioningBlock *interpreted_block, Block *raw_block);

/* BITMAP MANAGEMENT */
int bit_from_bitmap(Disk *disk, int position);
int bit_from_byte(unsigned char byte, int position);
int turn_bitmap_bit_to_one(Disk *disk, int position);
int turn_bitmap_bit_to_zero(Disk *disk, int position);
unsigned int get_free_block_number(Disk *disk);
int used_blocks(Disk *disk);
int free_blocks(Disk *disk);

/* BIT FIDDELING */
void int_from_chars(unsigned char *bytes, unsigned int *integer);
void chars_from_int(unsigned char *bytes, unsigned int *integer);

/* HELPER METHODS */
void fill_directory_name(char *name_buffer, char *name);
unsigned int get_directory_pointer(Disk *disk, DirectoryBlock *dir);
unsigned int new_directory_block(Disk *disk, unsigned int father_pointer);
unsigned int create_directory_extension(Disk *disk);
unsigned int new_index_block(Disk *disk);
