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
 * @file  path_utils.c
 * @brief Implementation of methods in include/utils/path_utils.h
 *
 * ### Examples
 * See [the header file's documentation](@ref path_utils_examples).
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "utils/path_utils.h"
#include "utils/string_utils.h"

/**
 * @struct path_normalize_iter_data_t
 * @brief Data passed to ::__path_normalize_callback, so that it can transform a path into a stack.
 *
 * @var path_normalize_iter_data_t::stack
 *     @brief Slash-separated parts of path that will be outputted.
 * @var path_normalize_iter_data_t::first_token
 *     @brief `1` if this is the first token being processed (`0` otherwise).
 * @var path_normalize_iter_data_t::first_token_empty
 *     @brief If the first token was empty (path is absolute because it starts with `/`).
 */
typedef struct {
    GPtrArray *stack;
    int        first_token, first_token_empty;
} path_normalize_iter_data_t;

/**
 * @brief   Method called for every part of the path separated by slashes.
 * @details Auxiliary method for ::path_normalize.
 *
 * @param user_data A pointer to a ::path_normalize_iter_data_t.
 * @param token     Current part of the path being processed.
 *
 * @retval 0 Always, not to stop tokenization.
 */
int __path_normalize_callback(void *user_data, char *token) {
    path_normalize_iter_data_t *iter_data = (path_normalize_iter_data_t *) user_data;

    if (*token) { /* Skip empty tokens, this is, remove consecutive separators */
        if (strcmp(token, ".") == 0) {
            /* Only add . to the beginning of relative paths */

            if (iter_data->first_token)
                g_ptr_array_add(iter_data->stack, strdup("."));
        } else if (strcmp(token, "..") == 0) {

            if (iter_data->stack->len == 0) {
                /* Only add .. to the beginning if the path is relative (dont' output /..) */
                if (!iter_data->first_token_empty)
                    g_ptr_array_add(iter_data->stack, strdup(".."));

            } else if (strcmp(g_ptr_array_index(iter_data->stack, iter_data->stack->len - 1),
                              "..") == 0) {

                /*
                 * On non-empty paths, only add .. if the last directory is also .. (case where
                 * you can't remove the previous directory from the path).
                 */
                g_ptr_array_add(iter_data->stack, strdup(".."));

            } else if (iter_data->stack->len == 1 &&
                       strcmp(g_ptr_array_index(iter_data->stack, 0), ".") == 0) {

                /*
                 * When the path is is relative and starts with dot (./..), replace the
                 * first directory with ..
                 */

                free(g_ptr_array_index(iter_data->stack, 0));
                g_ptr_array_index(iter_data->stack, 0) = strdup("..");

            } else {

                /* Normal case: remove last directory */
                free(g_ptr_array_index(iter_data->stack, iter_data->stack->len - 1));
                g_ptr_array_set_size(iter_data->stack, iter_data->stack->len - 1);
            }
        } else {
            /* Neither . or .. -> add directory to stack */
            g_ptr_array_add(iter_data->stack, strdup(token));
        }
    } else if (iter_data->first_token) {
        iter_data->first_token_empty = 1;
    }

    iter_data->first_token = 0;
    return 0;
}

void path_normalize(char *path) {
    path_normalize_iter_data_t iter_data = {.stack             = g_ptr_array_new(),
                                            .first_token       = 1,
                                            .first_token_empty = 0};

    string_tokenize(path, '/', __path_normalize_callback, &iter_data);

    /* Regenerate path from stack */
    if (iter_data.stack->len == 0) {
        if (strlen(path) != 0)
            strcpy(path, "/");
        else
            strcpy(path, ".");
    } else {
        char  new_path[PATH_MAX];
        char *write = new_path;

        if (iter_data.first_token_empty) {
            *write = '/';
            write++;
        }

        for (size_t i = 0; i < iter_data.stack->len; ++i) {
            write  = stpcpy(write, g_ptr_array_index(iter_data.stack, i));
            *write = '/';
            write++;
        }
        *(write - 1) = '\0'; /* Remove last backslash and terminate the string */

        strcpy(path, new_path);
    }

    for (size_t i = 0; i < iter_data.stack->len; ++i)
        free(g_ptr_array_index(iter_data.stack, i));
    g_ptr_array_unref(iter_data.stack);
}

void path_concat(char *path, const char *add) {
    char new_path[PATH_MAX];
    snprintf(new_path, PATH_MAX, "%s/%s", path, add);
    path_normalize(new_path);
    strcpy(path, new_path);
}
