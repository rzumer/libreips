#include <assert.h>
#include "libreips.h"

#define TRUE (1)
#define FALSE (0)

#define HEADER "PATCH"
#define FOOTER "EOF"

/* arbitrary chunk sizes */
#define RECORD_CHUNK_SIZE (128)
#define DATA_CHUNK_SIZE (1024)

#define RECORD_HEADER_LENGTH (5)
#define RLE_RECORD_LENGTH (8)

#define EMPTY_RECORD (Record) { 0xFFFFFFFF, 0, 0, 0 }

typedef struct Record
{
    unsigned int offset;
    unsigned short size;
    unsigned char* data;
    int rle; /* flag */
} Record;

typedef struct Patch
{
    unsigned long num_records;
    Record** records;
} Patch;

Record record_init(const unsigned int offset)
{
    Record record = { offset };

    assert(offset <= 0xFFFFFF);

    record.data = malloc(DATA_CHUNK_SIZE);

    return record;
}

void record_free(Record* const record)
{
    if (record->data)
    {
        free(record->data);
    }

    free(record);
}

Patch patch_init()
{
    Patch patch = { 0 };

    patch.records = malloc(RECORD_CHUNK_SIZE);

    return patch;
}

void patch_free(Patch* const patch)
{
    int i;

    if (patch->records)
    {
        for (i = 0; i < patch->num_records; i++)
        {
            record_free(patch->records[i]);
        }
    }
}

int push_record(Patch* const patch, Record* const record)
{
    Patch* new_chunk;

    if (patch->num_records > 0 && patch->num_records % RECORD_CHUNK_SIZE == 0)
    {
        new_chunk = realloc(patch->records, patch->num_records + RECORD_CHUNK_SIZE);

        if (new_chunk)
        {
            patch->records = new_chunk;
        }
        else
        {
            return FALSE;
        }
    }

    if (patch->records != NULL)
    {
        *(patch->records + patch->num_records++) = record;
    }

    return FALSE;
}

int push_byte(Record* const record, const unsigned char value)
{
    Record* new_chunk;

    if (record->data == NULL || record->size == 0xFFFF)
    {
        return FALSE;
    }

    if (record->size > 0 && record->size % DATA_CHUNK_SIZE == 0)
    {
        new_chunk = realloc(record->data, record->size + DATA_CHUNK_SIZE);

        if (new_chunk)
        {
            record->data = new_chunk;
        }
        else
        {
            return FALSE;
        }
    }

    *(record->data + record->size++) = value;
    return TRUE;
}

unsigned long lips_create_patch(const unsigned char* const original, const unsigned char* const modified,
    unsigned char* output, const unsigned long size)
{
    unsigned long i = 0, j = 0;
    unsigned char original_byte = 0, modified_byte = 0;
    Patch patch = patch_init();
    Record cur_record = EMPTY_RECORD;
    unsigned long patch_size = sizeof(HEADER) + sizeof(FOOTER);

    assert(patch_size == 8);

    /* First pass (parse) */
    for (i = 0; i < size; i++)
    {
        original_byte = *(original + i);
        modified_byte = *(modified + i);

        if (original_byte != modified_byte)
        {
            if (!cur_record.data)
            {
                cur_record = record_init(i);
                patch_size += RECORD_HEADER_LENGTH;
            }

            push_byte(&cur_record, modified_byte);
            patch_size++;
        }
        else
        {
            push_record(&patch, &cur_record);
            cur_record = EMPTY_RECORD;
        }
    }

    /* Push any remaining record */
    if (cur_record.data)
    {
        push_record(&patch, &cur_record);
    }

    /* Second pass (write) */
    output = malloc(patch_size);

    /* Header */
    memcpy(output, HEADER, sizeof(HEADER));
    j += sizeof(HEADER);

    for (i = 0; i < patch.num_records; i++)
    {
        /* Offset */
        *((unsigned int*)(output + j)) = patch.records[i]->offset;
        j += 3;

        /* Size */
        *((unsigned short*)(output + j)) = patch.records[i]->size;
        j += 2;

        /* Data */
        memcpy(output + j, patch.records[i]->data, patch.records[i]->size);
    }

    /* Footer */
    memcpy(output + j, FOOTER, sizeof(FOOTER));
    j += sizeof(FOOTER);

    patch_free(&patch);
    return j;
}

unsigned char* lips_apply_patch(unsigned char* original, unsigned char* patch, unsigned long size)
{
    unsigned long i = 0, j = 0;
    unsigned char* output = malloc(size);
    Record cur_record = EMPTY_RECORD;

    memcpy(output, original, size);

    /* Header */
    assert(memcmp(patch, HEADER, sizeof(HEADER)) == 0);
    i += sizeof(HEADER);

    while (i + RECORD_HEADER_LENGTH < size)
    {
        /* Offset */
        cur_record.offset =
            *(output + i++) << 16 & 0x00FF0000 |
            *(output + i++) <<  8 & 0x0000FF00 |
            *(output + i++)       & 0x000000FF;

        /* Size */
        cur_record.size = *((unsigned short*)(output + i));
        i += 2;

        if (cur_record.size > 0) /* regular record */
        {
            if (i + cur_record.size > size)
            {
                break;
            }

            /* Data */
            memcpy(output + cur_record.offset, patch + i, cur_record.size);

            i += cur_record.size;
        }
        else /* RLE record */
        {
            if (i + RLE_RECORD_LENGTH - RECORD_HEADER_LENGTH > size)
            {
                break;
            }

            /* RLE Size */
            cur_record.size = *((unsigned short*)(output + i));
            i += 2;

            /* Data */
            for (j = 0; j < cur_record.size; j++)
            {
                *(output + cur_record.offset) = *(patch + i);
            }

            i++;
        }
    }

    assert(memcmp(patch + i, FOOTER, sizeof(FOOTER)) == 0);

    return output;
}
