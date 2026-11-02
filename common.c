#include "common.h"

OperationType check_operation_type(char *argv[])
{
    //printf("Argument received: %s\n", argv[1]);
    if(strcmp(argv[1], "-v") == 0)
    {
        return e_view;
    }
    else if (strcmp(argv[1] , "-e") == 0)
    {
        return e_edit;
    }
    else
    {
        return e_unsupported;
    }
}

Status read_and_validate(char *argv[], ToviewInfo *Toview)
{
    char *filename;
    FILE *fp;

    if (strcmp(argv[1], "-v") == 0)
    {
        filename = argv[2];
    }
    else if (strcmp(argv[1], "-e") == 0)
    {
        filename = argv[4];
    }
    else
    {
        return FAILURE;
    }
    char *extn = strrchr(filename, '.');

    if (extn == NULL || strcmp(extn, ".mp3") != 0)
    {
        printf("Error: Invalid file extension. Use an MP3 file.\n");
        return FAILURE;
    }

    Toview->mp3_fname = filename;
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        printf("ERROR: Unable to open file %s\n", filename);
        return FAILURE;
    }
    Toview->fptr_mp3_file = fp;
    return SUCCESS;
}