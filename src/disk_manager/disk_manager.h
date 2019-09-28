#define BLOCK_SIZE 1024
#define BITMAP_BYTES 131072
// 1024 chars * 128 bitmap blocks = 131 072 chars of bitmap
#define DISK_BLOCKS 1048357
// 1048576 total blocks - 1 index block - 128 bitmap blocks


typedef struct directory_entry DirectoryEntry;

typedef struct block Block;
typedef struct index_block IndexBlock;
typedef struct directory_block DirectoryBlock;


// Disk struct
typedef struct disk {
    IndexBlock *index;
    unsigned char bitmap[BITMAP_BYTES];
    Block *blocks[DISK_BLOCKS];
} Disk;


Disk *open_disk(char *diskname);
int close_disk(Disk *disk);

/* BLOCK CONVERSIONS */
IndexBlock *get_index_block(Block *block);

/* BITMAP MANAGEMENT */
int bit_from_bitmap(Disk *disk, int position);
int bit_from_byte(unsigned char byte, int position);
int used_blocks(Disk *disk);
int free_blocks(Disk *disk);

/* BIT FIDDELING */
void int_from_chars(unsigned char *bytes, unsigned int *integer);
void chars_from_int(unsigned char *bytes, unsigned int *integer);
