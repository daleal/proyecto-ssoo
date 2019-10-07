#include <stdio.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]); //montamos el disco
    cr_bitmap(1, false);

    //Lee intro.txt y guarda en buffer 
    crFile *file_r = cr_open("intro.txt", 'r');
    unsigned int size = file_r->index->size;
    unsigned char buffer[size];
    cr_read(file_r, buffer, size);
    cr_close(file_r);

    //Escribe en intro_copy el mismo contenido de intro.txt
    // crFILE *file_w = cr_open("/memes/intro_copy.txt", 'w');
    // cr_write(file_w, buffer, size); 
    // cr_close(file_W);

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
    
    // cr_rm("/memes/intro_copy.txt");

    cr_unload("/Withered Leaf.mp3", "/NO SE QUE DESTINO PONER");

    //cr_load("example.txt"); //esto deberia estar en nuestro pc o no?

    cr_unmount();
    return 0;
}


 disco se llenó o porque el archivo no puede crecer más, este número puede ser
menor a nbytes (incluso 0).
int cr_close(crFILE* file_desc). Función para cerrar archivos. Cierra el archivo indicado por
file desc. Debe garantizar que cuando esta función retorna, el archivo se encuentra actualizado en disco.
int cr_rm(char* path). Función para borrar archivos. Elimina el archivo referenciado por la ruta path
del directorio correspondiente. Los bloques que estaban siendo usados por el archivo deben quedar libres.
int cr_unload(char* orig, char* dest). Función que se encarga de copiar un archivo o un árbol
de directorios (es decir, un directorio y todos sus contenidos) del disco, referenciado por orig, hacia un nuevo
archivo o directorio de ruta dest en su computador.
int cr_load(char* orig). F
