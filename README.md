# LibreIPS

LibreIPS is a permissively-licensed IPS patch creation and application library with an API aimed to be C89-compatible.

## API

The API is exposed by `libreips.h` and is defined as follows:

```c
unsigned long lips_create_patch(const unsigned char* const original, const unsigned char* const modified,
    unsigned char* output, const unsigned long size);

unsigned char* lips_apply_patch(unsigned char* original, unsigned char* patch, unsigned long size);
```

When creating a patch, LibreIPS takes care of allocating memory for the output file contents.

## CLI Usage

A CLI tool is available for testing and convenience. Usage is as follows:

```sh
libreips_cli c <original> <modified> <output>
libreips_cli a <original> <patch> <output>
```

Where `c` creates and `a` applies a pastch.
