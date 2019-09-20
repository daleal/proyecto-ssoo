typedef struct disk Disk;
typedef struct directory_block DirectoryBlock;
typedef struct index_block IndexBlock;
typedef struct block Block;


typedef struct directory_entry DirectoryEntry;


Disk *open_disk(char *diskname);
