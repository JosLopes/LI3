/*
 * Copyright 2023 Humberto Gomes, José Lopes, José Matos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file  query_writer.c
 * @brief Implementation of methods in include/queries/query_writer.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_writer_examples).
 */

#include <glib.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "queries/query_writer.h"
#include "utils/string_pool.h"

/**
 * @struct query_writer
 * @brief  Information about where to output query results to.
 *
 * @var query_writer::stream
 *     @brief Where to write query outputs to. May be `NULL` (see ::query_writer::lines in that
 *            case).
 * @var query_writer::formatted
 *     @brief If the output of the query should be formatted (pretty printed).
 * @var query_writer::is_first_field
 *     @brief Whether the next field to be printed is the first field of the current object.
 * @var query_writer::current_object
 *    @brief Number of the object currently being written (starts counting up from `1`).
 * @var query_writer::strings
 *     @brief In case ::query_writer::stream is `NULL`, this is where strings in
 *            ::query_writer::lines are allocated.
 * @var query_writer::lines
 *     @brief Lines of output of a query, only initialized if ::query_writer::stream is `NULL`.
 * @var query_writer::current_line
 *    @brief Current line being printed. Used for outputting non-formatted query results to strings.
 * @var query_writer::current_line_cursor
 *    @brief Position to start writing in ::query_write::current_line.
 */
struct query_writer {
    FILE *stream;

    int formatted;

    int    is_first_field;
    size_t current_object;

    string_pool_t *strings;
    GPtrArray     *lines;

    size_t current_line_cursor;
    char   current_line[LINE_MAX];
};

/** @brief Size of each pool block in ::query_writer::strings. */
#define QUERY_WRITER_STRING_POOL_BLOCK_SIZE (1 << 17)

query_writer_t *query_writer_create(const char *out_file_path, int formatted) {
    query_writer_t *ret = malloc(sizeof(query_writer_t));
    if (!ret)
        return NULL;

    ret->formatted           = formatted;
    ret->is_first_field      = 1;
    ret->current_object      = 1;
    ret->current_line_cursor = 0;

    if (out_file_path) {
        ret->stream = fopen(out_file_path, "w");
        if (!ret->stream) {
            free(ret);
            return NULL;
        }

        ret->strings = NULL;
        ret->lines   = NULL;
    } else {
        ret->stream  = NULL;
        ret->strings = string_pool_create(QUERY_WRITER_STRING_POOL_BLOCK_SIZE);
        if (!ret->strings) {
            free(ret);
            return NULL;
        }
        ret->lines = g_ptr_array_new();
    }

    return ret;
}

void query_writer_write_new_object(query_writer_t *writer) {
    if (writer->stream) {
        /* Spacing after last item (don't add spacing to the beginning of the file) */
        if (writer->current_object != 1)
            fputc('\n', writer->stream);

        /* Print object number for formatted output */
        if (writer->formatted)
            fprintf(writer->stream, "--- %zu ---\n", writer->current_object);
    } else {
        if (writer->current_object != 1) {
            if (writer->formatted) {
                /* Spacing after last item (don't add spacing to the beginning of the file) */
                char *empty = string_pool_put(writer->strings, "");
                g_ptr_array_add(writer->lines, empty);
            } else {
                /* Flush last time (it's invalid for the first object) */
                g_ptr_array_add(writer->lines,
                                string_pool_put(writer->strings, writer->current_line));
                writer->current_line_cursor = 0;
            }
        }

        /* Print object number for formatted output */
        if (writer->formatted) {
            char line[LINE_MAX];
            snprintf(line, LINE_MAX, "--- %zu ---", writer->current_object);
            g_ptr_array_add(writer->lines, string_pool_put(writer->strings, line));
        }
    }

    writer->current_object++;
    writer->is_first_field = 1;
}

void query_writer_write_new_field(query_writer_t *writer,
                                  const char     *key,
                                  const char     *format,
                                  ...) {
    va_list printf_args;
    va_start(printf_args, format);

    if (writer->stream) {

        if (writer->formatted) {
            /* Print line "key: value" */
            fprintf(writer->stream, "%s: ", key);
            vfprintf(writer->stream, format, printf_args);
            fputc('\n', writer->stream);
        } else {
            /* Print only values, adding semicolons between them */
            if (!writer->is_first_field) {
                fputc(';', writer->stream);
            }
            vfprintf(writer->stream, format, printf_args);
            writer->is_first_field = 0;
        }
    } else {
        if (writer->formatted) {
            /* Print line "key: value" */
            char   line[LINE_MAX];
            size_t len = snprintf(line, LINE_MAX, "%s: ", key);
            vsnprintf(line + len, LINE_MAX - len, format, printf_args);

            char *pool_line = string_pool_put(writer->strings, line);
            g_ptr_array_add(writer->lines, pool_line);
        } else {
            /*
             * Print only values, adding semicolons between them. This is done to
             * writer->current_line, which, when completed, is flushed to writer->lines.
             */

            if (!writer->is_first_field) {
                writer->current_line[writer->current_line_cursor] = ';';
                writer->current_line_cursor++;
            }

            writer->current_line_cursor +=
                vsnprintf(writer->current_line + writer->current_line_cursor,
                          LINE_MAX - writer->current_line_cursor,
                          format,
                          printf_args);

            writer->is_first_field = 0;
        }
    }

    va_end(printf_args);
}

const char *const *query_writer_get_lines(query_writer_t *writer, size_t *out_n) {
    if (writer->stream)
        return NULL;

    /* Flush last line when printing to a set of strings */
    if (writer->current_line_cursor != 0) {
        writer->current_line_cursor = 0;
        g_ptr_array_add(writer->lines, string_pool_put(writer->strings, writer->current_line));
    }

    *out_n = writer->lines->len;
    return (const char *const *) writer->lines->pdata;
}

void query_writer_free(query_writer_t *writer) {
    /* Flush missing last line before closing file */
    if (!writer->formatted && writer->stream &&
        !(writer->is_first_field && writer->current_object == 1))
        fputc('\n', writer->stream);

    if (writer->stream) {
        fclose(writer->stream);
    } else {
        string_pool_free(writer->strings);
        g_ptr_array_unref(writer->lines);
    }

    free(writer);
}
