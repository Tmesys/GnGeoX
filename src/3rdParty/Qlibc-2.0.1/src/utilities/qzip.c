/******************************************************************************
 * qLibc
 *
 * Copyright (c) 2010-2015 Seungyoung Kim.
 * Copyright (c) ? Mathieu Peponas.
 * Copyright (c) ? Joonas Pihlajamaa.
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
 * @file qzip.c Zip file decompression APIs.
 */
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "utilities/qzip.h"

/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
static bool search_eof_central_dir ( qzip_file_t* zip_file )
{
    long readBytes = 0, i = 0;
    qzip_cd_end_record_t *zip_tmp_end_record;
    unsigned char buffer[BUFFER_SIZE]; // limits maximum zip descriptor size

    /*
    Depending on the comment length, the start of the EOCD will be at different offsets
    from the end of file.

    If n=0 (empty comment), the EOCD starts at 22 bytes from the end
    If n=0xffff (max length comment), the EOCD starts at 22 + 0xffff = 65557 bytes from
    the end
    The interval where the EOCD signature may exist is between 65557 and 18 from the end.
    That is a total of about 65.5 kb. That is not much on a modern computer so we can read that
    whole interval into a buffer and scan it backwards to find the signature.
    */

    // Fill the buffer, but at most the whole file
    readBytes = ( zip_file->file_size < sizeof ( buffer ) ) ? zip_file->file_size : sizeof ( buffer );
    fseek ( zip_file->file, zip_file->file_size - readBytes, SEEK_SET );
    fread ( buffer, 1, readBytes, zip_file->file );

    // Naively assume signature can only be found in one place...
    for ( i = readBytes - sizeof ( qzip_cd_end_record_t ); i >= 0; i-- )
    {
        zip_tmp_end_record = ( qzip_cd_end_record_t * ) ( buffer + i );
        if ( zip_tmp_end_record->signature == 0x06054B50 )
        {
            break;
        }
    }

    if ( i < 0 )
    {
        printf ( "End record signature not found in zip!" );
        return false;
    }

    memcpy ( &zip_file->zip_end_record, zip_tmp_end_record, sizeof ( qzip_cd_end_record_t ) );

    if ( zip_file->zip_end_record.disk_number != 0 )
    {
        printf ( "Multi disk zip files are not supported (%d)\n", zip_file->zip_end_record.disk_number );
        return false;
    }

    if ( zip_file->zip_end_record.num_entries == 0 )
    {
        printf ( "No files in zip\n" );
        return false;
    }

    return true;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
static bool search_central_dir ( qzip_file_t* zip_file, qzip_entry_t *zip_entry )
{
    unsigned char zip_file_name[FILENAME_MAX];
    qzip_local_file_header_t tmp_file_header;

    // Go to the beginning of central directory
    fseek ( zip_file->file, zip_file->zip_end_record.central_directory_offset, SEEK_SET );

    for ( int i = 0; i < zip_file->zip_end_record.num_entries; i++ )
    {
        fread ( &zip_entry->file_header, 1, sizeof ( qzip_cd_file_header_t ), zip_file->file );

        // Check signature
        if ( zip_entry->file_header.signature != 0x02014B50 )
        {
            return false;
        }

        fread ( zip_file_name, 1, zip_entry->file_header.filename_length, zip_file->file ); // read filename
        zip_file_name[zip_entry->file_header.filename_length] = 0;

        if ( strcmp ( zip_file_name, zip_entry->file_name ) != 0 )
        {
            fseek ( zip_file->file, zip_entry->file_header.extra_field_length, SEEK_CUR ); // skip
            fseek ( zip_file->file, zip_entry->file_header.file_comment_length, SEEK_CUR ); // skip
            continue;
        }

        if ( zip_entry->file_header.compression_method != 0
                && zip_entry->file_header.compression_method != 8 )
        {
            printf ( "Unsupported compression method : %d\n", zip_entry->file_header.compression_method );
            return false;
        }

        fseek ( zip_file->file, zip_entry->file_header.extra_field_length, SEEK_CUR ); // skip
        fseek ( zip_file->file, zip_entry->file_header.file_comment_length, SEEK_CUR ); // skip

        break;
    }

    // Go to the beginning of local file section
    fseek ( zip_file->file, zip_entry->file_header.relative_offset_of_local_header, SEEK_SET );
    fread ( &tmp_file_header, 1, sizeof ( qzip_local_file_header_t ), zip_file->file );
    fseek ( zip_file->file, tmp_file_header.filename_length, SEEK_CUR ); // skip
    fseek ( zip_file->file, tmp_file_header.extra_field_length, SEEK_CUR ); // skip

    zip_entry->compressed_data_position = ftell ( zip_file->file );

    return true;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
qzip_entry_t* qzip_open_entry ( qzip_file_t* zip_file, const char* file_name, uint32_t file_crc )
{
    qzip_entry_t* zip_entry = NULL;

    zip_entry = calloc ( 1, sizeof ( qzip_entry_t ) );
    if ( zip_entry == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    strcpy ( zip_entry->file_name, file_name );

    if ( search_central_dir ( zip_file, zip_entry ) == true )
    {
        if ( file_crc != 0 && zip_entry->file_header.crc32 != file_crc )
        {
            printf ( "CRC does not match : %d vs provided %d\n", zip_entry->file_header.crc32, file_crc );
        }

        if ( zip_entry->file_header.compression_method == 8 )
        {
            zip_entry->zlib_stream = calloc ( 1, sizeof ( z_stream ) );
            if ( zip_entry->zlib_stream == NULL )
            {
                errno = ENOMEM;
                free ( zip_entry );
                return NULL;
            }

            zip_entry->inbuf = zip_file->map + zip_entry->compressed_data_position;

            zip_entry->zlib_stream->avail_in = zip_entry->file_header.compressed_size;
            zip_entry->zlib_stream->next_in = zip_entry->inbuf;

            if ( inflateInit2 ( zip_entry->zlib_stream, -MAX_WBITS ) != Z_OK )
            {
                printf ( "Error initializing decompression\n" );
                return NULL;
            }
        }

        zip_entry->readed = 0;
        zip_entry->file_io = zip_file->file;

        return zip_entry;
    }

    return NULL;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
bool qzip_seek_entry ( qzip_entry_t* zip_entry, uint32_t offset )
{
    uint8_t* buf = NULL;
    uint32_t s = 4096, c = 0;

    buf = ( uint8_t* ) calloc ( 1, s );
    if ( buf == NULL )
    {
        errno = ENOMEM;
        return ( false );
    }

    while ( offset )
    {
        c = offset;

        if ( c > s )
        {
            c = s;
        }

        c = qzip_read_entry ( zip_entry, buf, c );

        if ( c == 0 )
        {
            break;
        }

        offset -= c;
    }

    free ( buf );

    return ( true );
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
int32_t qzip_read_entry ( qzip_entry_t* zip_entry, uint8_t* data, uint32_t size )
{
    int32_t readed = 0;
    int32_t todo = 0;
    int32_t ret = 0;

    if ( zip_entry == NULL )
    {
        errno = EINVAL;
        return -1;
    }

    if ( zip_entry->file_header.compression_method == 8 )
    {
        zip_entry->zlib_stream->next_out = data;
        zip_entry->zlib_stream->avail_out = size;

        ret = inflate ( zip_entry->zlib_stream, Z_NO_FLUSH );
        if ( ret == Z_STREAM_ERROR )
        {
            printf ( "Error in decompression\n" );
            return ret;
        }

        switch ( ret )
        {
        case ( Z_NEED_DICT ) :
            {
                ret = Z_DATA_ERROR; /* and fall through */
            }
        case ( Z_DATA_ERROR ) :
        case ( Z_MEM_ERROR ) :
            {
                if ( inflateEnd ( zip_entry->zlib_stream ) != Z_OK )
                {
                    printf ( "Error finalizing decompression\n" );
                }

                return ret;
            }
        }

        readed = ( size - zip_entry->zlib_stream->avail_out );
    }
    else     /* Stored */
    {
        todo = ( zip_entry->readed - zip_entry->file_header.uncompressed_size );

        if ( todo < size )
        {
            todo = size;
        }

        readed = fread ( data, 1, size, zip_entry->file_io );
        zip_entry->readed += readed;
    }

    //zip_entry->compressed_data_position = ftell ( zip_entry->file_io );

    return readed;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
uint8_t* qzip_load_entry ( qzip_file_t* zip_file, const char* filename, uint32_t* outlen )
{
    uint32_t unc_size = 0;

    if ( zip_file == NULL )
    {
        errno = EINVAL;
        return NULL;
    }

    qzip_entry_t* zip_entry = qzip_open_entry ( zip_file, filename, 0 );
    if ( zip_entry == NULL )
    {
        return NULL;
    }

    uint8_t* data = calloc ( 1, zip_entry->file_header.uncompressed_size );
    if ( !data )
    {
        errno = ENOMEM;
        return NULL;
    }

    unc_size = qzip_read_entry ( zip_entry, data, zip_entry->file_header.uncompressed_size );
    if ( unc_size != zip_entry->file_header.uncompressed_size )
    {
        printf ( "Readed data size different from uncompressed size %d!=%d \n",
                 unc_size, zip_entry->file_header.uncompressed_size );
    }

    *outlen = unc_size;

    qzip_close_entry ( zip_entry );

    return data;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qzip_close_entry ( qzip_entry_t* zip_entry )
{
    if ( zip_entry == NULL )
    {
        errno = EINVAL;
        return;
    }

    if ( zip_entry->file_header.compression_method == 8 )
    {
        inflateEnd ( zip_entry->zlib_stream );
        free ( zip_entry->zlib_stream );
    }

    free ( zip_entry );
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
qzip_file_t* qzip_open_file ( const char* file )
{
    qzip_file_t* zip_file = NULL;
    struct stat sb;

#ifdef __linux__

    if ( lstat ( file, &sb ) == -1 )
    {
        printf ( "Couldn't open %s\n", file );
        free ( zip_file );
        errno = EINVAL;

        return NULL;
    }

    if ( !S_ISREG ( sb.st_mode ) && !S_ISLNK ( sb.st_mode ) )
    {
        printf ( "%s is not a regular file\n", file );
        free ( zip_file );
        errno = EINVAL;

        return NULL;
    }

#endif

    zip_file = calloc ( 1, sizeof ( qzip_file_t ) );
    if ( zip_file == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    zip_file->file = fopen ( file, "rb" );
    if ( zip_file->file == NULL )
    {
        printf ( "Ho no! Couldn't open %s\n", file );
        free ( zip_file );
        return NULL;
    }

    fseek ( zip_file->file, 0, SEEK_END );
    zip_file->file_size = ftell ( zip_file->file );

    zip_file->map = mmap ( 0, zip_file->file_size, PROT_READ, MAP_SHARED, fileno ( zip_file->file ), 0 );

    if ( search_eof_central_dir ( zip_file ) == false )
    {
        printf ( "Strange %s\n", file );
        fclose ( zip_file->file );
        free ( zip_file );
        return NULL;
    }

    return zip_file;
}
/**
 * ????.
 *
 * @param value ???
 *
 * @return ???
 *
 */
void qzip_close_file ( qzip_file_t* zip_file )
{
    fseek ( zip_file->file, 0, SEEK_END );
    int32_t size = ftell ( zip_file->file );
    munmap ( zip_file->map, size );
    fclose ( zip_file->file );
    free ( zip_file );
}
