# LibreIPS

LibreIPS is a permissively-licensed IPS patch creation and application library with an API aimed to be C89-compatible.

## API

The API is exposed by `libreips.h` and is defined as follows:

```c
unsigned char* lips_create_patch(const unsigned char* const original, const unsigned char* const modified,
    const unsigned long size, unsigned long* const output_size);

unsigned char* lips_apply_patch(const unsigned char* const original, const unsigned char* const patch,
    const unsigned long original_size, const unsigned long patch_size);
```

## CLI Usage

A CLI tool is available for testing and convenience. Usage is as follows:

```sh
libreips_cli c <original> <modified> <output>
libreips_cli a <original> <patch> <output>
```

Where `c` creates and `a` applies a pastch.
