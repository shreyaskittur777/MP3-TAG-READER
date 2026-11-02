#include "common.h"

Status to_view(ToviewInfo *Toview)
{
    if (open_files(Toview) == FAILURE)
    {
        return FAILURE;
    }

    if (check_ID_tag(Toview) == FAILURE)
    {
        goto cleanup;
    }

    if (check_version(Toview) == FAILURE)
    {
        goto cleanup;
    }

    if (skip_header(Toview) == FAILURE)
    {
        goto cleanup;
    }

    if (mp3_view(Toview) == FAILURE)
    {
        goto cleanup;
    }

    fclose(Toview->fptr_mp3_file);
    Toview->fptr_mp3_file = NULL;

    return SUCCESS;

    cleanup:

        fclose(Toview->fptr_mp3_file);
        Toview->fptr_mp3_file = NULL;

    return FAILURE;
}

//for open files 
Status open_files(ToviewInfo *Toview)
{
    if(Toview->fptr_mp3_file == NULL)
    {
        printf("ERROR : Unable to open files %s.\n", Toview->mp3_fname);
        return FAILURE;
    }
    return  SUCCESS;
}
//check ID3 tag
Status check_ID_tag(ToviewInfo *Toview)
{
    char id[4];
    if(fread(id,1,3,Toview->fptr_mp3_file)!= 3)
    {
        printf("ERROR : Unable to read ID3 Tag\n");
        return FAILURE;
    }
    id[3]= '\0';
    if(strcmp(id, "ID3") != 0)
    {
        printf("ERROR : Invalid MP3 Tag \n");
        return FAILURE;
    }
    return SUCCESS;
}
//check version
Status check_version(ToviewInfo *Toview)
{
    unsigned char version;
    if(fread(&version , 1, 1, Toview->fptr_mp3_file) != 1)
    {
        printf("ERROR: Unable to read ID3 version\n");
        return FAILURE;   
    }
    if(version != 3 && version  != 4)
    {
        printf("ERROR : Invalid version\n");
        return FAILURE;
    }  
    Toview->version = version;
    return SUCCESS;
}
//skip header
Status skip_header(ToviewInfo *Toview)
{
    unsigned char header[6];
    if(fread(header, 1, 6, Toview->fptr_mp3_file) != 6)
    {
        printf("ERROR : Unable to read ID3 header\n");
        return FAILURE;
    }
    Toview->size = ((unsigned int)(header[2] & 0x7F) << 21) |((unsigned int)(header[3] & 0x7F) << 14) |
                   ((unsigned int)(header[4] & 0x7F) << 7) |(unsigned int)(header[5] & 0x7F);
    return SUCCESS;
}
Status mp3_view(ToviewInfo *Toview)
{       
    strcpy(Toview->title, "Not Available");
    strcpy(Toview->artist, "Not Available");    
    strcpy(Toview->album, "Not Available");
    strcpy(Toview->year, "Not Available");
    strcpy(Toview->genre, "Not Available");
    
    
    Toview->loop = 0;
    Toview->padding = 0;
    Toview->remaining_size = Toview->size;

    //printf("Total ID3 Size: %u\n", Toview->size);

    while (Toview->remaining_size >= 10)
    {
        if (read_tag(Toview) == FAILURE)
        {
            return FAILURE;
        }
        // Stop if padding is found
        if (Toview->padding == 1)
        {
            break;
        }

        if (read_size(Toview) == FAILURE)
        {
            return FAILURE;
        }

        if (Toview->frame_size > Toview->remaining_size - 10)
        {
            printf("ERROR : Invalid frame size\n");
            return FAILURE;
        }
        if (skip_frame_flags(Toview) == FAILURE)
        {
            return FAILURE;
        }

        if (Read_contents(Toview) == FAILURE)
        {
            return FAILURE;
        }

        Toview->remaining_size -= (10 + Toview->frame_size);
        Toview->loop++;
    }
    printf("\nTITLE  : %s", Toview->title);
    printf("\nARTIST : %s", Toview->artist);    
    printf("\nALBUM  : %s", Toview->album);
    printf("\nYEAR   : %s", Toview->year);
    printf("\nGENRE  : %s\n", Toview->genre);
    printf("\n");
    return SUCCESS;
}
// reading tag 
Status read_tag(ToviewInfo *Toview)
{
    char tag[5];

    if (fread(tag, 1, 4, Toview->fptr_mp3_file) != 4)
    {
        printf("ERROR : Unable to read frame ID\n");
        return FAILURE;
    }

    tag[4] = '\0';

    /*printf("Read bytes: %02X %02X %02X %02X\n",
           (unsigned char)tag[0],
           (unsigned char)tag[1],
           (unsigned char)tag[2],
           (unsigned char)tag[3]);*/

    int i = 0;

    while (i < 4 && tag[i] == '\0')
    {
        i++;
    }

    if (i == 4)
    {
        Toview->padding = 1;
        return SUCCESS;
    }
    strcpy(Toview->frame_id, tag);
    //printf("Frame ID: %s\n", tag);
    return SUCCESS;
}

//read size
Status read_size(ToviewInfo *Toview)
{
    unsigned char size_bytes[4];

    if(fread(size_bytes,1,4,Toview->fptr_mp3_file) != 4)
    {
        printf("ERROR : Unable read the size\n");
        return FAILURE;
    }
    if(Toview->version == 3)
    {
        Toview->frame_size = ((unsigned int)size_bytes[0] << 24) |((unsigned int)size_bytes[1] << 16) |
                     ((unsigned int)size_bytes[2] << 8) |(unsigned int)size_bytes[3];
    }
    else if(Toview -> version == 4)
    {
        Toview->frame_size = ((size_bytes[0] & 0x7F) << 21) |((size_bytes[1] & 0x7F) << 14) 
                        |((size_bytes[2] & 0x7F) << 7)  | (size_bytes[3] & 0x7F);
    }
    return SUCCESS;
}

//read contents from file 
Status Read_contents(ToviewInfo *Toview)
{
    char *data;

    data = malloc(Toview->frame_size + 1);
    if (data == NULL)
    {
        printf("ERROR : Memory allocation failed\n");
        return FAILURE;
    }

    if (fread(data, 1, Toview->frame_size,
              Toview->fptr_mp3_file) != Toview->frame_size)
    {
        printf("ERROR : Unable to read frame content\n");
        free(data);
        return FAILURE;
    }

    data[Toview->frame_size] = '\0';

    // Skip text encoding byte
    char *content = data + 1;

    if (strcmp(Toview->frame_id, "TIT2") == 0)
    {
        strcpy(Toview->title, content);
    }
    else if (strcmp(Toview->frame_id, "TPE1") == 0)
    {
        strcpy(Toview->artist, content);
    }
    else if (strcmp(Toview->frame_id, "TALB") == 0)
    {
        strcpy(Toview->album, content);
    }
    else if (strcmp(Toview->frame_id, "TYER") == 0)
    {
        strcpy(Toview->year, content);
    }
    else if (strcmp(Toview->frame_id, "TCON") == 0)
    {
        strcpy(Toview->genre, content);
    }
    free(data);
    return SUCCESS;
}

Status skip_frame_flags(ToviewInfo *Toview)
{
    if (fseek(Toview->fptr_mp3_file, 2, SEEK_CUR) != 0)
    {
        printf("ERROR : Unable to skip frame flags\n");
        return FAILURE;
    }

    return SUCCESS;
}