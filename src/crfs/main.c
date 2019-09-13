#include <stdio.h>
#include "../constants/constants.h"
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    crFILE *file = cr_open("/memes/file.txt", "r");
    cr_close(file);
    return 0;
}
