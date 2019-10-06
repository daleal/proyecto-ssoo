#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../constants/constants.h"


extern int errno;


void handle_error(char *error)
{
    int errnum = errno;
    int string_size = strlen(error) + strlen(strerror(errnum)) + 4;
    char *formatted_error = malloc(string_size * sizeof(char));
    sprintf(formatted_error, "%s: %s\n", error, strerror(errnum));
    fprintf(stderr, "Error of value %d\n", errno);
    fprintf(stderr, formatted_error);
    free(formatted_error);
}


void log_error(char *error)
{
    int string_size = strlen(error) + 2;
    char *formatted_error = malloc(string_size * sizeof(char));
    sprintf(formatted_error, "%s\n", error);
    fprintf(stderr, formatted_error);
    free(formatted_error);
}
