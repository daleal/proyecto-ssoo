#include <stdio.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    crFILE *file = cr_open("new.txt", 'w');
    char buffer[3000];
    strcpy(buffer, "holaa!!");
    cr_write(file, buffer, strlen(buffer));
    cr_close(file);

    cr_ls("/");

    file = cr_open("new.txt", 'r');
    char new_buffer[3000];
    cr_read(file, new_buffer, strlen(buffer));
    FILE *write = fopen("nuevo.txt", "wb");
    fwrite(new_buffer, sizeof(unsigned char), strlen(buffer), write);
    fclose(write);
    cr_close(file);

    // aux();
    // cr_bitmap(1, false);
    cr_unmount();
    return 0;
}
