#ifndef LIBREIPS_H
#define LIBREIPS_H

unsigned char* lips_create_patch(const unsigned char* const original, const unsigned char* const modified,
    const unsigned long size, unsigned long* const output_size);
unsigned char* lips_apply_patch(const unsigned char* const original, const unsigned char* const patch,
    const unsigned long original_size, const unsigned long patch_size);

#endif /* LIBREIPS_H */
