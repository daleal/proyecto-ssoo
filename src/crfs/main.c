#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "../extern_methods/extern_methods.h"
#include "main.h"


void separator()
{
    fprintf(stderr, "\n=================================================================\n\n");
}


int main(int argc, char **argv)
{
    // Used variable declarations
    crFILE *cr_file;
    unsigned char *buffer;
    unsigned int buffer_size;
    unsigned int read_size;
    int status;

    // Mount the disk
    cr_mount(argv[1]);

    // Get the first bitmap block as bits
    cr_bitmap(1, false);

    separator();

    // See root file structure
    cr_ls("/");

    separator();

    // Open a non existent file
    cr_file = cr_open("/memes/file.txt", 'r');

    // cr_file is NULL, a message gets logged to stderr
    // Close file (returns 0, because there is nothing to close)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    separator();

    // Read an existent file
    cr_file = cr_open("/intro.txt", 'r');

    // Set buffer size as the whole file size
    buffer_size = cr_file->index->size;
    buffer = calloc(buffer_size + 1, sizeof(unsigned char));

    // Read file to buffer and get the read size
    read_size = cr_read(cr_file, buffer, buffer_size);

    // Close file (returns 1 because cr_file is not NULL)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    separator();

    // Escribe en intro_copy el mismo contenido de intro.txt
    // crFILE *file_w = cr_open("/memes/intro_copy.txt", 'w');
    // cr_write(file_w, buffer, size);
    // cr_close(file_w);

    // free(buffer);

    // int nbytes = 5;

    // // Eee en dos partes el archivo para mostrar funcionalidad
    // // de reader
    // crFILE *file_incomplete = cr_open("/memes/intro_copy.txt", 'r');
    // unsigned char *I_buffer = calloc(nbytes, sizeof(unsigned char));
    // nbytes = cr_read(file_incomplete, I_buffer, nbytes);
    // printf("The first %i bytes of the file are: %s\n", nbytes, I_buffer);

    // free(I_buffer);

    // unsigned char *C_buffer = calloc(file_incomplete->index->size, sizeof(unsigned char));
    // nbytes = cr_read(file_incomplete, C_buffer, file_incomplete->index->size);
    // printf("The remaining %i bytes read frome the file are: %s\n", nbytes, C_buffer);

    // free(C_buffer);

    // cr_close(file_incomplete);

    // cr_rm("/memes/intro_copy.txt");

    // // cr_unload("/Withered Leaf.mp3", "./generated");

    // cr_load("./pikachu2.png");

    // cr_ls("/");

    cr_unmount();
    return 0;
}
