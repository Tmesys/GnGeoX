/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) 2021 Mourad Reggadi.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/**
 * qzip header file.
 *
 * @file qzip.h
 */

#ifndef QZIP_H
#define QZIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zlib.h>

#define BUFFER_SIZE 65536


typedef struct __attribute__ ( ( __packed__ ) )
{
    uint32_t signature; // 0x06054b50
    uint16_t disk_number; // Number of this disk
    uint16_t central_directory_disknumber; // Disk where central directory starts
    uint16_t num_entries_this_disk; // Numbers of central directory records on this disk
    uint16_t num_entries; // Total number of central directory records
    uint32_t central_directory_size; //Size of central directory in bytes
    uint32_t central_directory_offset; //Offset to start of central directory
    uint16_t comment_length; //Comment length (n)
    // Followed by .ZIP file comment (variable size)
} qzip_cd_end_record_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t signature; // 0x02014B50
    uint16_t version_made_by; // unsupported
    uint16_t version_needed_to_extract; // unsupported
    uint16_t general_purpose_bit_flag; // unsupported
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length; // unsupported
    uint16_t file_comment_length; // unsupported
    uint16_t disk_number_start; // unsupported
    uint16_t internal_file_attributes; // unsupported
    uint32_t external_file_attributes; // unsupported
    uint32_t relative_offset_of_local_header;
} qzip_cd_file_header_t;

typedef struct __attribute__ ((__packed__)) {
    uint32_t signature; // 0x02014B50
    uint16_t version_needed_to_extract; // unsupported
    uint16_t general_purpose_bit_flag; // unsupported
    uint16_t compression_method;
    uint16_t last_mod_file_time;
    uint16_t last_mod_file_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length; // unsupported
} qzip_local_file_header_t;

typedef struct qzip_entry_s qzip_entry_t;
struct qzip_entry_s
{
    qzip_cd_file_header_t file_header;
    FILE* file_io;
    z_streamp zlib_stream;
    char file_name[FILENAME_MAX];
    int compressed_data_position;
    char* inbuf;
    int readed;
};

typedef struct qzip_file_s qzip_file_t;
struct qzip_file_s
{
    qzip_cd_end_record_t zip_end_record;
    FILE* file;
    char* map;
    int file_size;
};

extern qzip_entry_t* qzip_open_entry ( qzip_file_t*, const char*, uint32_t );
extern bool qzip_seek_entry ( qzip_entry_t*, uint32_t );
extern int32_t qzip_read_entry ( qzip_entry_t*, uint8_t*, uint32_t );
extern uint8_t* qzip_load_entry ( qzip_file_t*, const char*, uint32_t* );
extern void qzip_close_entry ( qzip_entry_t* );
extern qzip_file_t* qzip_open_file ( const char* );
extern void qzip_close_file ( qzip_file_t* );

#ifdef __cplusplus
}
#endif

#endif /* QALLOC_H */

