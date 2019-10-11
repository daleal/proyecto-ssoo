#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cr_API/cr_API.h"
#include "../extern_methods/extern_methods.h"
#include "main.h"


#define ARBITRARY_SIZE 512

#define REAL_IMAGE_NAME "./pikachu.png"
#define VIRTUAL_IMAGE_NAME "/pikachu.png"

#define VIRTUAL_FOLDER_NAME "/memes"
#define INVALID_REAL_FOLDER_NAME "./no_existo"
#define VALID_REAL_FOLDER_NAME "./"


void separator()
{
    fprintf(stderr, "\n=================================================================\n\n");
    // fprintf(stderr, data);
    // fprintf(stderr, "\n=================================================================\n\n");
}


/*
 * INSTRUCTIONS:
 * Put an image in the root directory of this repo and
 * change the variables REAL_IMAGE_NAME and VIRTUAL_IMAGE_NAME
 * to the name of the image (REAL_IMAGE_NAME with a leading "./"
 * and VIRTUAL_IMAGE_NAME with a leading "/").
 * Make sure there are no dirs in the root directory with the
 * name INVALID_REAL_FOLDER_NAME or change that value not to
 * match any existing folder (always with a leading "./").
 * Make sure that VIRTUAL_FOLDER_NAME is a valid folder inside
 * the virtual disk.
 * Make sure that VALID_REAL_FOLDER_NAME exists in the repo
 * and that the variable starts with a leading "./".
 */
int main(int argc, char **argv)
{
    // Used variable declarations
    crFILE *cr_file;
    unsigned char *buffer;
    unsigned int buffer_size;
    unsigned int read_size;
    unsigned int write_size;
    int status;

    separator();  // ==========================================================

    // Try to unmount disk with no mounted disk
    cr_unmount();

    separator();  // ==========================================================

    // Mount the disk
    cr_mount(argv[1]);

    // Try to mount another disk (error, a disk is already mounted)
    cr_mount(argv[1]);

    separator();  // ==========================================================

    // Get the first bitmap block as bits
    cr_bitmap(1, false);

    separator();  // ==========================================================

    // See root file structure
    cr_ls("/");

    separator();  // ==========================================================

    // Open a non existent file
    cr_file = cr_open("/memes/file.txt", 'r');

    // cr_file is NULL, a message gets logged to stderr
    // Close file (returns 0, because there is nothing to close)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    separator();  // ==========================================================

    // Read an existent file
    cr_file = cr_open("/intro.txt", 'r');

    // Set buffer size as the whole file size
    buffer_size = cr_file->index->size;
    buffer = calloc(buffer_size + 1, sizeof(unsigned char));

    // Read file to buffer and get the read size
    read_size = cr_read(cr_file, buffer, buffer_size);
    printf("Bytes read: %i\n", read_size);

    // Close file (returns 1 because cr_file is not NULL)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    separator();  // ==========================================================

    // Open an existing file in write mode
    cr_file = cr_open("/intro.txt", 'w');

    // cr_file is NULL, a message gets logged to stderr
    // Close file (returns 0, because there is nothing to close)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    separator();  // ==========================================================

    // Create a new folder
    status = cr_mkdir("/ultra_important_stuff");
    printf("Directory successfully created: %i\n\n", status);

    // Check that dir appears listed in root
    cr_ls("/");

    separator();  // ==========================================================

    // Open a new file in write mode
    cr_file = cr_open("/ultra_important_stuff/intro_copy.txt", 'w');

    // Rewrite the content of "intro.txt" into new file
    write_size = cr_write(cr_file, buffer, read_size);
    free(buffer);  // Free data buffer
    printf("Bytes written: %i\n", write_size);

    // Close file (returns 1 because cr_file is not NULL)
    status = cr_close(cr_file);
    printf("cr_close status: %i\n", status);

    // Check that file exists
    status = cr_exists("/ultra_important_stuff/intro_copy.txt");
    printf("\nFile created correctly: %i\n\n", status);

    // Check that file appears listed inside folder
    cr_ls("/ultra_important_stuff");

    separator();  // ==========================================================

    // Open the file written a few lines ago
    cr_file = cr_open("/ultra_important_stuff/intro_copy.txt", 'r');
    printf("File size: %i bytes\n\n", cr_file->index->size);

    // Set buffer size as half of the file size
    buffer_size = cr_file->index->size / 4;
    buffer = calloc(buffer_size + 1, sizeof(unsigned char));

    // Read file bytes to buffer and get the read size
    read_size = cr_read(cr_file, buffer, buffer_size);
    printf("Buffer size: %i bytes\n", buffer_size);
    printf("Bytes read: %i bytes\n", read_size);
    printf("Data read:\n\t==== READ ====\n%s\n\t==== READ ====\n", buffer);
    free(buffer);

    separator();  // ==========================================================

    // Set an arbitrarily big buffer
    buffer_size = ARBITRARY_SIZE;
    buffer = calloc(buffer_size + 1, sizeof(unsigned char));

    // Read file bytes to buffer and get the read size
    read_size = cr_read(cr_file, buffer, buffer_size);
    printf("Buffer size: %i bytes\n", buffer_size);
    printf("Bytes read: %i bytes\n", read_size);
    printf("Data read:\n\t==== READ ====\n%s\n\t==== READ ====\n", buffer);
    free(buffer);

    // Close file (returns 1 because cr_file is not NULL)
    status = cr_close(cr_file);
    printf("\ncr_close status: %i\n", status);

    separator();  // ==========================================================

    // Delete file just created
    status = cr_rm("/ultra_important_stuff/intro_copy.txt");
    printf("File correctly removed: %i\n\n", status);

    // Check that file no longer appears listed inside folder
    cr_ls("/ultra_important_stuff");

    separator();  // ==========================================================

    // Load image to system
    status = cr_load(REAL_IMAGE_NAME);
    printf("File correctly loaded: %i\n", status);

    // Check that file appears listed inside folder
    cr_ls("/");

    // Get image size
    cr_file = cr_open(VIRTUAL_IMAGE_NAME, 'r');
    printf("\nFile size: %i bytes\n", cr_file->index->size);
    cr_close(cr_file);

    separator();  // ==========================================================

    // Unload folder to invalid external folder
    status = cr_unload(VIRTUAL_FOLDER_NAME, INVALID_REAL_FOLDER_NAME);
    printf("Unload status: %i\n", status);

    // Unload folder to valid external folder
    status = cr_unload(VIRTUAL_FOLDER_NAME, VALID_REAL_FOLDER_NAME);
    printf("\nUnload status: %i\n", status);

    separator();  // ==========================================================

    // Unmount disk
    cr_unmount();

    return 0;
}
