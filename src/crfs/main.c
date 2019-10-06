#include <stdio.h>
#include <stdlib.h>
#include "../constants/constants.h"
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    cr_bitmap(1, false);
    Disk* disk = open_disk(argv[1]);
    
    printf("abri el disco\n");
    crFILE *file_desc = malloc(sizeof(crFILE));
    file_desc->index = get_index_block(cr_cd(disk,"/intro.txt"));
    printf("llego aca\n");
    file_desc->reader = 0;
    unsigned char buffer[3000];
    cr_read(file_desc, buffer, 3000);
    cr_close(file_desc);
    cr_unmount();
    return 0;
}
