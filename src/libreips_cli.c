#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libreips.h"

#define APP_NAME "libreips_cli"

void print_usage()
{
    printf("Usage:\n");
    printf("\t" APP_NAME " c <original> <modified> <output>\n");
    printf("\t" APP_NAME " a <original> <patch> <output>\n");
}

void print_invalid_argument(char* argument)
{
    printf("Invalid argument: %s\n\n", argument);
    print_usage();
}

void print_memory_error()
{
    printf("An error occurred while attempting to allocate memory.\n");
}

int main(int argc, char* argv[])
{
    FILE* original_file, * operand_file, * output_file;
    unsigned char* original_bytes, * operand_bytes, * output_bytes = 0;
    unsigned long cur_file_size = 0;
    unsigned long read_file_bytes = 0;
    unsigned long original_size = 0, output_size = 0;
    int apply = 0; /* 0: create, 1: apply */

    if (argc < 5)
    {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "c") == 0)
    {
        apply = 0;
    }
    else if (strcmp(argv[1], "a") == 0)
    {
        apply = 1;
    }
    else
    {
        print_usage();
        return 0;
    }

    /* Open files to read */
    original_file = fopen(argv[2], "r");

    if (!original_file)
    {
        print_invalid_argument("original");
        return 1;
    }

    operand_file = fopen(argv[3], "r");

    if (!operand_file)
    {
        if (apply)
        {
            print_invalid_argument("patch");
        }
        else
        {
            print_invalid_argument("modified");
        }

        return 1;
    }

    /* Read file contents */
    fseek(original_file, 0, SEEK_END);
    cur_file_size = ftell(original_file);
    fseek(original_file, 0, SEEK_SET);
    original_bytes = malloc(cur_file_size);

    if (!original_bytes)
    {
        print_memory_error();
        return 1;
    }

    read_file_bytes = fread(original_bytes, 1, cur_file_size, original_file);
    fclose(original_file);
    original_size = cur_file_size;

    fseek(operand_file, 0, SEEK_END);
    cur_file_size = ftell(operand_file);
    fseek(operand_file, 0, SEEK_SET);
    operand_bytes = malloc(cur_file_size);

    if (!operand_bytes)
    {
        print_memory_error();
        return 1;
    }

    read_file_bytes = fread(operand_bytes, 1, cur_file_size, operand_file);
    fclose(operand_file);

    /* Open the output file to ensure that it is writable */
    output_file = fopen(argv[4], "w");

    if (!output_file)
    {
        print_invalid_argument("output");
        return 1;
    }

    /* Apply/Create */
    if (apply)
    {
        output_bytes = lips_apply_patch(original_bytes, operand_bytes, original_size, cur_file_size);
        output_size = original_size;
    }
    else
    {
        output_bytes = lips_create_patch(original_bytes, operand_bytes, original_size, &output_size);
    }

    if (output_bytes)
    {
        fwrite(output_bytes, 1, output_size, output_file);
        printf("Successfully wrote %lu bytes.\n", output_size);
    }

    fclose(output_file);
    free(output_bytes);
    free(operand_bytes);
    free(original_bytes);

    if (!output_bytes)
    {
        if (apply)
        {
            printf("Internal error on patch application.\n");
            return 2;
        }
        else
        {
            printf("Internal error on patch creation.\n");
            return 2;
        }
    }

    return 0;
}
