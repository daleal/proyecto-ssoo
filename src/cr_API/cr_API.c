#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../error_handler/error_handler.h"
#include "../disk_manager/disk_manager.h"
#include "../internal_cr_API/internal_cr_API.h"
#include "cr_API.h"


// File structs
struct crfile {
    IndexBlock *index;
    unsigned long reader;
};


Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char mode)
{
    crFILE *file_desc = malloc(sizeof(crFILE));
    // Block *raw = go_to_block(mounted_disk, 0);
    // DirectoryBlock *dir = get_directory_block(raw);
    // for (int i = 0; i < 32; i++) {
    //     printf(
    //         "%u - %s - %u\n",
    //         dir->directories[i]->status,
    //         dir->directories[i]->name,
    //         dir->directories[i]->file_pointer
    //     );
    // }

    // Block *raw = go_to_block(mounted_disk, 706386);
    // IndexBlock *index = get_index_block(raw);
    // FILE *raw_file;
    // crFILE *file = malloc(sizeof(crFILE));
    // file->index = index;
    // file->reader = (unsigned long)0;

    // unsigned char buffer[index->size];
    // cr_read(file, buffer, index->size);
    // raw_file = fopen("Withered Leaf.mp3", "wb");
    // fwrite(buffer, sizeof(unsigned char), index->size, raw_file);
    // fclose(raw_file);

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
        data_read[read++] = *get_file_byte(file_desc, file_desc->reader);
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
            location = get_file_byte(file_desc, counter);
            *location = data[file_desc->index->size++];
            ++counter;
        }
    }
    return counter;
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
            (subdirectory->status == (unsigned char)2) |    // Directory
            (subdirectory->status == (unsigned char)4) |    // File
            (subdirectory->status == (unsigned char)8) |    // Same Dir
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
    if (mounted_disk == NULL) {
        log_error("No disk is mounted");
        return 0;
    }
    char new_path[strlen(foldername) + 1];
    char filename[strlen(foldername) + 1];
    Block *raw_father;
    DirectoryBlock *father;
    unsigned int father_pointer;
    unsigned int actual_pointer;
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
    actual_pointer = father_pointer;

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
        if ((subdirectory->status != (unsigned char)32) & (i == 31)) {
            // Could not find empty entry and there are no more entries
            unsigned int extension_pointer;
            if (!(extension_pointer = create_directory_extension(mounted_disk, actual_pointer))) {
                log_error("No disk space left");
                if (!turn_bitmap_bit_to_zero(mounted_disk, new_dir_pointer)) {
                    log_error("Could not free unused block. Drive might be malfunctioning");
                }
                return 0;
            }
            subdirectory->status = (unsigned char)32;
            subdirectory->file_pointer = extension_pointer;
            i = -1;  // So the loop starts over with the continuation
        } else if ((subdirectory->status == (unsigned char)32) & (i != 0)) {
            // :subdirectory is the continuation of :father
            actual_pointer = subdirectory->file_pointer;
            raw_father = go_to_block(mounted_disk, subdirectory->file_pointer);
            free_directory_block(father);
            father = get_directory_block(raw_father);  // Get continuation
            i = -1;  // So the loop starts over with the continuation
        } else if (
            (subdirectory->status != (unsigned char)2) &    // Directory
            (subdirectory->status != (unsigned char)4) &    // File
            (subdirectory->status != (unsigned char)8) &    // Same Dir
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
 * The method recieves a Disk struct pointer :disk,
 * a crFILE struct pointer :file_desc and an unsigned
 * long :position. It then returns a pointer to the
 * byte corresponding to :position inside of the file
 * :file_desc.
 */
unsigned char *get_file_byte(crFILE *file_desc, unsigned long position)
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
        aux = go_to_block(mounted_disk, index->data_blocks[data_block]);
        return &aux->data[offset];
    } else if (position < SIMPLE_DIRECT_BYTES_LIMIT) {
        // Offset to the simple directioning block
        position -= DATA_BYTES_LIMIT;
        aux = go_to_block(mounted_disk, index->simple_directioning_block);
        simple = get_directioning_block(aux);
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(mounted_disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    } else if (position < DOUBLE_DIRECT_BYTES_LIMIT) {
        // Offset to the double directioning block
        position -= SIMPLE_DIRECT_BYTES_LIMIT;
        aux = go_to_block(mounted_disk, index->double_directioning_block);
        doublex = get_directioning_block(aux);
        simple_block = position / (BLOCK_SIZE * 256);
        aux = go_to_block(mounted_disk, doublex->pointers[simple_block]);
        free_directioning_block(doublex);
        simple = get_directioning_block(aux);
        // Offset to the simple directioning block
        position -= BLOCK_SIZE * 256 * simple_block;
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(mounted_disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    } else {
        // Offset to the triple directioning block
        position -= DOUBLE_DIRECT_BYTES_LIMIT;
        aux = go_to_block(mounted_disk, index->triple_directioning_block);
        triple = get_directioning_block(aux);
        doublex_block = position / (BLOCK_SIZE * 256 * 256);
        aux = go_to_block(mounted_disk, triple->pointers[doublex_block]);
        free_directioning_block(triple);
        doublex = get_directioning_block(aux);
        // Offset to the double directioning block
        position -= BLOCK_SIZE * 256 * 256 * doublex_block;
        simple_block = position / (BLOCK_SIZE * 256);
        aux = go_to_block(mounted_disk, doublex->pointers[simple_block]);
        free_directioning_block(doublex);
        simple = get_directioning_block(aux);
        // Offset to the simple directioning block
        position -= BLOCK_SIZE * 256 * simple_block;
        data_block = position / BLOCK_SIZE;
        offset = position % BLOCK_SIZE;
        aux = go_to_block(mounted_disk, simple->pointers[data_block]);
        free_directioning_block(simple);
        return &aux->data[offset];
    }
}


/*
 * The method recieves a destination path
 * :destination and a DirectoryEntry struct
 * :file_entry and copies the file pointed
 * by :file_entry to the real filesystem
 * inside :destination.
 */
int unload_file(char *destination, char *location, DirectoryEntry *file_entry)
{
    char full_path[strlen(destination) + 27 + 2];
    sprintf(full_path, "%s/%s", destination, file_entry->name);

    char virtual_path[strlen(location) + 27 + 2];
    sprintf(virtual_path, "%s/%s", location, file_entry->name);

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
    unsigned char buffer[reading_file->index->size];
    cr_read(reading_file, buffer, reading_file->index->size);

    // Write data in real file
    fwrite(buffer, sizeof(unsigned char), reading_file->index->size, writing_file);

    // Close files
    cr_close(reading_file);
    fclose(writing_file);
}


/*
 * The method recieves a destination path
 * :destination and a DirectoryEntry struct
 * :folder_entry and copies the folder pointed
 * by :folder_entry to the real filesystem
 * inside :destination.
 */
int unload_folder(char *destination, char *location, DirectoryEntry *folder_entry)
{
    char full_path[strlen(destination) + 27 + 2];
    sprintf(full_path, "%s/%s", destination, folder_entry->name);

    char virtual_path[strlen(location) + 27 + 2];
    sprintf(virtual_path, "%s/%s", location, folder_entry->name);

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
    unsigned char buffer[reading_file->index->size];
    cr_read(reading_file, buffer, reading_file->index->size);

    // Write data in real file
    fwrite(buffer, sizeof(unsigned char), reading_file->index->size, writing_file);

    // Close files
    cr_close(reading_file);
    fclose(writing_file);
}
