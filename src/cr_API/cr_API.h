#include <stdbool.h>
#include "../internal_cr_API/internal_cr_API.h"


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

/* AUXILIARY METHODS */
int unload_file(char *destination, char *location, DirectoryEntry *file);
int load_file(char *destination, char *location, char *filename);
int unload_folder(char *destination, char *location, DirectoryEntry *file);
int load_folder(char *destination, char *location, char *foldername);
Disk *get_disk();
