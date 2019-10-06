#include <stdbool.h>
#include "../disk_manager/disk_manager.h"
#include "../internal_cr_API/internal_cr_API.h"




// File structs
typedef struct crfile{
    IndexBlock *index;
    int reader; 
} crFILE;

// File management functions
crFILE *cr_open(char *path, char mode);
int cr_read(crFILE *file_desc, void *buffer, int nbytes);
int cr_write(crFILE *file_desc, void *buffer, int nbytes);
int cr_close(crFILE *file_desc);
int cr_rm(char *path);
int cr_unload(char *orig, char *dest);
int cr_load(char *orig);

// General API functions
void cr_mount(char *diskname);
void cr_unmount();
void cr_bitmap(unsigned block, bool hex);
int cr_exists(char *path);
void cr_ls(char *path);
int cr_mkdir(char *foldername);

//Auxiliary functions
int min(int a, int b);
int reader_to_block(int reader);
int directioning_to_block(int reader, int mode, int pointer);
int get_index(int reader, int mode);