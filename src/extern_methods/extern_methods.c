#include <stdio.h>
#include <stdlib.h>
#include "../error_handler/error_handler.h"
#include "../cr_API/cr_API.h"
#include "extern_methods.h"


int cr_cat(char *path)
{
    crFILE *file = cr_open(path, 'r');
    if (file == NULL) {
        log_error("No such file");
        return 0;
    }
    char *buffer = calloc(file->index->size + 1, sizeof(unsigned char));
    cr_read(file, buffer, file->index->size);
    cr_close(file);
    printf("%s\n", buffer);
    free(buffer);
    return 1;
}
