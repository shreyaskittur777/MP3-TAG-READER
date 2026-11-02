#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef unsigned int uint;

typedef enum
{
    e_view,
    e_edit,
    e_unsupported
} OperationType;

typedef enum
{
    SUCCESS,
    FAILURE
} Status;


typedef struct Toview
{
    char *mp3_fname;
    FILE *fptr_mp3_file;

    unsigned int size;
    unsigned int frame_size;
    unsigned int remaining_size;

    unsigned char version;
    int padding;
    uint loop;

    // Frame IDs
    char title[100];
    char artist[100];
    char album[100];
    char year[10];
    char genre[50];
    char frame_id[5];

    // Editing members
    char *new_value;
    char *edit_tag;

    unsigned char *old_data;
    unsigned int old_frame_size;

    unsigned char *new_data;
    unsigned int new_frame_size;

    unsigned char old_frame_flags[2];

    long selected_frame_position;

    unsigned int original_tag_size;
    unsigned int updated_tag_size;
    unsigned int frame_header_size;

    FILE *temp_file;

} ToviewInfo;


/* Common functions */
//cheCK OPERATION TYPE
OperationType check_operation_type(char *argv[]);

// read and validate with extentions
Status read_and_validate(char *argv[], ToviewInfo *Toview);

/* View functions */
//view operation 
Status to_view(ToviewInfo *Toview);

//open files
Status open_files(ToviewInfo *Toview);

//check ID3 tag
Status check_ID_tag(ToviewInfo *Toview);

//check version
Status check_version(ToviewInfo *Toview);

Status mp3_view(ToviewInfo *Toview);

//skip header
Status skip_header(ToviewInfo *Toview);

//skip frame flags 
Status skip_frame_flags(ToviewInfo *Toview);

//read tag
Status read_tag(ToviewInfo *Toview);

//read size
Status read_size(ToviewInfo *Toview);

//read contents
Status Read_contents(ToviewInfo *Toview);

/* Edit functions */
Status to_edit(char *argv[], ToviewInfo *Toview);

//edit option checking 
Status to_edit(char *argv[], ToviewInfo *Toview);

Status check_edit_option(char *argv[], ToviewInfo *Toview);

Status find_edit_tag(ToviewInfo *Toview);

Status read_old_frame(ToviewInfo *Toview);

Status create_new_frame(ToviewInfo *Toview);

Status open_temp_file(ToviewInfo *Toview);

Status copy_id3_header(ToviewInfo *Toview);

Status copy_before_selected_frame(ToviewInfo *Toview);

Status write_new_frame(ToviewInfo *Toview);

Status copy_remaining_data(ToviewInfo *Toview);

Status update_id3_size(ToviewInfo *Toview);

Status replace_original_file(ToviewInfo *Toview);

#endif