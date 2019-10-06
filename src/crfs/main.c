#include <stdio.h>
#include "../constants/constants.h"
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    cr_bitmap(1, false);
    crFILE *file = cr_open("/Withered Leaf.mp3", 'r');
    test_cr_open(file);
    crFILE *file1 = cr_open("/intro.txt", 'r');
    test_cr_open(file1);

    crFILE *file3 = cr_open("/memes/ack.jpg", 'r');
    test_cr_open(file3);
    crFILE *file4 = cr_open("/memes/loops.jpg", 'r');
    test_cr_open(file4);
    crFILE *file5 = cr_open("/dir/texto.txt", 'r');
    test_cr_open(file5);
    cr_close(file);
    cr_unmount();
    return 0;
}
