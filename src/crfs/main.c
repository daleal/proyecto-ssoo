#include <stdio.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    crFILE *file = cr_open("new.txt", 'w');
    // for (int i = 0; i < 100; i++)
    // {
    //     char result[50];  
    //     sprintf(result, "%d", i); 
    //     printf("%s\n", result);
    //     file = cr_open(result, 'w');
    // }
    
    
    // cr_bitmap(1, false);
    cr_close(file);
    cr_unmount();
    return 0;
}
