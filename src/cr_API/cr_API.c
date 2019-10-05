#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../constants/constants.h"
#include "../error_handler/error_handler.h"
#include "../disk_manager/disk_manager.h"
#include "../internal_cr_API/internal_cr_API.h"
#include "cr_API.h"


// File structs
struct crfile {
    Block *block;
    int size;
};


Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char mode)
{
    crFILE *file_desc = malloc(sizeof(crFILE));
    return file_desc;
}


int cr_read(crFILE *file_desc, void *buffer, int nbytes)
{
    
    int read;
    int bytes = file_desc->reader + nbytes;
    if (bytes > file_desc->index->size){
        read = file_desc->index->size - file_desc->reader;
    }
    else read = nbytes;

    unsigned char save[read];
    int data = 32*252;
    int simple = 32*253;
    int doublex = 32*254;
    int triple = 32*255;

    Block *block;
    DirectioningBlock *directoring;
    if (file_desc->reader < data){
        int actual = reader_to_block(file_desc->reader);
        block = go_to_block(disk,file_desc->index->data_blocks[actual]);
    }
    else if (file_desc->reader < simle){
        block = go_to_block(disk, file_desc->index->simple_directioning_block);
        directoring = get_directioning_block(block);
    }
    else if (file_desc->reader < doublex){
        block = go_to_block(disk, file_desc->index->double_directioning_block);
        directoring = get_directioning_block(block);
    }
    else if (file_desc->reader < triple){
        block = go_to_block(disk, file_desc->index->triple_directioning_block);
        directoring = get_directioning_block(block);
    }
    
    for (int i = 0; i <= read; i++){

        if (i + file_desc->reader < data){

            if ((reader + i)%32 == 0) {
                actual = reader_to_block(reader + i);
                block = go_to_block(disk,file_desc->index->data_blocks[actual]);
            }
            save[i] = block[i%32];
        }

        else if (i + file_desc->reader < simple){
            save[i] = directoring[i%32];
        }

        else if (i + file_desc->reader < doublex){
            save[i] = directoring_double[i%32];
        }

        else{
            save[i] = directoring_triple[i%32];
        }
    }

    *buffer = save;
    file_desc->reader = file_desc->reader + read;
    return read;
}


int cr_write(crFILE *file_desc, void *buffer, int nbytes)
{
    return 0;
}


int cr_close(crFILE *file_desc)
{
    free(file_desc);
    return 0;
}


int cr_rm(char *path)
{
    return 0;
}


int cr_unload(char *orig, char *dest)
{
    return 0;
}


int cr_load(char *orig)
{
    return 0;
}


// General API functions
void cr_mount(char *diskname)
{
   if (mounted_disk != NULL) {
        handle_error("A disk is already mounted");
        return;
    }
    mounted_disk = open_disk(diskname);
    if (mounted_disk == NULL) {
        handle_error("Could not mount disk");
    }
}


void cr_unmount()
{
    if (!close_disk(mounted_disk)) {
        log_error("No disk is mounted. Could not unmount disk");
    }
}


void cr_bitmap(unsigned block, bool hex)
{
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return;
    }
    if (block == 0) {
        for (int i = 0; i < BITMAP_BYTES; i++) {
            if (!!hex) {
                fprintf(stderr, "0x%X\n",  mounted_disk->bitmap[i]);
            } else {
                for (int j = 0; j < 8; j++) {
                    fprintf(stderr, "%i ", bit_from_bitmap(mounted_disk, (i * 8) + j));
                }
                fprintf(stderr, "\n");
            }
        }
        fprintf(stderr, "Used blocks: %i\n", used_blocks(mounted_disk));
        fprintf(stderr, "Free blocks: %i\n", free_blocks(mounted_disk));
    } else if (block >= 1 && block <= 129) {
        --block;
        for (int offset = 0; offset < BLOCK_SIZE; offset++) {
            if (!!hex) {
                fprintf(stderr, "0x%X\n", mounted_disk->bitmap[(block * BLOCK_SIZE) + offset]);
            } else {
                for (int j = 0; j < 8; j++) {
                    fprintf(
                        stderr, "%i ",
                        bit_from_bitmap(mounted_disk, (((block * BLOCK_SIZE) + offset) * 8) + j)
                    );
                }
                fprintf(stderr, "\n");
            }
        }
    } else {
        log_error("Invalid bitmap block. Please try again with a number between 1 and 129");
    }
}


int cr_exists(char *path)
{
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return 0;
    }
    char path_duplicate[strlen(path) + 1];
    strcpy(path_duplicate, path);
    int start_point = strlen(path_duplicate);
    Block *raw;
    DirectoryBlock *directory;
    DirectoryEntry *subdirectory;
    // Remove last directory/file from the path
    for (int i = strlen(path_duplicate) - 1; i >= 0; i--) {
        if (path_duplicate[i] == '/') {
            path_duplicate[i] = '\0';
            break;
        } else {
            --start_point;
        }
    }
    // Get incognito file name
    char incognito[strlen(path) - start_point + 1];
    for (int i = 0; i < strlen(path) - start_point; i++) {
        incognito[i] = path[start_point + i];
    }
    incognito[strlen(path) - start_point] = '\0';
    // Find if :incognito exists
    if (!strcmp(path_duplicate, "")) {
        // If path duplicate is now empty call cr_cd with "/"
        raw = cr_cd(mounted_disk, "/");
    } else {
        // If path is not empty
        raw = cr_cd(mounted_disk, path_duplicate);
    }
    if (raw == NULL) {
        // Father of the last file could not be found
        return 0;
    }
    directory = get_directory_block(raw);
    for (int i = 0; i < 32; i++) {
        subdirectory = directory->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            raw = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(directory);
            directory = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status == (unsigned char)2) |    // Directory
            (subdirectory->status == (unsigned char)4) |    // File
            (subdirectory->status == (unsigned char)8) |    // Same Dir
            (subdirectory->status == (unsigned char)16)) {  // Father Dir
            // :subdirectory corresponds to valid entry
            if (!strcmp(subdirectory->name, incognito)) {
                // :subdirectory corresponds to the file we were looking for
                free_directory_block(directory);
                return 1;
            }
        }
    }
    free_directory_block(directory);
    return 0;
}


void cr_ls(char *path)
{
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return;
    }
    Block *raw = cr_cd(mounted_disk, path);
    if (raw == NULL) {
        log_error("No such directory");
        return;
    }
    DirectoryBlock *directory = get_directory_block(raw);
    DirectoryEntry *subdirectory;
    for (int i = 0; i < 32; i++) {
        subdirectory = directory->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            raw = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(directory);
            directory = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status == (unsigned char)2) |    // Directory
            (subdirectory->status == (unsigned char)4) |    // File
            (subdirectory->status == (unsigned char)8) |    // Same Dir
            (subdirectory->status == (unsigned char)16)) {  // Father Dir
            // :subdirectory corresponds to valid entry
            printf("%s\n", subdirectory->name);
        }
    }
    free_directory_block(directory);
}


int cr_mkdir(char *foldername)
{
    return 0;
}

int reader_to_block(int reader)
{
    return reader / 32;
}

