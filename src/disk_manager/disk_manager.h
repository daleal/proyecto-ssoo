#define BLOCK_SIZE 1024
#define BITMAP_BYTES 131072
// 1024 chars * 128 bitmap blocks = 131 072 chars of bitmap
#define DISK_BLOCKS 1048357
// 1048576 total blocks - 1 index block - 128 bitmap blocks
#define TOTAL_BLOCKS 1048576


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
    Block *index;
    unsigned char bitmap[BITMAP_BYTES];
    Block *blocks[DISK_BLOCKS];
} Disk;

/****************************************************
 *                   END OF STRUCTS                 *
 ****************************************************/


Disk *open_disk(char *diskname);
int close_disk(Disk *disk);

/* NAVIGATION */
Block *go_to_block(Disk *disk, unsigned int pointer);

/* BLOCK MANAGEMENT */
DirectoryBlock *get_directory_block(Block *block);
void free_directory_block(DirectoryBlock *block);
IndexBlock *get_index_block(Block *block);
DirectioningBlock *get_directioning_block(Block *block);
void free_directioning_block(DirectioningBlock *block);

/* BLOCK REVERSE TRANSLATE */
void reverse_translate_block_directory(DirectoryBlock *interpreted_block, Block *raw_block);

/* BITMAP MANAGEMENT */
int bit_from_bitmap(Disk *disk, int position);
int bit_from_byte(unsigned char byte, int position);
int turn_bitmap_bit_to_one(Disk *disk, int position);
int turn_bitmap_bit_to_zero(Disk *disk, int position);
int used_blocks(Disk *disk);
int free_blocks(Disk *disk);

/* BIT FIDDELING */
void int_from_chars(unsigned char *bytes, unsigned int *integer);
void chars_from_int(unsigned char *bytes, unsigned int *integer);
