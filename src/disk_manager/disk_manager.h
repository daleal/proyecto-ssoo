#define BLOCK_SIZE 1024
#define BITMAP_BYTES 131072
// 1024 chars * 128 bitmap blocks = 131 072 chars of bitmap


typedef struct directory_entry DirectoryEntry;

typedef struct block Block;
typedef struct index_block IndexBlock;
typedef struct directory_block DirectoryBlock;


// Disk struct
typedef struct disk {
    DirectoryBlock *index;
    unsigned char bitmap[BITMAP_BYTES];
} Disk;


Disk *open_disk(char *diskname);
int bit_from_bitmap(Disk *disk, int position);
int bit_from_byte(unsigned char byte, int position);
int used_blocks(Disk *disk);
int free_blocks(Disk *disk);
