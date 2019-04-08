#ifndef LIBREIPS_H
#define LIBREIPS_H

unsigned long lips_create_patch(const unsigned char* const original, const unsigned char* const modified,
    unsigned char* output, const unsigned long size);

#endif /* LIBREIPS_H */
