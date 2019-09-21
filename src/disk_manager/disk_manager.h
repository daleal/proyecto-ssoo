typedef struct directory_entry DirectoryEntry;

typedef struct block Block;
typedef struct index_block IndexBlock;
typedef struct directory_block DirectoryBlock;


// Disk struct
typedef struct disk {
    DirectoryBlock *index;
    unsigned char bitmap[131072];
    // 1024 chars * 128 bitmap blocks = 131 072 chars of bitmap
} Disk;


Disk *open_disk(char *diskname);
