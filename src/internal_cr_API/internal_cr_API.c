#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../constants/constants.h"
#include "../error_handler/error_handler.h"
#include "../disk_manager/disk_manager.h"
#include "internal_cr_API.h"


Block *cr_cd(Disk *disk, char *path)
{
    char new_path[strlen(path) + 1];
    char filename[strlen(path) + 1];
    split_path(path, new_path, filename);
    bool found = false;
    DirectoryEntry *directory;
    Block *raw = cr_folder_cd(disk, new_path);
    DirectoryBlock *block = get_directory_block(raw);
    if (raw == NULL) {
        return NULL;
    }
    for (int i = 0; i < 32; i++) {
        directory = block->directories[i];
        if ((directory->status == (unsigned char)32) & (i != 0)) {
            // :directory is the continuation of :block
            raw = go_to_block(disk, directory->file_pointer);
            free_directory_block(block);
            block = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (directory->status == (unsigned char)2) |    // Directory
            (directory->status == (unsigned char)8) |    // Same Dir
            (directory->status == (unsigned char)16)) {  // Father Dir
            // :directory corresponds to :subfolder
            if (!strcmp(directory->name, filename)) {
                // Directory corresponds to wanted subfolder
                found = true;
                raw = go_to_block(disk, directory->file_pointer);
                free_directory_block(block);
                block = get_directory_block(raw);
                break;
            }
        }
    }
    if (!found) {
        // If a matching block was not found during this iteration
        free_directory_block(block);
        return NULL;
    } else {
        // In case that :raw is the continuation of another directory
        while (block->directories[0]->status == (unsigned char)32) {
            raw = go_to_block(disk, block->directories[0]->file_pointer);
            free_directory_block(block);
            block = get_directory_block(raw);
        }
        free_directory_block(block);
        return raw;
    }
}


/*
 * This method returns the directory block of
 * :path assuming :path is a folder path.
 */
Block *cr_folder_cd(Disk *disk, char *path)
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
            if ((directory->status == (unsigned char)32) & (i != 0)) {
                // :directory is the continuation of :interpreted
                actual = go_to_block(disk, directory->file_pointer);
                free_directory_block(interpreted);
                interpreted = get_directory_block(actual);  // Get continuation
                i = -1;  // So the loop starts over with the continuation
            } else if (
                (directory->status == (unsigned char)2) |    // Directory
                (directory->status == (unsigned char)8) |    // Same Dir
                (directory->status == (unsigned char)16)) {  // Father Dir
                // :directory corresponds to :subfolder
                if (!strcmp(directory->name, subfolder)) {
                    // Directory corresponds to wanted subfolder
                    found = true;
                    actual = go_to_block(disk, directory->file_pointer);
                    free_directory_block(interpreted);
                    interpreted = get_directory_block(actual);
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
    // In case that :actual is the continuation of another directory
    while (interpreted->directories[0]->status == (unsigned char)32) {
        actual = go_to_block(disk, interpreted->directories[0]->file_pointer);
        free_directory_block(interpreted);
        interpreted = get_directory_block(actual);
    }
    free_directory_block(interpreted);
    return actual;
}


/*
 * This method splits :path by its last separator
 * into :new_path and :filename, :new_path being
 * the first part of the string and :filename
 * being the last part of the string.
 */
void split_path(char *path, char *new_path, char *filename)
{
    strcpy(new_path, path);
    int total_length;
    if ((strlen(new_path) > 1) & (new_path[strlen(new_path) - 1] == '/')) {
        new_path[strlen(new_path) - 1] = '\0';
    }
    total_length = strlen(new_path);
    int start_point = strlen(new_path);
    // Remove last directory/file from the path
    for (int i = strlen(new_path) - 1; i >= 0; i--) {
        if (new_path[i] == '/') {
            new_path[i] = '\0';
            break;
        } else {
            --start_point;
        }
    }
    // Get filename
    for (int i = 0; i < total_length - start_point; i++) {
        filename[i] = path[start_point + i];
    }
    filename[total_length - start_point] = '\0';
    // Check border cases
    if (!strcmp(new_path, "")) {
        strcpy(new_path, "/");
    }
    if (!strcmp(filename, "")) {
        strcpy(filename, ".");
    }
}
