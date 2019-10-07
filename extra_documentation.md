# Extern Methods

This document contains every detail about the functionality, implementation and use cases of every function defined in the extern methods by ThanOS.

<!-- markdownlint-disable MD024 -->

## cr_cat

```c
#include "extern_methods.h"
int cr_cat(char *path);
```

### Description

Prints the contents of `path` to the `stdout`.

### Return Value and Error Handling

If path is not an existing file, it logs a message to `stderr` and returns `0`. Otherwise, it returns `1`

## snap

```c
#include "extern_methods.h"
void snap();
```

### Description

Recursively removes half the files of every folder.

### Return Value and Error Handling

No error handling. No return values. No return. Just destruction and dust.
