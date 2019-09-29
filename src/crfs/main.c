#include <stdio.h>
#include "../constants/constants.h"
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    cr_bitmap(1, false);
    crFILE *file = cr_open("/memes/file.txt", 'r');
    cr_close(file);
    cr_unmount();
    return 0;
}
