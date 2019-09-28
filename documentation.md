# Cruz FileSystem

This document contains every detail about the functionality, implementation and use cases of every function defined in the API developed by ThanOS.

<!-- markdownlint-disable MD024 -->

## cr_open

```c
#include "cr_API.h"

crFILE *cr_open(char *path, char *mode);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_read

```c
#include "cr_API.h"

int cr_read(crFILE *file_desc, void *buffer, int nbytes);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_write

```c
#include "cr_API.h"

int cr_write(crFILE *file_desc, void *buffer, int nbytes);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_close

```c
#include "cr_API.h"

int cr_close(crFILE *file_desc);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_rm

```c
#include "cr_API.h"

int cr_rm(char *path);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_unload

```c
#include "cr_API.h"

int cr_unload(char *orig, char *dest);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_load

```c
#include "cr_API.h"

int cr_load(char *orig);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_mount

```c
#include "cr_API.h"

void cr_mount(char *diskname);
```

### Description

Opens the disk `diskname` and saves a pointer to it in the variable `mounted_disk`.

### Return Value and Error Handling

The function returns void. In case that `diskname` does not exist, it logs a message to `stderr` and sets the variable `mounted_disk` to `NULL`.

### Notes

Aditional notes about the function

## cr_bitmap

```c
#include "cr_API.h"

void cr_bitmap(unsigned block, bool hex);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_exists

```c
#include "cr_API.h"

int cr_exists(char *path);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_ls

```c
#include "cr_API.h"

void cr_ls(char *path);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function

## cr_mkdir

```c
#include "cr_API.h"

int cr_mkdir(char *foldername);
```

### Description

Function description

### Return Value and Error Handling

Describe return value

### Notes

Aditional notes about the function
