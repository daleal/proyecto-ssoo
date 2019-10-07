#include <stdio.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    aux();
    cr_unmount();
    return 0;
}
