#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../error_handler/error_handler.h"
#include "cr_API.h"


Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char mode)
{

    crFILE *file_desc = malloc(sizeof(crFILE));

    // Variables for case of root
    char* path_for_split;
    char* extension = "/";
    int path_has_backlas = 0;
    // Review if have backslas
    for (int i = 0; i < strlen(path); i++)
    {
        if (path[i]=='/')
        {
            path_has_backlas = 1;
        }

    }
    // In case dont have one, make a concatenate in the begin
    // If have one, copy the same path
    if (!path_has_backlas)
    {
        path_for_split = malloc(strlen(extension)+1+strlen(path));
        strcpy(path_for_split, extension);
        strcat(path_for_split, path);
    } else
    {
        path_for_split = malloc(1+strlen(path));
        strcpy(path_for_split, path);
    }



    char new_path[strlen(path_for_split) + 1];

    // First split the path
    char filename[strlen(path_for_split) + 1];
    split_path(path_for_split, new_path, filename);
    // Free the path for split
    free(path_for_split);

    // Condition in case the disk is unmounted
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return 0;
    }

    // Second give the folder path to a raw block
    Block *raw =  cr_folder_cd(mounted_disk, new_path);

    // Variable for a conditional filename
    int exist_file_name = 0;
    int find_it_file_name = 0;

    // Variable for save the number of the directory
    // in the directory block of filename
    int n_directory_file_name;

    // Variable for write mode, save the number of a invalid directory
    int n_directory_invalid;
    int capture_invalid = 0;

    // Variable for write mode, use for save the pointer for create new index
    // block and the index block
    unsigned int extension_pointer;
    unsigned int new_index_block_pointer;

    // Condition in case not found the folder path
    if (raw == NULL)
    {
        log_error("No such directory, please re-write path.");
        return NULL;
    }
    // Third search the filename in the directory block
    DirectoryBlock *block = get_directory_block(raw);
    for (int n_dir = 0; n_dir < 32; n_dir++)
    {
        if (!strcmp(block->directories[n_dir]->name, filename) && block->directories[n_dir]->status == (unsigned char)4)
        {
            exist_file_name = 1;
            // Save the number directory
            n_directory_file_name = n_dir;
            find_it_file_name = 1;
        }

        // Conditional only for write mode

        if ((block->directories[n_dir]->status != (unsigned char)2) &    // Directory
        (block->directories[n_dir]->status != (unsigned char)4) &    // File
        (block->directories[n_dir]->status != (unsigned char)8) &    // Same Dir
        (block->directories[n_dir]->status != (unsigned char)16) && !capture_invalid)
        {
            n_directory_invalid = n_dir;
            capture_invalid = 1;
        }

        // Conditional in case of search in the extension of directory
        if (block->directories[n_dir]->status == (unsigned char)32 && n_dir == 31)
        {
            if (!find_it_file_name)
            {
                // Create a raw block with the extension directory
                raw = go_to_block(mounted_disk, block->directories[n_dir]->file_pointer);
                free_directory_block(block);
                block = get_directory_block(raw);
                // Start the loop after de identificator
                n_dir = -1;
                continue;
            }

        }

        // Conditional only for write mode, in case need extension
        if (block->directories[n_dir]->status == (unsigned char)1 && n_dir == 31)
        {
            n_directory_invalid = -1;
        }


    }
    // Forth Check the mode
    if (mode == 'r')
    {
        // Condition in case not found the filename
        if (exist_file_name)
        {
            unsigned int file_pointer = block->directories[n_directory_file_name]->file_pointer;
            // Go to the index block of the file and give to cr_FILE
            Block *raw_index_file = go_to_block(mounted_disk, file_pointer);
            file_desc -> index = get_index_block(raw_index_file);
            file_desc -> reader = 0;
        }
        else
        {
            log_error("No such file in that directory, please re-write path.");
            return NULL;
        }
    } else if (mode == 'w')
    {
        if (exist_file_name)
        {
            log_error("File already exists");
            return NULL;
        }
        else
        {
            // Conditional in case found a invalid directory
            if (n_directory_invalid != -1)
            {
                unsigned int file_pointer = block->directories[n_directory_invalid]->file_pointer;
                // Create the new index block for the file
                if (!(new_index_block_pointer = new_index_block(mounted_disk))) {
                    log_error("No disk space left");
                    return NULL;
                }
                // Set the invalid entry like a valid one
                // Rembember n_directory_invalid have number
                // of a invalid directory, make it valid
                block->directories[n_directory_invalid]->status = (unsigned char)2;
                fill_directory_name(block->directories[n_directory_invalid]->name, filename);
                block->directories[n_directory_invalid]->file_pointer = file_pointer;

                // Go to the index block of the file and give to cr_FILE
                Block *raw_index_file = go_to_block(mounted_disk, file_pointer);
                file_desc -> index = get_index_block(raw_index_file);
                file_desc -> reader = 0;
            }
            // Extension of the directory block
            else
            {
                // Create the extension
                if (!(extension_pointer = create_directory_extension(mounted_disk))) {
                    log_error("No disk space left");
                    return NULL;
                }
                // Create the new index block for the file
                if (!(new_index_block_pointer = new_index_block(mounted_disk))) {
                    log_error("No disk space left");
                    return NULL;
                }
                // Change the status the directory and give
                // the information to the new pointer
                block->directories[31]->status = (unsigned char)32;
                block->directories[31]->file_pointer = extension_pointer;
                // Create a raw block with the extension directory
                raw = go_to_block(mounted_disk, block->directories[31]->file_pointer);
                free_directory_block(block);
                block = get_directory_block(raw);
                // Set the first entry
                block->directories[1]->status = (unsigned char)2;
                fill_directory_name(block->directories[1]->name, filename);
                block->directories[1]->file_pointer = new_index_block_pointer;
                    // Go to the index block of the file and give to cr_FILE
                Block *raw_index_file = go_to_block(mounted_disk, new_index_block_pointer);
                file_desc -> index = get_index_block(raw_index_file);
                file_desc -> reader = 0;

            }

        }
    } else
    {
        log_error("Invalid mode.");
        return NULL;
    }


    return file_desc;
}


int cr_read(crFILE *file_desc, void *buffer, int nbytes)
{
    if (file_desc == NULL) {
        log_error("Invalid file pointer");
        return -1;
    }
    int read = 0;
    unsigned char *data_read = buffer;
    if (file_desc->reader + nbytes > file_desc->index->size) {
        nbytes = file_desc->index->size - file_desc->reader;
    }
    while (nbytes--) {
        data_read[read++] = *get_file_byte(mounted_disk, file_desc, file_desc->reader);
        ++file_desc->reader;
    }
    return read;
}


int cr_write(crFILE *file_desc, void *buffer, int nbytes)
{
    if (file_desc == NULL) {
        log_error("Invalid file pointer");
        return -1;
    }
    file_desc->index->size = 0;
    unsigned long counter = 0;
    unsigned char *data = buffer;
    unsigned char *location;
    while (nbytes--) {
        if (data[file_desc->index->size] == '\0') {
            break;
        } else {
            location = get_file_byte(mounted_disk, file_desc, counter);
            *location = data[file_desc->index->size++];
            ++counter;
        }
    }
    return counter;
}


int cr_close(crFILE *file_desc)
{
    if (file_desc == NULL){
        return 0;
    }

    free(file_desc->index);
    free(file_desc);
    return 1;
}


int cr_rm(char *path)
{
    return 0;
}


void aux()
{
    cr_unload("/memes", "./");
}


int cr_unload(char *orig, char *dest)
{
    bool exists = false;
    char orig_path[strlen(orig) + 1];
    char filename[strlen(orig) + 1];
    Block *actual_raw_folder;
    DirectoryBlock *actual_folder;
    DirectoryEntry *subdirectory;
    split_path(orig, orig_path, filename);
    actual_raw_folder = cr_folder_cd(mounted_disk, orig_path);
    // Check if destination exists
    struct stat st = {0};
    if (stat(dest, &st) == -1) {
        log_error("Invalid destination folder");
        return 0;
    }

    actual_folder = get_directory_block(actual_raw_folder);

    // Check if virtual dir exists
    if (actual_folder == NULL) {
        log_error("Invalid virtual origin");
        return 0;
    }

    for (int i = 0; i < 32; i++) {
        subdirectory = actual_folder->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            actual_raw_folder = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(actual_folder);
            actual_folder = get_directory_block(actual_raw_folder);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status == (unsigned char)2) &&
            (!strcmp(subdirectory->name, filename))) {  // Directory
            // Dir to copy
            exists = true;
            unload_folder(dest, orig_path, subdirectory);
            break;
        } else if (
            (subdirectory->status == (unsigned char)4) &&
            (!strcmp(subdirectory->name, filename))) {  // File
            // File to copy
            exists = true;
            unload_file(dest, orig_path, subdirectory);
            break;
        }
    }

    free_directory_block(actual_folder);

    if (!exists) {
        log_error("Invalid virtual origin");
        return 0;
    }

    // recursive_unload(orig, dest);

    return 1;
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
    char new_path[strlen(path) + 1];
    char filename[strlen(path) + 1];
    Block *raw;
    DirectoryBlock *directory;
    DirectoryEntry *subdirectory;
    split_path(path, new_path, filename);
    raw = cr_folder_cd(mounted_disk, new_path);
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
            (subdirectory->status == (unsigned char)2) ||    // Directory
            (subdirectory->status == (unsigned char)4) ||    // File
            (subdirectory->status == (unsigned char)8) ||    // Same Dir
            (subdirectory->status == (unsigned char)16)) {  // Father Dir
            // :subdirectory corresponds to valid entry
            if (!strcmp(subdirectory->name, filename)) {
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
            (subdirectory->status == (unsigned char)2) ||    // Directory
            (subdirectory->status == (unsigned char)4) ||    // File
            (subdirectory->status == (unsigned char)8) ||    // Same Dir
            (subdirectory->status == (unsigned char)16)) {  // Father Dir
            // :subdirectory corresponds to valid entry
            printf("%s\n", subdirectory->name);
        }
    }
    free_directory_block(directory);
}


int cr_mkdir(char *foldername)
{
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return 0;
    }
    char new_path[strlen(foldername) + 1];
    char filename[strlen(foldername) + 1];
    Block *raw_father;
    DirectoryBlock *father;
    unsigned int father_pointer;
    unsigned int new_dir_pointer;
    DirectoryEntry *subdirectory;
    split_path(foldername, new_path, filename);
    raw_father = cr_folder_cd(mounted_disk, new_path);
    if (raw_father == NULL) {
        // Father could not be found
        return 0;
    }
    father = get_directory_block(raw_father);
    father_pointer = get_directory_pointer(mounted_disk, father);

    // Create new dir
    if (!(new_dir_pointer = new_directory_block(mounted_disk, father_pointer))) {
        log_error("No disk space left");
        return 0;
    }

    Block *raw = cr_folder_cd(mounted_disk, new_path);
    DirectoryBlock *directory = get_directory_block(raw);

    // Check if folder already exists
    for (int i = 0; i < 32; i++) {
        subdirectory = directory->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            raw = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(directory);
            directory = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (subdirectory->status == (unsigned char)2) {
            // :subdirectory corresponds to valid directory
            if (!strcmp(subdirectory->name, filename)) {
                // :subdirectory has the same name that we want to put to
                // our new folder
                free_directory_block(directory);
                log_error("Folder already exists");
                return 0;
            }
        }
    }
    free_directory_block(directory);

    // Fill father dir
    for (int i = 0; i < 32; i++) {
        subdirectory = father->directories[i];
        if ((subdirectory->status != (unsigned char)32) && (i == 31)) {
            // Could not find empty entry and there are no more entries
            unsigned int extension_pointer;
            if (!(extension_pointer = create_directory_extension(mounted_disk))) {
                log_error("No disk space left");
                if (!turn_bitmap_bit_to_zero(mounted_disk, new_dir_pointer)) {
                    log_error("Could not free unused block. Drive might be malfunctioning");
                }
                return 0;
            }
            subdirectory->status = (unsigned char)32;
            subdirectory->file_pointer = extension_pointer;
            i = -1;  // So the loop starts over with the continuation
        } else if ((subdirectory->status == (unsigned char)32) && (i != 0)) {
            // :subdirectory is the continuation of :father
            raw_father = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(father);
            father = get_directory_block(raw_father);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status != (unsigned char)2) &&    // Directory
            (subdirectory->status != (unsigned char)4) &&    // File
            (subdirectory->status != (unsigned char)8) &&    // Same Dir
            (subdirectory->status != (unsigned char)16)) {  // Father Dir
            // :subdirectory corresponds to invalid entry
            subdirectory->status = (unsigned char)2;
            fill_directory_name(subdirectory->name, filename);
            subdirectory->file_pointer = new_dir_pointer;

            // Translate father block to raw block
            reverse_translate_directory_block(father, raw_father);
            break;
        }
    }

    free_directory_block(father);

    return 1;
}



/* AUXILIARY METHODS */


/*
 * The method recieves a destination path
 * :destination and a DirectoryEntry struct
 * :file_entry and copies the file pointed
 * by :file_entry to the real filesystem
 * inside :destination.
 */
int unload_file(char *destination, char *location, DirectoryEntry *file)
{
    /*
     * :destination corresponds to the real system location
     * without the filename attached and :full_path
     * corresponds to the real system location
     * with the filename attached.
     * :location corresponds to the virtual system location
     * without the filename attached and :virtual_path
     * corresponds to the virtual system location
     * with the filename attached.
     */
    char full_path[strlen(destination) + 27 + 2];
    sprintf(full_path, "%s/%s", destination, file->name);

    char virtual_path[strlen(location) + 27 + 2];
    sprintf(virtual_path, "%s/%s", location, file->name);

    if (access(full_path, F_OK) != -1) {
        // File already exists
        // char log[256 + strlen(full_path)];
        // sprintf(log, "Could not unload file. File %s already exists", full_path);
        // log_error(log);
        return 0;
    }

    // Open files
    crFILE *reading_file = cr_open(virtual_path, 'r');
    FILE *writing_file = fopen(full_path, "wb");

    if (reading_file == NULL) {
        // Virtual file does not exist
        printf("INVALID: %s\n", virtual_path);
        return -1;
    }

    // Get file data in the buffer
    unsigned char *buffer = malloc(reading_file->index->size * sizeof(unsigned char));
    cr_read(reading_file, buffer, reading_file->index->size);

    // Write data in real file
    fwrite(buffer, sizeof(unsigned char), reading_file->index->size, writing_file);

    // Close files
    free(buffer);
    cr_close(reading_file);
    fclose(writing_file);

    return 1;
}


/*
 * The method recieves a destination path
 * :destination and a DirectoryEntry struct
 * :folder_entry and copies the folder pointed
 * by :folder_entry to the real filesystem
 * inside :destination.
 */
int unload_folder(char *destination, char *location, DirectoryEntry *file)
{
    /*
     * :destination and :location don't include the
     * folder's name, they are only the place where
     * the folder lives in the virtual disk and where
     * it will live in the real computer.
     * :virtual_path and :full_path include the name
     * of the folder once created
     */
    char path_start[strlen(destination) + 10];
    strcpy(path_start, destination);
    if (!strcmp(destination, "")) {
        strcpy(path_start, ".");
    }
    char full_path[strlen(path_start) + 27 + 2];
    sprintf(full_path, "%s/%s", path_start, file->name);

    char virtual_path[strlen(location) + 27 + 2];
    sprintf(virtual_path, "%s/%s", location, file->name);

    // Struct stat
    struct stat st = {0};

    if (stat(full_path, &st) != -1) {
        // Directory already exists
        // char log[256 + strlen(full_path)];
        // sprintf(log, "Could not unload directory. Directory %s already exists", full_path);
        // log_error(log);
        return 0;
    }

    if (mkdir(full_path, S_IRWXU) == -1) {
        // Failed to create the directory
        return -1;
    }

    Block *raw = go_to_block(mounted_disk, file->file_pointer);
    DirectoryBlock *dir = get_directory_block(raw);
    DirectoryEntry *subdirectory;

    for (int i = 0; i < 32; i++) {
        subdirectory = dir->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            raw = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(dir);
            dir = get_directory_block(raw);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (subdirectory->status == (unsigned char)2) {  // Directory
            // Dir to copy
            unload_folder(full_path, virtual_path, subdirectory);
        } else if (subdirectory->status == (unsigned char)4) {  // File
            // File to copy
            unload_file(full_path, virtual_path, subdirectory);
        }
    }

    return 1;
}


void recursive_unload(char *orig, char *dest)
{
    char orig_path[strlen(orig) + 1];
    char filename[strlen(orig) + 1];
    Block *actual_raw_folder;
    DirectoryBlock *actual_folder;
    DirectoryEntry *subdirectory;
    split_path(orig, orig_path, filename);
    actual_raw_folder = cr_folder_cd(mounted_disk, orig_path);
    actual_folder = get_directory_block(actual_raw_folder);
    for (int i = 0; i < 32; i++) {
        subdirectory = actual_folder->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            // :subdirectory is the continuation of :directory
            actual_raw_folder = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(actual_folder);
            actual_folder = get_directory_block(actual_raw_folder);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status == (unsigned char)2) &&
            (!strcmp(subdirectory->name, filename))) {  // Directory
            // Dir to copy
            unload_folder(dest, orig_path, subdirectory);
        } else if (
            (subdirectory->status == (unsigned char)4) &&
            (!strcmp(subdirectory->name, filename))) {  // File
            // File to copy
            unload_file(dest, orig_path, subdirectory);
        }
    }
    free_directory_block(actual_folder);
}
