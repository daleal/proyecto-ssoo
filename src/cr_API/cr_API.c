#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "../error_handler/error_handler.h"
#include "cr_API.h"


Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char mode)
{
    char folder[strlen(path) + 1];
    char filename[strlen(path) + 1];
    split_path(path, folder, filename);
    Block *raw_father = cr_folder_cd(mounted_disk, folder);
    if (raw_father == NULL) {
        log_error("Invalid path to file");
        return NULL;
    }
    if (mode == 'r') {
        if (!cr_exists(path)) {
            log_error("Invalid file");
            return NULL;
        }
        DirectoryBlock *dir = get_directory_block(raw_father);
        DirectoryEntry *subdirectory;

        for (int i = 0; i < 32; i++) {
            subdirectory = dir->directories[i];
            if (subdirectory->status == (unsigned char)32) {
                // :subdirectory is the continuation of :directory
                raw_father = go_to_block(mounted_disk, subdirectory->file_pointer);
                free_directory_block(dir);
                dir = get_directory_block(raw_father);  // Get continuation
                i = -1;  // So the loop starts over with the continuation
            } else if (
                (subdirectory->status == (unsigned char)4) &&  // File
                !strcmp(subdirectory->name, filename)) {
                // File to open
                crFILE *file = malloc(sizeof(crFILE));
                file->raw_index = go_to_block(mounted_disk, subdirectory->file_pointer);
                file->index = get_index_block(file->raw_index);
                file->reader = 0;
                file->reading = 1;
                free_directory_block(dir);
                return file;
            }
        }
        log_error("Could not find requested file");
        return NULL;
    } else if (mode == 'w') {
        if (cr_exists(path)) {
            log_error("File already exists");
            return NULL;
        }

        DirectoryBlock *dir = get_directory_block(raw_father);
        DirectoryEntry *subdirectory;

        for (int i = 0; i < 32; i++) {
            subdirectory = dir->directories[i];
            if (subdirectory->status == (unsigned char)32) {
                // :subdirectory is the continuation of :directory
                raw_father = go_to_block(mounted_disk, subdirectory->file_pointer);
                free_directory_block(dir);
                dir = get_directory_block(raw_father);  // Get continuation
                i = -1;  // So the loop starts over with the continuation
            } else if ((i == 31) && (subdirectory->status != (unsigned char)32)) {
                subdirectory->file_pointer = create_directory_extension(mounted_disk);
                // Save changed directory
                reverse_translate_directory_block(dir, raw_father);
                // Get continuation
                raw_father = go_to_block(mounted_disk, subdirectory->file_pointer);
                free_directory_block(dir);
                dir = get_directory_block(raw_father);  // Get continuation
                i = -1;  // So the loop starts over with the continuation
            } else if (
                (subdirectory->status != (unsigned char)2) &&
                (subdirectory->status != (unsigned char)4) &&
                (subdirectory->status != (unsigned char)8) &&
                (subdirectory->status != (unsigned char)16) &&
                (subdirectory->status != (unsigned char)32)) {
                // Empty entry
                unsigned int index_block_pointer;
                // Create the new index block for the file
                if (!(index_block_pointer = new_index_block(mounted_disk))) {
                    free_directory_block(dir);
                    log_error("No disk space left");
                    return NULL;
                }
                crFILE *file = malloc(sizeof(crFILE));
                fill_directory_name(subdirectory->name, filename);
                subdirectory->status = (unsigned char)4;
                subdirectory->file_pointer = index_block_pointer;
                // Save changed directory
                reverse_translate_directory_block(dir, raw_father);
                file->raw_index = go_to_block(mounted_disk, subdirectory->file_pointer);
                file->index = get_index_block(file->raw_index);
                file->reader = 0;
                file->reading = 0;
                free_directory_block(dir);
                return file;
            }
        }
    } else {
        log_error("Invalid file mode");
        return NULL;
    }
    log_error("Invalid action");
    return NULL;
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
    expand_file(mounted_disk, file_desc, nbytes);
    unsigned long counter = 0;
    unsigned char *data = buffer;
    unsigned char *location;
    while (nbytes--) {
        location = get_file_byte(mounted_disk, file_desc, counter);
        *location = data[file_desc->index->size++];
        ++counter;
    }
    file_desc->index->size = counter;
    reverse_translate_index_block(file_desc->index, file_desc->raw_index);
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
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return 0;
    }
    int str_len = 0;
    while (path[str_len] != '\0')
    str_len++;
    char new_path[str_len+1];
    char file[str_len+1];
    split_path(path, new_path, file);
    Block *raw = cr_folder_cd(mounted_disk, new_path);
    if (raw == NULL) {
        log_error("No such directory");
        return 0;
    }
    raw = cr_cd(mounted_disk, path);
    if (raw == NULL){
        log_error("No such file");
        return 0;
    }
    DirectoryBlock *directory = get_directory_block(raw);
    unsigned int dir_pointer = get_directory_pointer(mounted_disk, directory);
    DirectoryEntry *subdirectory;
    for (int i = 0; i < 32; i++) {
        subdirectory = directory->directories[i];
        if (subdirectory->status == (unsigned char)32) {
            raw = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(directory);
            directory = get_directory_block(raw);
            i = -1;
        } else if ((subdirectory->status == (unsigned char)4) && !strcmp(subdirectory->name,file))
        {
            turn_bitmap_bit_to_zero(mounted_disk, subdirectory->file_pointer);
            Block *index_raw = go_to_block(mounted_disk,subdirectory->file_pointer);
            IndexBlock *index = get_index_block(index_raw);
            int size = 0;
            for (int i=0;i<256;i++){
                turn_bitmap_bit_to_zero(mounted_disk, index->data_blocks[i]);
                size += 1024;
                if (size >= index->size){
                    break;
                }
                if (i==252){
                    Block *direc_simp_raw = go_to_block(mounted_disk,index->simple_directioning_block);
                    DirectioningBlock *direc_simp=  get_directioning_block(direc_simp_raw);
                    turn_bitmap_bit_to_zero(mounted_disk, index->simple_directioning_block);
                    for (int j = 0; j<256;j++){
                        turn_bitmap_bit_to_zero(mounted_disk, direc_simp->pointers[i]);
                        size += 1024;
                        if (size >= index->size){
                            break;
                        }
                    }
                    free_directioning_block(direc_simp);
                } else if (i==253){
                    Block *direc_doble_raw = go_to_block(mounted_disk, index->double_directioning_block);
                    DirectioningBlock *direc_doble=  get_directioning_block(direc_doble_raw);
                    turn_bitmap_bit_to_zero(mounted_disk, index->double_directioning_block);
                    for (int j = 0; j<256;j++){
                        Block *direc_simp_doble_raw = go_to_block(mounted_disk, direc_doble->pointers[j]);
                        DirectioningBlock *direc_simp_doble=  get_directioning_block(direc_simp_doble_raw);
                        turn_bitmap_bit_to_zero(mounted_disk, direc_doble->pointers[j]);
                        for (int k=0;k<256;k++){
                            turn_bitmap_bit_to_zero(mounted_disk, direc_simp_doble->pointers[k]);
                            size += 1024;
                            if (size >= index->size){
                                break;
                            }
                        }
                        free_directioning_block(direc_simp_doble);
                        if (size >= index->size){
                            break;
                        }
                    }
                    free_directioning_block(direc_doble);
                } else if (i==254){
                    Block *direc_triple_raw = go_to_block(mounted_disk, index->triple_directioning_block);
                    DirectioningBlock *direc_triple=  get_directioning_block(direc_triple_raw);
                    turn_bitmap_bit_to_zero(mounted_disk, index->triple_directioning_block);
                    for (int j = 0;j<256;j++){
                        Block *direc_doble_triple_raw = go_to_block(mounted_disk, direc_triple->pointers[j]);
                        DirectioningBlock *direc_doble_triple=  get_directioning_block(direc_doble_triple_raw);
                        turn_bitmap_bit_to_zero(mounted_disk, direc_triple->pointers[j]);
                        for (int k=0;k<256;k++){
                            Block *direc_simple_doble_triple_raw = go_to_block(mounted_disk, direc_doble_triple->pointers[k]);
                            DirectioningBlock *direc_simple_doble_triple=  get_directioning_block(direc_simple_doble_triple_raw);
                            turn_bitmap_bit_to_zero(mounted_disk, direc_doble_triple->pointers[k]);
                            for(int l=0;l<256;l++){
                                turn_bitmap_bit_to_zero(mounted_disk, direc_simple_doble_triple->pointers[l]);
                                size += 1024;
                                if (size >= index->size){
                                    break;
                                }
                            }
                            free_directioning_block(direc_simple_doble_triple);
                            if (size >= index->size){
                                break;
                            }
                        }
                        free_directioning_block(direc_doble_triple);
                        if (size >= index->size){
                            break;
                        }
                    }
                    free_directioning_block(direc_triple);
                }
                if (size >= index->size){
                    break;
                }
            }
            free_index_block(index);
            subdirectory->status = (unsigned char)0;
            subdirectory->file_pointer = (unsigned char)0;
            int pos = 0;
            while (subdirectory->name[pos] != '\0'){
                subdirectory->name[pos] = '\0';
                pos++;
            }
            Block* raw_dir = go_to_block(mounted_disk, dir_pointer);
            reverse_translate_directory_block(directory, raw_dir);
        }
    }
    return 1;
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
 * :destination and a string :filename
 * and copies the file in :location/:filename
 * to the virtual filesystem inside :destination.
 */
int load_file(char *destination, char *location, char *filename)
{
    /*
     * :location corresponds to the real system location
     * without the filename attached and :full_path
     * corresponds to the real system location
     * with the filename attached.
     * :destination corresponds to the virtual system location
     * without the filename attached and :virtual_path
     * corresponds to the virtual system location
     * with the filename attached.
     */
    char full_path[strlen(location) + 27 + 2];
    sprintf(full_path, "%s/%s", location, filename);

    char virtual_path[strlen(destination) + 27 + 2];
    sprintf(virtual_path, "%s/%s", destination, filename);

    if (cr_exists(virtual_path)) {
        // File already exists
        // char log[256 + strlen(full_path)];
        // sprintf(log, "Could not unload file. File %s already exists", full_path);
        // log_error(log);
        return 0;
    }

    // Open files
    crFILE *writing_file = cr_open(virtual_path, 'w');
    FILE *reading_file = fopen(full_path, "rb");

    if (reading_file == NULL) {
        // Real file does not exist
        return -1;
    }

    fseek(reading_file, 0, SEEK_END);
    long fsize = ftell(reading_file);
    fseek(reading_file, 0, SEEK_SET);

    // Get file data in the buffer
    unsigned char *buffer = malloc((fsize + 1) * sizeof(unsigned char));
    fread(buffer, sizeof(unsigned char), fsize, reading_file);

    // Write data in virtual file
    cr_write(writing_file, buffer, fsize);

    // Close files
    free(buffer);
    cr_close(writing_file);
    fclose(reading_file);

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
    free_directory_block(dir);

    return 1;
}


/*
 * The method recieves a destination path
 * :destination and a DirectoryEntry struct
 * :folder_entry and copies the folder pointed
 * by :folder_entry to the real filesystem
 * inside :destination.
 */
int load_folder(char *destination, char *location, char *foldername)
{
    /*
     * :destination and :location don't include the
     * folder's name, they are only the place where
     * the folder lives in the real computer and where
     * it will live in the virtual disk.
     * :virtual_path and :full_path include the name
     * of the folder once created
     */
    char path_start[strlen(location) + 10];
    strcpy(path_start, location);
    if (!strcmp(location, "")) {
        strcpy(path_start, ".");
    }
    char full_path[strlen(path_start) + 27 + 2];
    sprintf(full_path, "%s/%s", path_start, foldername);

    char virtual_path[strlen(location) + 27 + 2];
    sprintf(virtual_path, "%s/%s", location, foldername);

    if (cr_exists(virtual_path)) {
        // Directory already exists
        // char log[256 + strlen(full_path)];
        // sprintf(log, "Could not unload directory. Directory %s already exists", full_path);
        // log_error(log);
        return 0;
    }

    if (!cr_mkdir(full_path)) {
        // Failed to create the directory
        return -1;
    }

    DIR *d;
    struct dirent *dir;
    d = opendir(full_path);

    if (d == NULL) {
        return 0;
    }
    while ((dir = readdir(d)) != NULL) {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {

        } else {
            char sub_path[strlen(dir->d_name) + 27 + 2];
            sprintf(sub_path, "%s/%s", full_path, dir->d_name);
            struct stat st;
            stat(sub_path, &st);
            if (S_ISDIR(st.st_mode)) {
                // Directory
                load_folder(virtual_path, full_path, dir->d_name);
            } else {
                // File
                load_file(virtual_path, full_path, dir->d_name);
            }
            // dir->d_name;  // Do something
        }
    }

    free(d);

    return 1;
}
