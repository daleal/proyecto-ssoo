#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../error_handler/error_handler.h"
#include "internal_cr_API.h"


Block *cr_cd(Disk *disk, char *path)
{
    char new_path[strlen(path) + 1];
    char filename[strlen(path) + 1];
    split_path(path, new_path, filename);
    bool found = false;
    DirectoryEntry *directory;
    Block *raw = cr_folder_cd(disk, new_path);
    Block *father = raw;
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
                father = raw;
                break;
            }
        }
    }
    if (!found) {
        // If a matching block was not found during this iteration
        free_directory_block(block);
        return NULL;
    } else {
        free_directory_block(block);
        return father;
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
    Block *father = actual;
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
                    father = actual;
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
    return father;
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


/*
 * The method recieves a Disk struct pointer :disk,
 * a crFILE struct pointer :file_desc and an unsigned
 * long :position. It then returns a pointer to the
 * byte corresponding to :position inside of the file
 * :file_desc.
 */
unsigned char *get_file_byte(Disk *disk, crFILE *file_desc, unsigned long position)
{
    Block *aux;
    DirectioningBlock *simple;
    DirectioningBlock *doublex;
    DirectioningBlock *triple;
    IndexBlock *index = file_desc->index;
    int offset;
    int data_block;
    int simple_block;
    int doublex_block;
    if (position < DATA_BYTES_LIMIT) {
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(disk, index->data_blocks[data_block]);
        return &aux->data[offset];
    } else if (position < SIMPLE_DIRECT_BYTES_LIMIT) {
        // Offset to the simple directioning block
        position -= DATA_BYTES_LIMIT;
        aux = go_to_block(disk, index->simple_directioning_block);
        simple = get_directioning_block(aux);
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    } else if (position < DOUBLE_DIRECT_BYTES_LIMIT) {
        // Offset to the double directioning block
        position -= SIMPLE_DIRECT_BYTES_LIMIT;
        aux = go_to_block(disk, index->double_directioning_block);
        doublex = get_directioning_block(aux);
        simple_block = position / (BLOCK_SIZE * 256);
        aux = go_to_block(disk, doublex->pointers[simple_block]);
        free_directioning_block(doublex);
        simple = get_directioning_block(aux);
        // Offset to the simple directioning block
        position -= BLOCK_SIZE * 256 * simple_block;
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    } else {
        // Offset to the triple directioning block
        position -= DOUBLE_DIRECT_BYTES_LIMIT;
        aux = go_to_block(disk, index->triple_directioning_block);
        triple = get_directioning_block(aux);
        doublex_block = position / (BLOCK_SIZE * 256 * 256);
        aux = go_to_block(disk, triple->pointers[doublex_block]);
        free_directioning_block(triple);
        doublex = get_directioning_block(aux);
        // Offset to the double directioning block
        position -= BLOCK_SIZE * 256 * 256 * doublex_block;
        simple_block = position / (BLOCK_SIZE * 256);
        aux = go_to_block(disk, doublex->pointers[simple_block]);
        free_directioning_block(doublex);
        simple = get_directioning_block(aux);
        // Offset to the simple directioning block
        position -= BLOCK_SIZE * 256 * simple_block;
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    }
}


int expand_file(crFILE *file, unsigned long size)
{
    if (size < DATA_BYTES_LIMIT) {

    } else {
        // Fill data blocks
    }
    if (size < SIMPLE_DIRECT_BYTES_LIMIT) {

    } else {
        // Fill simple directioning block
    }
    if (size < DOUBLE_DIRECT_BYTES_LIMIT) {

    } else {
        // Fill double directioning block
    }
    if (size < TRIPLE_DIRECT_BYTES_LIMIT) {

    }
}


/*
 * This method recieves an unsigned long integer
 * :size and returns the amount of blocks (not
 * counting the index block) that a file of that
 * size should occupy.
 */
unsigned long new_file_block_amount(unsigned long size)
{
    unsigned long blocks = 0;
    if (size < DATA_BYTES_LIMIT) {
        blocks += size / BLOCK_SIZE;
        blocks += ((size % BLOCK_SIZE) != 0);
        return blocks;
    } else {
        // Fill data blocks
        blocks += 252;
    }
    blocks += 1;  // Simple Directioning Block
    if (size < SIMPLE_DIRECT_BYTES_LIMIT) {
        size -= DATA_BYTES_LIMIT;
        blocks += size / BLOCK_SIZE;
        blocks += ((size % BLOCK_SIZE) != 0);
        return blocks;
    } else {
        // Fill simple directioning block
        blocks += 256;
    }
    blocks += 1;  // Double Directioning Block
    if (size < DOUBLE_DIRECT_BYTES_LIMIT) {
        size -= SIMPLE_DIRECT_BYTES_LIMIT;
        blocks += size / (BLOCK_SIZE * 256);
        blocks += ((size % (BLOCK_SIZE * 256)) != 0);
        // Offset to the simple directioning block
        size -= BLOCK_SIZE * 256 * (size / (BLOCK_SIZE * 256));
        blocks += size / BLOCK_SIZE;
        blocks += ((size % BLOCK_SIZE) != 0);
        return blocks;
    } else {
        // Fill double directioning block
        blocks += (256 + 256 * 256);
    }
    blocks += 1;  // Triple Directioning Block
    if (size < TRIPLE_DIRECT_BYTES_LIMIT) {
        size -= DOUBLE_DIRECT_BYTES_LIMIT;
        blocks += size / (BLOCK_SIZE * 256 * 256);
        blocks += ((size % (BLOCK_SIZE * 256 * 256)) != 0);
        // Offset to the double directioning block
        size -= BLOCK_SIZE * 256 * 256 * (size / (BLOCK_SIZE * 256 * 256));
        blocks += size / (BLOCK_SIZE * 256);
        blocks += ((size % (BLOCK_SIZE * 256)) != 0);
        // Offset to the simple directioning block
        size -= BLOCK_SIZE * 256 * (size / (BLOCK_SIZE * 256));
        blocks += size / BLOCK_SIZE;
        blocks += ((size % BLOCK_SIZE) != 0);
        return blocks;
    }
    return blocks + 256 + 256 * 256 + 256 * 256 * 256;  // Never happening
}
