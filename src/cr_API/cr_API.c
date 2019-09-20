#include <stdio.h>
#include <stdlib.h>
#include "../constants/constants.h"
#include "../disk_manager/disk_manager.h"
#include "cr_API.h"


// File structs
struct crfile {

};


Disk *mounted_disk = NULL;


// File management functions
crFILE *cr_open(char *path, char *mode)
{
    crFILE *file_desc = malloc(sizeof(crFILE));
    return file_desc;
}


int cr_read(crFILE *file_desc, void *buffer, int nbytes)
{
    return 0;
}


int cr_write(crFILE *file_desc, void *buffer, int nbytes)
{
    return 0;
}


int cr_close(crFILE *file_desc)
{
    free(file_desc);
    return 0;
}


int cr_rm(char *path)
{
    return 0;
}


int cr_unload(char *orig, char *dest)
{
    return 0;
}


int cr_load(char *orig)
{
    return 0;
}


// General API functions
void cr_mount(char *diskname)
{
    mounted_disk = open_disk(diskname);
}


void cr_unmount()
{
    free(mounted_disk);
}


void cr_bitmap(unsigned block, bool hex)
{

}


int cr_exists(char *path)
{
    return 0;
}


void cr_ls(char *path)
{

}


int cr_mkdir(char *foldername)
{
    return 0;
}
