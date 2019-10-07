#include <stdio.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]); //montamos el disco
    cr_bitmap(1, false);

    //Lee intro.txt y guarda en buffer 
    crFile *file_r = cr_open("/intro.txt", 'r');
    unsigned int size = file_r->index->size;
    unsigned char buffer[size];
    cr_read(file_r, buffer, size);
    cr_close(file_r);

    //Escribe en intro_copy el mismo contenido de intro.txt
    crFILE *file_w = cr_open("/memes/intro_copy.txt", 'w');
    cr_write(file_w, buffer, size); 
    cr_close(file_W);

    //Eee en dos partes el archivo para mostrar funcionalidad
    //de reader
    crFILE *file_incomplete = cr_open("/memes/intro_copy.txt", 'r');
    unsigned char I_buffer[file_incomplete->index->size/4];
    cr_read(file_incomplete, I_buffer, file_incomplete->index->size/4);
    printf("This is the first part of the file: %s\n", I_buffer);

    unsigned char C_buffer[file_incomplete->index->size*(3/4)];
    cr_read(file_incomplete, C_buffer, file_incomplete->index->size*(3/4));
    printf("This is the second part of the file: %s\n", C_buffer);

    cr_close(file_incomplete);
    
    cr_rm("/memes/intro_copy.txt");

    cr_unload("/Withered Leaf.mp3", "./");

    cr_load("/pikachu2.png"); 

    cr_unmount();
    return 0;
}


