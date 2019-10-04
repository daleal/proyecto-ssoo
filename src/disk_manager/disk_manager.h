#define BLOCK_SIZE 1024
#define BITMAP_BYTES 131072
// 1024 chars * 128 bitmap blocks = 131 072 chars of bitmap
#define DISK_BLOCKS 1048357
// 1048576 total blocks - 1 index block - 128 bitmap blocks
#define TOTAL_BLOCKS 1048576


typedef struct directory_entry DirectoryEntry;

typedef struct block Block;
typedef struct index_block IndexBlock;
typedef struct directory_block DirectoryBlock;


// Disk struct
typedef struct disk {
    Block *index;
    unsigned char bitmap[BITMAP_BYTES];
    Block *blocks[DISK_BLOCKS];
} Disk;


Disk *open_disk(char *diskname);
int close_disk(Disk *disk);

/* NAVIGATION */
Block *go_to_block(Disk *disk, unsigned int pointer);

/* BLOCK MANAGEMENT */
DirectoryBlock *get_directory_block(Block *block);
void free_directory_block(DirectoryBlock *block);
IndexBlock *get_index_block(Block *block);

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
