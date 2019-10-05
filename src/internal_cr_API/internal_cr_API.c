#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../constants/constants.h"
#include "../error_handler/error_handler.h"
#include "../disk_manager/disk_manager.h"
#include "internal_cr_API.h"


Block *cr_cd(Disk *disk, char *path)
{
    char tokenize[strlen(path) + 1];
    strcpy(tokenize, path);
    char *subfolder = strtok(tokenize, "/");  // Remove initial slash
    Block *actual = disk->index;
    DirectoryBlock *interpreted = get_directory_block(actual);
    DirectoryEntry *directory;
    bool found;
    while (subfolder) {
        found = false;
        for (int i = 0; i < 32; i++) {
            directory = interpreted->directories[i];
            if (directory->status == (unsigned char)32) {
                // :directory is the continuation of :interpreted
                actual = go_to_block(disk, directory->file_pointer);
                free_directory_block(interpreted);
                interpreted = get_directory_block(actual);  // Get continuation
                i = -1;  // So the loop starts over with the continuation
            } else if (directory->status == (unsigned char)2) {
                // :directory corresponds to :subfolder
                if (!strcmp(directory->name, subfolder)) {
                    // Directory corresponds to wanted subfolder
                    found = true;
                    actual = go_to_block(disk, directory->file_pointer);
                    free_directory_block(interpreted);
                    interpreted = get_directory_block(actual);
                    break;
                }
            }
        }
        if (!found) {
            // If a matching block was not found during this iteration
            free_directory_block(interpreted);
            return NULL;
        }
        subfolder = strtok(NULL, "/");
    }
    free_directory_block(interpreted);
    return actual;
}
