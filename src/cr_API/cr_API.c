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
    if (mounted_disk == NULL) {
        printf("No disk is mounted.\n");
        return;
    }
    if (block == 0) {
        for (int i = 0; i < BITMAP_BYTES; i++) {
            if (!!hex) {
                fprintf(stderr, "0x%X\n",  mounted_disk->bitmap[i]);
            } else {
                for (int j = 0; j < 8; j++) {
                    fprintf(stderr, "%i ", bit_from_bitmap(mounted_disk, (i * 8) + j));
                }
                fprintf(stderr, "\n");
            }
        }
        fprintf(stderr, "Used blocks: %i\n", used_blocks(mounted_disk));
        fprintf(stderr, "Free blocks: %i\n", free_blocks(mounted_disk));
    } else if (block >= 1 && block <= 129) {
        --block;
        for (int offset = 0; offset < BLOCK_SIZE; offset++) {
            if (!!hex) {
                fprintf(stderr, "0x%X\n", mounted_disk->bitmap[(block * BLOCK_SIZE) + offset]);
            } else {
                for (int j = 0; j < 8; j++) {
                    fprintf(
                        stderr, "%i ",
                        bit_from_bitmap(mounted_disk, (((block * BLOCK_SIZE) + offset) * 8) + j)
                    );
                }
                fprintf(stderr, "\n");
            }
        }
    } else {
        fprintf(
            stderr,
            "Invalid bitmap block %i. Please try again with a number between 1 and 129.\n",
            block
        );
    }
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
