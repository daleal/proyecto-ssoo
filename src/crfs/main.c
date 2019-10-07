#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "../extern_methods/extern_methods.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);  // Montamos el disco
    cr_bitmap(1, false);

    // No existe, retorna NULL
    crFILE *file = cr_open("/memes/file.txt", 'r');
    // Escribe en el stderr
    cr_close(file);

    // Lee intro.txt y guarda en buffer
    crFILE *file_r = cr_open("/intro.txt", 'r');
    unsigned int size = file_r->index->size;
    unsigned char *buffer = calloc(size + 1, sizeof(unsigned char));
    cr_read(file_r, buffer, size);
    cr_close(file_r);

    // Escribe en intro_copy el mismo contenido de intro.txt
    crFILE *file_w = cr_open("/memes/intro_copy.txt", 'w');
    cr_write(file_w, buffer, size);
    cr_close(file_w);

    free(buffer);

    int nbytes;

    // Eee en dos partes el archivo para mostrar funcionalidad
    // de reader
    crFILE *file_incomplete = cr_open("/memes/intro_copy.txt", 'r');
    nbytes = file_incomplete->index->size / 4;
    unsigned char *I_buffer = calloc(nbytes + 1, sizeof(unsigned char));
    cr_read(file_incomplete, I_buffer, nbytes);
    printf("This is the first part of the file: %s\n", I_buffer);

    free(I_buffer);

    unsigned char *C_buffer = calloc((file_incomplete->index->size * (3 / 4)) + 1, sizeof(unsigned char));
    cr_read(file_incomplete, C_buffer, file_incomplete->index->size*(3/4));
    printf("This is the second part of the file: %s\n", C_buffer);

    free(C_buffer);

    cr_close(file_incomplete);

    cr_rm("/memes/intro_copy.txt");

    cr_unload("/Withered Leaf.mp3", "./");

    cr_load("./pikachu2.png");

    cr_ls("/");

    cr_unmount();
    return 0;
}
