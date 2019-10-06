#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cr_API.h"

#include "../constants/constants.h"
#include "../error_handler/error_handler.h"





Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char mode)
{
    crFILE *file_desc = malloc(sizeof(crFILE));
    return file_desc;
}


int cr_read(crFILE *file_desc, void *buffer, int nbytes)
{
    
    int total;
    int bytes = file_desc->reader + nbytes;
    if (bytes > file_desc->index->size){
        total = file_desc->index->size - file_desc->reader;
    }
    else total = nbytes;

    unsigned char save[total];
    int data = BLOCK_SIZE * 252;
    int simple = BLOCK_SIZE * NUM_POINTERS + data;
    int doublex = BLOCK_SIZE * pow(NUM_POINTERS, 2) + simple;
    int triple = BLOCK_SIZE * pow(NUM_POINTERS, 3) + doublex;

    Block *block;
    Block *data_block;
    Block *simple_block;
    Block *double_block;
    DirectioningBlock *directioning;
    DirectioningBlock *directioning_double;
    DirectioningBlock *directioning_triple;

    int actual;
    int actual_pointer;
    int simple_pointer;
    int index_data = 0;
    int index_simple = 0;
    int index_double = 0;
    int index_triple = 0;



    if (file_desc->reader < data){
        index_data = min(data - file_desc->reader, total);
        actual = reader_to_block(file_desc->reader);
        block = go_to_block(mounted_disk,file_desc->index->data_blocks[actual]);

        for (int i = 0; i < index_data; i++){
            if ((i + file_desc->reader)%BLOCK_SIZE == 0){
                actual = reader_to_block(file_desc->reader+i);
                block = go_to_block(mounted_disk, file_desc->index->data_blocks[actual]);
            }
            save[i] = block->data[get_index(file_desc->reader + i, 0)];
        }
        file_desc->reader = file_desc->reader + index_data;
    } 

    if ((data < file_desc->reader) && (file_desc->reader < simple))
    {
        index_simple = min(simple - file_desc->reader, total - index_data);
        
        block = go_to_block(mounted_disk, file_desc->index->simple_directioning_block);
        directioning = get_directioning_block(block);
        actual = directioning_to_block(file_desc->reader, 1, 1);
        data_block = go_to_block(mounted_disk, directioning->pointers[actual]); 

        for (int i = 0; i < index_simple; i++){
            if ((i + file_desc->reader)%BLOCK_SIZE == 0){
                actual = directioning_to_block(file_desc->reader + i, 1, 1);
                data_block = go_to_block(mounted_disk, directioning->pointers[actual]);
            }

            save[i] = data_block->data[get_index(i + file_desc->reader, 1)];
        }
        file_desc->reader = file_desc->reader + index_simple;   
    }

    if ((simple < file_desc->reader) && (file_desc->reader < doublex)){
        index_double = min(doublex - file_desc->reader, total - index_simple);
        
        block = go_to_block(mounted_disk, file_desc->index->double_directioning_block);
        directioning_double = get_directioning_block(block);
        actual_pointer = directioning_to_block(file_desc->reader, 2, 2);
        simple_block = go_to_block(mounted_disk, directioning_double->pointers[actual_pointer]);
        directioning = get_directioning_block(simple_block);
        actual = directioning_to_block(file_desc->reader, 2, 1);
        data_block = go_to_block(mounted_disk, directioning->pointers[actual]);

        for (int i = 0; i < index_double; i++){
            if ((i + file_desc->reader)%BLOCK_SIZE == 0){
                actual_pointer = directioning_to_block(file_desc->reader + i, 2, 2);
                simple_block = go_to_block(mounted_disk, directioning_double->pointers[actual_pointer]);
                directioning = get_directioning_block(simple_block);
                actual = directioning_to_block(file_desc->reader + i, 2, 1);
                data_block = go_to_block(mounted_disk, directioning->pointers[actual]);
            }
            save[i] = data_block->data[get_index(i + file_desc->reader, 2)];
        }
        file_desc->reader = file_desc->reader + index_double;
        
    }

    if ((doublex < file_desc->reader) && (file_desc->reader < triple)){
        index_triple = min(triple - file_desc->reader, total - index_double);
        
        block = go_to_block(mounted_disk, file_desc->index->triple_directioning_block);
        directioning_triple = get_directioning_block(block);
        actual_pointer = directioning_to_block(file_desc->reader, 3, 3);
        
        double_block = go_to_block(mounted_disk, directioning_triple->pointers[actual_pointer]);
        directioning_double = get_directioning_block(double_block);
        simple_pointer = directioning_to_block(file_desc->reader, 3, 2);
        
        simple_block = go_to_block(mounted_disk, directioning_double->pointers[simple_pointer]);
        directioning = get_directioning_block(simple_block);
        actual = directioning_to_block(file_desc->reader, 3, 1);
        data_block = go_to_block(mounted_disk, directioning->pointers[actual]);

        for (int i = 0; i < index_triple; i++){
            if ((i + file_desc->reader)%BLOCK_SIZE == 0){
                actual_pointer = directioning_to_block(file_desc->reader + i, 3, 3);
        
                double_block = go_to_block(mounted_disk, directioning_triple->pointers[actual_pointer]);
                directioning_double = get_directioning_block(double_block);
                simple_pointer = directioning_to_block(file_desc->reader + i, 3, 2);

                simple_block = go_to_block(mounted_disk, directioning_double->pointers[simple_pointer]);
                directioning = get_directioning_block(simple_block);
                actual = directioning_to_block(file_desc->reader + i, 3, 1);
                data_block = go_to_block(mounted_disk, directioning->pointers[actual]);
            }
        
            save[i] = data_block->data[get_index(i + file_desc->reader, 3)];
        }
        file_desc->reader = file_desc->reader + index_triple;
    }
    printf("%s\n",save);
    //*buffer = save;
    return total;
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
    return (reader / BLOCK_SIZE) % 252;
}

int min(int a, int b)
{
    if (a >= b) return b;
    return a;
}

int directioning_to_block(int reader, int mode, int pointer)
{
    if (mode == 1){
        return (reader - (252*1024)) / BLOCK_SIZE;
    }
    if(mode == 2){
        if (pointer == 2) return (reader - (256*BLOCK_SIZE + 252*BLOCK_SIZE) / BLOCK_SIZE) / 256; //dentro de los simples
        return (reader - (256*BLOCK_SIZE + 252*BLOCK_SIZE) / BLOCK_SIZE) % 256; //dentro de los bloques
    }
    if (mode == 3){
        int floor = (reader - 67629056) / BLOCK_SIZE;
        if (pointer == 1) return floor%256; //en que bloque estoy del 1 al 256
        else if(pointer == 2) return (floor/256)%256; //dentro de los simples
        else return ((floor/256)/256)%256; //dentro de los dobles
        
    }
    return 0;
}

int get_index(int reader, int mode)
{
    if(mode == 0) return (reader % BLOCK_SIZE);
    else if (mode == 1) return (reader - (252*1024)) % BLOCK_SIZE;
    else if (mode == 2) return (reader - (256*BLOCK_SIZE + 252*BLOCK_SIZE) % BLOCK_SIZE);
    else return (reader - 67629056) % BLOCK_SIZE;
    return 0;
}