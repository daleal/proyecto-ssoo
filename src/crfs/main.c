#include <stdio.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "main.h"


int main(int argc, char **argv)
{
    cr_mount(argv[1]);
    cr_ls("/");
    aux();
    cr_ls("/");
    crFILE *git = cr_open("/documentation.md", 'r');
    char buffer[git->index->size + 1];
    cr_read(git, buffer, git->index->size);
    printf("%s\n", buffer);
    // cr_bitmap(1, false);
    cr_unmount();
    return 0;
}
