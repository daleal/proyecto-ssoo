#include <stdbool.h>


Block *cr_cd(Disk *disk, char *path);
Block *cr_folder_cd(Disk *disk, char *path);
void split_path(char *path, char *new_path, char *filename);
