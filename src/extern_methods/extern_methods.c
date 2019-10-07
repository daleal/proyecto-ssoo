#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../error_handler/error_handler.h"
#include "../cr_API/cr_API.h"
#include "extern_methods.h"


int cr_cat(char *path)
{
    crFILE *file = cr_open(path, 'r');
    if (file == NULL) {
        log_error("No such file");
        return 0;
    }
    char *buffer = calloc(file->index->size + 1, sizeof(unsigned char));
    cr_read(file, buffer, file->index->size);
    cr_close(file);
    printf("%s\n", buffer);
    free(buffer);
    return 1;
}


void snap()
{
    recursive_snap(get_disk(), 0, "/");
}


void recursive_snap(Disk *disk, unsigned int pointer, char *dir_path)
{
    bool dust = true;
    char aux[strlen(dir_path) + 27 + 1];
    Block *raw = go_to_block(disk, pointer);
    DirectoryBlock *dir = get_directory_block(raw);
    DirectoryEntry *subdirectory;

    for (int i = 0; i < 32; i++) {
        subdirectory = dir->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            raw = go_to_block(disk, subdirectory->file_pointer);
            free_directory_block(dir);
            dir = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (subdirectory->status == (unsigned char)2) {  // Re-Snap
            // Dir to Re-Snap
            sprintf(aux, "%s/%s", dir_path, subdirectory->name);
            recursive_snap(disk, subdirectory->file_pointer, aux);
        } else if (subdirectory->status == (unsigned char)4) {  // Dust?
            // File to dust?
            if (dust) {
                sprintf(aux, "%s/%s", dir_path, subdirectory->name);
                cr_rm(aux);
                printf("File dusted: %s\n", aux);
            }
            dust = !dust;
        }
    }
    free_directory_block(dir);
}
