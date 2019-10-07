#include <stdio.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "../extern_methods/extern_methods.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    cr_load("./src/cr_API/cr_API.c");
    cr_cat("/cr_API.c");
    cr_unmount();
    return 0;
}
