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
  
    cr_open("/intro.txt", 0);
    //cr_close(file_desc);
    cr_unmount();
    return 0;
}
