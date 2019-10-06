#define MAX_DISKNAME_LENGTH 256

#define BLOCK_SIZE 1024
#define TOTAL_BLOCKS 1048576
#define BITMAP_BYTES BLOCK_SIZE * 128
// BLOCK_SIZE * 128 bitmap blocks
#define DISK_BLOCKS TOTAL_BLOCKS - 128 - 1
// 1048576 total blocks - 1 index block - 128 bitmap blocks
