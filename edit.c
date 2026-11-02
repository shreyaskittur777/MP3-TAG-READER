
#include "common.h"

Status to_edit(char *argv[], ToviewInfo *Toview)
{
    Toview->old_data = NULL;
    Toview->new_data = NULL;
    Toview->temp_file = NULL;

    if (open_files(Toview) == FAILURE)
        return FAILURE;

    if (check_ID_tag(Toview) == FAILURE)
        goto cleanup;

    if (check_version(Toview) == FAILURE)
        goto cleanup;

    if (check_edit_option(argv, Toview) == FAILURE)
        goto cleanup;

    if (skip_header(Toview) == FAILURE)
        goto cleanup;

    Toview->original_tag_size = Toview->size;
    Toview->frame_header_size = 10;
    Toview->remaining_size = Toview->size;

    if (find_edit_tag(Toview) == FAILURE)
        goto cleanup;

    if (read_old_frame(Toview) == FAILURE)
        goto cleanup;

    if (create_new_frame(Toview) == FAILURE)
        goto cleanup;

    if (open_temp_file(Toview) == FAILURE)
        goto cleanup;

    if (copy_id3_header(Toview) == FAILURE)
        goto cleanup;

    if (copy_before_selected_frame(Toview) == FAILURE)
        goto cleanup;

    if (write_new_frame(Toview) == FAILURE)
        goto cleanup;

    if (copy_remaining_data(Toview) == FAILURE)
        goto cleanup;

    if (update_id3_size(Toview) == FAILURE)
        goto cleanup;

    if (replace_original_file(Toview) == FAILURE)
        goto cleanup;

    printf("\nMP3 file edited successfully!\n");

    free(Toview->old_data);
    Toview->old_data = NULL;

    free(Toview->new_data);
    Toview->new_data = NULL;

    return SUCCESS;

cleanup:

    if (Toview->fptr_mp3_file != NULL)
    {
        fclose(Toview->fptr_mp3_file);
        Toview->fptr_mp3_file = NULL;
    }

    if (Toview->temp_file != NULL)
    {
        fclose(Toview->temp_file);
        Toview->temp_file = NULL;
    }

    free(Toview->old_data);
    Toview->old_data = NULL;

    free(Toview->new_data);
    Toview->new_data = NULL;

    return FAILURE;
}

Status check_edit_option(char *argv[], ToviewInfo *Toview)
{
    if (strcmp(argv[2], "-t") == 0)
        Toview->edit_tag = "TIT2";

    else if (strcmp(argv[2], "-a") == 0)
        Toview->edit_tag = "TPE1";

    else if (strcmp(argv[2], "-A") == 0)
        Toview->edit_tag = "TALB";

    else if (strcmp(argv[2], "-y") == 0)
        Toview->edit_tag = "TYER";

    else if (strcmp(argv[2], "-g") == 0)
        Toview->edit_tag = "TCON";

    else
    {
        printf("ERROR: Invalid edit option\n");
        return FAILURE;
    }

    Toview->new_value = argv[3];

    printf("Selected Tag : %s\n", Toview->edit_tag);
    printf("New Value   : %s\n", Toview->new_value);

    return SUCCESS;
}

Status find_edit_tag(ToviewInfo *Toview)
{
    char frame_id[5];
    unsigned char size_bytes[4];
    unsigned int frame_size;

    while (Toview->remaining_size >= 10)
    {
        Toview->selected_frame_position =
            ftell(Toview->fptr_mp3_file);

        if (fread(frame_id, 1, 4,
                   Toview->fptr_mp3_file) != 4)
            return FAILURE;

        frame_id[4] = '\0';

        if (frame_id[0] == '\0')
        {
            Toview->frame_size = 0;
            printf("INFO: Tag not found. Adding new tag.\n");
            return SUCCESS;
        }

        if (fread(size_bytes, 1, 4,
                   Toview->fptr_mp3_file) != 4)
            return FAILURE;

        frame_size =
            ((unsigned int)size_bytes[0] << 24) |
            ((unsigned int)size_bytes[1] << 16) |
            ((unsigned int)size_bytes[2] << 8) |
            (unsigned int)size_bytes[3];

        if (frame_size > Toview->remaining_size - 10)
            return FAILURE;

        if (strcmp(frame_id, Toview->edit_tag) == 0)
        {
            Toview->frame_size = frame_size;
            return SUCCESS;
        }

        if (fseek(Toview->fptr_mp3_file,
                  2 + frame_size, SEEK_CUR) != 0)
            return FAILURE;

        Toview->remaining_size -= 10 + frame_size;
    }

    Toview->selected_frame_position =
        ftell(Toview->fptr_mp3_file);

    Toview->frame_size = 0;

    //printf("INFO: Tag not found. Adding new tag.\n");

    return SUCCESS;
}

Status read_old_frame(ToviewInfo *Toview)
{
    if (Toview->frame_size == 0)
    {
        Toview->old_frame_size = 0;
        Toview->old_frame_flags[0] = 0;
        Toview->old_frame_flags[1] = 0;
        Toview->old_data = NULL;

        return SUCCESS;
    }

    if (fread(Toview->old_frame_flags, 1, 2,
              Toview->fptr_mp3_file) != 2)
    {
        printf("ERROR: Unable to read frame flags\n");
        return FAILURE;
    }

    Toview->old_frame_size = Toview->frame_size;

    Toview->old_data = malloc(Toview->old_frame_size);

    if (Toview->old_data == NULL)
    {
        printf("ERROR: Memory allocation failed\n");
        return FAILURE;
    }

    if (fread(Toview->old_data, 1,
              Toview->old_frame_size,
              Toview->fptr_mp3_file) != Toview->old_frame_size)
    {
        printf("ERROR: Unable to read old frame content\n");
        return FAILURE;
    }

    return SUCCESS;
}

Status create_new_frame(ToviewInfo *Toview)
{
    unsigned int text_length;

    text_length = strlen(Toview->new_value);

    Toview->new_frame_size = text_length + 1;

    Toview->new_data = malloc(Toview->new_frame_size + 1);

    if (Toview->new_data == NULL)
    {
        printf("ERROR: Memory allocation failed\n");
        return FAILURE;
    }

    Toview->new_data[0] = 0;

    memcpy(Toview->new_data + 1,
           Toview->new_value,
           text_length + 1);

    return SUCCESS;
}

Status open_temp_file(ToviewInfo *Toview)
{
    Toview->temp_file = fopen("temp.mp3", "wb");

    if (Toview->temp_file == NULL)
    {
        printf("ERROR: Unable to create temporary file\n");
        return FAILURE;
    }

    return SUCCESS;
}

Status copy_id3_header(ToviewInfo *Toview)
{
    unsigned char header[10];

    rewind(Toview->fptr_mp3_file);

    if (fread(header, 1, 10,
               Toview->fptr_mp3_file) != 10)
    {
        printf("ERROR: Unable to read ID3 header\n");
        return FAILURE;
    }

    if (fwrite(header, 1, 10,
               Toview->temp_file) != 10)
    {
        printf("ERROR: Unable to write ID3 header\n");
        return FAILURE;
    }

    return SUCCESS;
}

Status copy_before_selected_frame(ToviewInfo *Toview)
{
    unsigned char buffer[1024];
    long bytes_to_copy;
    size_t bytes_read;

    bytes_to_copy =
        Toview->selected_frame_position - 10;

    if (fseek(Toview->fptr_mp3_file, 10, SEEK_SET) != 0)
        return FAILURE;

    while (bytes_to_copy > 0)
    {
        size_t chunk_size;

        chunk_size =
            (bytes_to_copy > sizeof(buffer))
            ? sizeof(buffer)
            : (size_t)bytes_to_copy;

        bytes_read = fread(buffer, 1, chunk_size,
                           Toview->fptr_mp3_file);

        if (bytes_read != chunk_size)
            return FAILURE;

        if (fwrite(buffer, 1, bytes_read,
                   Toview->temp_file) != bytes_read)
            return FAILURE;

        bytes_to_copy -= bytes_read;
    }

    return SUCCESS;
}

Status write_new_frame(ToviewInfo *Toview)
{
    unsigned char frame_size[4];

    frame_size[0] =
        (Toview->new_frame_size >> 24) & 0xFF;

    frame_size[1] =
        (Toview->new_frame_size >> 16) & 0xFF;

    frame_size[2] =
        (Toview->new_frame_size >> 8) & 0xFF;

    frame_size[3] =
        Toview->new_frame_size & 0xFF;

    if (fwrite(Toview->edit_tag, 1, 4,
               Toview->temp_file) != 4)
        return FAILURE;

    if (fwrite(frame_size, 1, 4,
               Toview->temp_file) != 4)
        return FAILURE;

    if (fwrite(Toview->old_frame_flags, 1, 2,
               Toview->temp_file) != 2)
        return FAILURE;

    if (fwrite(Toview->new_data, 1,
               Toview->new_frame_size,
               Toview->temp_file) != Toview->new_frame_size)
        return FAILURE;

    return SUCCESS;
}

Status copy_remaining_data(ToviewInfo *Toview)
{
    unsigned char buffer[1024];
    size_t bytes_read;
    long remaining_position;

    if (Toview->old_frame_size == 0)
    {
        remaining_position =
            10 + Toview->original_tag_size;
    }
    else
    {
        remaining_position =
            Toview->selected_frame_position +
            Toview->frame_header_size +
            Toview->old_frame_size;
    }

    if (fseek(Toview->fptr_mp3_file,
              remaining_position, SEEK_SET) != 0)
        return FAILURE;

    while ((bytes_read = fread(buffer, 1,
                               sizeof(buffer),
                               Toview->fptr_mp3_file)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read,
                   Toview->temp_file) != bytes_read)
            return FAILURE;
    }

    return SUCCESS;
}

Status update_id3_size(ToviewInfo *Toview)
{
    unsigned char size_bytes[4];

    if (Toview->old_frame_size == 0)
    {
        Toview->updated_tag_size =
            (Toview->selected_frame_position - 10)
            + 10
            + Toview->new_frame_size;
    }
    else
    {
        Toview->updated_tag_size =
            Toview->original_tag_size
            - Toview->old_frame_size
            + Toview->new_frame_size;
    }

    size_bytes[0] =
        (Toview->updated_tag_size >> 21) & 0x7F;

    size_bytes[1] =
        (Toview->updated_tag_size >> 14) & 0x7F;

    size_bytes[2] =
        (Toview->updated_tag_size >> 7) & 0x7F;

    size_bytes[3] =
        Toview->updated_tag_size & 0x7F;

    if (fseek(Toview->temp_file, 6, SEEK_SET) != 0)
        return FAILURE;

    if (fwrite(size_bytes, 1, 4,
               Toview->temp_file) != 4)
        return FAILURE;

    return SUCCESS;
}

Status replace_original_file(ToviewInfo *Toview)
{
    if (fclose(Toview->temp_file) != 0)
    {
        Toview->temp_file = NULL;
        printf("ERROR: Unable to close temporary file\n");
        return FAILURE;
    }

    Toview->temp_file = NULL;

    fclose(Toview->fptr_mp3_file);
    Toview->fptr_mp3_file = NULL;

    if (rename(Toview->mp3_fname, "backup.mp3") != 0)
    {
        printf("ERROR: Unable to create backup file\n");
        return FAILURE;
    }

    if (rename("temp.mp3", Toview->mp3_fname) != 0)
    {
        printf("ERROR: Unable to replace original MP3\n");
        rename("backup.mp3", Toview->mp3_fname);
        return FAILURE;
    }

    remove("backup.mp3");

    return SUCCESS;
}
