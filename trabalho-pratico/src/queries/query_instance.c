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
 * @file  query_instance.c
 * @brief Implementation of methods in include/queries/query_instance.h
 *
 * ### Examples
 * See [the header file's documentation](@ref query_instance_examples).
 */

#include <stdlib.h>

#include "queries/query_instance.h"

/**
 * @struct query_instance
 * @brief  Instance of a query (in a file, or inputted by the user).
 *
 * @var query_instance::type
 *     @brief The type of a query, i.e. what defines what it does, i.e. the first number in the
 *            line.
 * @var query_instance::formatted
 *     @brief If the query's output should be formatted (pretty printed).
 * @var query_instance::line_in_file
 *     @brief The number of the line this query is in the input file (`1` for interactive mode).
 * @var query_instance::argument_data
 *     @brief The arguments of this query, after being parsed by the specific query type.
 */
struct query_instance {
    const query_type_t *type;
    int                 formatted;
    size_t              line_in_file;
    void               *argument_data;
};

query_instance_t *query_instance_create(void) {
    query_instance_t *const ret = malloc(sizeof(query_instance_t));
    if (!ret)
        return NULL;

    /* Invalid data so that deallocations and clones don't deal with uninitialized data. */
    ret->type          = NULL;
    ret->argument_data = NULL;
    return ret;
}

query_instance_t *query_instance_clone(const query_instance_t *query) {
    query_instance_t *const clone = malloc(sizeof(query_instance_t));
    if (!clone)
        return NULL;
    memcpy(clone, query, sizeof(query_instance_t));

    if (query->type && query->argument_data) {
        const query_type_clone_arguments_callback_t clone_cb =
            query_type_get_clone_arguments_callback(query->type);
        clone->argument_data = clone_cb(query->argument_data);
        if (!clone->argument_data) {
            free(clone);
            return NULL;
        }
    }
    return clone;
}

void query_instance_set_type(query_instance_t *query, const query_type_t *type) {
    query->type = type;
}

void query_instance_set_formatted(query_instance_t *query, int formatted) {
    query->formatted = formatted;
}

void query_instance_set_line_in_file(query_instance_t *query, size_t line_in_file) {
    query->line_in_file = line_in_file;
}

int query_instance_set_argument_data(query_instance_t *query, const void *argument_data) {
    if (!query->type)
        return 1;

    if (query->argument_data) {
        const query_type_free_arguments_callback_t free_cb =
            query_type_get_free_arguments_callback(query->type);
        free_cb(query->argument_data);
    }

    query_type_clone_arguments_callback_t clone_cb =
        query_type_get_clone_arguments_callback(query->type);
    query->argument_data = clone_cb(argument_data);
    if (!query->argument_data)
        return 1;
    return 0;
}

const query_type_t *query_instance_get_type(const query_instance_t *query) {
    return query->type;
}

int query_instance_get_formatted(const query_instance_t *query) {
    return query->formatted;
}

size_t query_instance_get_line_in_file(const query_instance_t *query) {
    return query->line_in_file;
}

const void *query_instance_get_argument_data(const query_instance_t *query) {
    return query->argument_data;
}

void query_instance_free(query_instance_t *query) {
    if (query->type) {
        const query_type_free_arguments_callback_t free_cb =
            query_type_get_free_arguments_callback(query->type);
        free_cb(query->argument_data);
    }
    free(query);
}
