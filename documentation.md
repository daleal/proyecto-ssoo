# Cruz FileSystem

This document contains every detail about the functionality, implementation and use cases of every function defined in the API developed by ThanOS.

<!-- markdownlint-disable MD024 -->

## cr_open

```c
#include "cr_API.h"

crFILE *cr_open(char *path, char mode);
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

Read `file_desc` from the last read byte (`file_desc->reader`), until the next `nbytes`. 

### Return Value and Error Handling

Returns the number of bytes read. If `nbytes` plus the numbers of bytes read to the moment is less than the total size of the file, returns exactly `nbytes`. In the other case returns  `nbytes` less the numbers of bytes unread.

If the pointer to `crFILE` is `NULL` returns `-1`.


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

If `crFILE` es a `NULL` pointer, returns `-1`.

### Notes

Aditional notes about the function

## cr_close

```c
#include "cr_API.h"

int cr_close(crFILE *file_desc);
```

### Description

Closes the file saved on `file_desc` and frees its memory.

### Return Value and Error Handling

This function returns `1` if the pointer to `crFILE` is `NULL`. `0` in other case.

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

The function returns `1`.  
If `dest` belongs to a non-existent destination in the computer it returns `0`.
If `orig` belongs to a non-existent virtual destination it returns `0`.

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

Opens the disk `diskname` and saves a pointer to it on the environmental variable `mounted_disk`.

### Return Value and Error Handling

The function returns void. In case that `diskname` does not exist, it logs a message to `stderr` and sets the environmental variable `mounted_disk` to `NULL`.

## cr_unmount

```c
#include "cr_API.h"

void cr_unmount();
```

### Description

Closes the disk loaded on the environmental variable `mounted_disk` and frees its memory.

### Return Value and Error Handling

The function returns void. In case that `mounted_disk` is set to `NULL`, it logs a message to `stderr`.

## cr_bitmap

```c
#include "cr_API.h"

void cr_bitmap(unsigned block, bool hex);
```

### Description

Shows a representation of the bitmap of `mounted_disk` in `stderr`.

If `block` is `0`, it shows the whole bitmap and then the amount of used blocks and the amount of free blocks left. If `block` is any number between `1` and `129` (both included), it shows the bitmap block in that position.

If `hex` is `true`, the bitmap will show every byte as a hex value. If `hex` is `false`, the bitmap will show every bit on its own.

### Return Value and Error Handling

The function returns void. In case that `block` is a value different than the ones specified (`0` to `129`), it logs a message to `stderr`. If no disk is mounted, it logs a message to `stderr`.

### Notes

As the bitmap contains 131072 bytes of information, calling the function with `block` set to `0` will probably overflow the console, so it is advisable to change the `stderr` file to an external file and use it as a debug log.

## cr_exists

```c
#include "cr_API.h"

int cr_exists(char *path);
```

### Description

Returns `1` if the file/directory `path` exists and `0` if it does not exist.

### Return Value and Error Handling

The function returns `1` or `0`. If no disk is mounted, it logs a message to `stderr` and returns `0`.

## cr_ls

```c
#include "cr_API.h"

void cr_ls(char *path);
```

### Description

Prints to `stdout` the files and directories inside the directory `path`.

### Return Value and Error Handling

The function returns void. In case that `path` does not exist, it logs a message to `stderr`. If no disk is mounted, it logs a message to `stderr`.

## cr_mkdir

```c
#include "cr_API.h"

int cr_mkdir(char *foldername);
```

### Description

Creates a folder in `foldername` if the complete path up to the last `/` exists.

### Return Value and Error Handling

The function returns `1` if it succeeds. If the path up to the last `/` does not exist, it logs a message to `stderr` and returns `0`. If a folder with the same name in the same path exists, it logs a message to `stderr` and returns `0`. If the disk has no space left, it logs a message to `stderr` and returns `0`. If no disk is mounted, it logs a message to `stderr` and returns `0`.

### Notes

Paths like `/dir1/dir2/` will be pre-processed to be `/dir1/dir2` and then created.
