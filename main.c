#include "common.h"
int main(int argc, char *argv[])
{
    ToviewInfo Toview;
    OperationType operation;

    printf("===================================================\n");
    printf("                 MP3 TAG READER                    \n");
    printf("                    ~SHREYAS~\n");
    printf("===================================================\n");
    if(argc < 2)
    {
        printf("\n");
        printf("Usage:\n");
        printf("./a.out -v song.mp3\n");
        printf("./a.out -e -t \"New Title\" song.mp3\n");
        printf("./a.out --help\n");
        printf("\n");
        return 1;
    }
    if (strcmp(argv[1], "--help") == SUCCESS)
    {
        printf("\n");
        printf("      << MP3 TAG READRE & EDITOR >>\n");
        printf("Usage:\n");
        printf("  ./a.out -v song.mp3\n");
        printf("  ./a.out -e -t \"New Title\" song.mp3\n");
        printf("  ./a.out --help\n");
        printf("\n");
        printf("===================================================\n");
        return 0;
    }

    operation = check_operation_type(argv);

    switch (operation)
    {
        case e_view:
        printf("                << MP3 VIEW MODE  >>\n");

        if (read_and_validate(argv, &Toview) == SUCCESS)
        {
            //printf("         << File validation successful >>\n");
            to_view(&Toview);
        }
        else
        {
            printf("File validation failed\n");
        }
        break;
    
        case e_edit:
        
        if (argc != 5)
        {
            printf("ERROR: Invalid arguments for edit\n");
            printf("Usage: ./a.exe -e -t \"New Title\" sample.mp3\n");
            return FAILURE;
        }

        if (read_and_validate(argv, &Toview) == SUCCESS)
        {
            to_edit(argv, &Toview);
        }
        break;

        case e_unsupported:
        printf("\n");
            printf("Error: Invalid option!\n");
            printf("Use --help to see the available options.\n");
            printf("\n");
            break;
    }
    printf("===================================================\n");
    return 0;
}
