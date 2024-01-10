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
 * @brief Instance of a query in a query file.
 *
 * @var query_instance::type
 *     @brief The type of a query, i.e. what defines what it does, i.e. the first number in the
 *            line.
 * @var query_instance::formatted
 *     @brief If the query's output should be formatted (pretty printed).
 * @var query_instance::number_in_file
 *     @brief When this query comes from a file, this is the number of the line the query was on.
 * @var query_instance::argument_data_free
 *     @brief Method that should be used to free ::query_instance::argument_data.
 * @var query_instance::argument_data
 *     @brief The arguments of this query, after being parsed by the specific query type.
 */
struct query_instance {
    size_t type;
    int    formatted;

    size_t number_in_file;

    void *argument_data;
};

query_instance_t *query_instance_create(void) {
    query_instance_t *ret = malloc(sizeof(struct query_instance));
    if (!ret)
        return NULL;

    /* Invalid data so that deallocations don't deal with uninitialized data */
    ret->type          = 0;
    ret->argument_data = NULL;
    return ret;
}

query_instance_t *query_instance_clone(const query_instance_t  *query,
                                       const query_type_list_t *query_type_list) {

    query_instance_t *clone = malloc(sizeof(query_instance_t));
    if (!clone)
        return NULL;
    memcpy(clone, query, sizeof(query_instance_t));

    const query_type_t *type = query_type_list_get_by_index(query_type_list, query->type);
    if (!type) {
        return NULL; /* Invalid query type */
    } else if (query->argument_data != NULL) {
        query_type_clone_arguments_callback_t cb = query_type_get_clone_arguments_callback(type);
        clone->argument_data                     = cb(query->argument_data);
        if (!clone->argument_data) {
            free(clone);
            return NULL;
        }
    }

    return clone;
}

void query_instance_set_type(query_instance_t *query, size_t type) {
    query->type = type;
}

void query_instance_set_formatted(query_instance_t *query, int formatted) {
    query->formatted = formatted;
}

void query_instance_set_number_in_file(query_instance_t *query, size_t number_in_file) {
    query->number_in_file = number_in_file;
}

int query_instance_set_argument_data(query_instance_t        *query,
                                     const void              *argument_data,
                                     const query_type_list_t *query_type_list) {

    const query_type_t *type = query_type_list_get_by_index(query_type_list, query->type);
    if (!type) {
        return 1; /* Invalid query type */
    }

    if (query->argument_data) {
        query_type_free_query_instance_argument_data_callback_t free_cb =
            query_type_get_free_query_instance_argument_data_callback(type);
        free_cb(query->argument_data);
        return 1;
    }

    query_type_clone_arguments_callback_t clone_cb = query_type_get_clone_arguments_callback(type);
    query->argument_data                           = clone_cb(argument_data);
    if (!query->argument_data) {
        return 1;
    }

    return 0;
}

size_t query_instance_get_type(const query_instance_t *query) {
    return query->type;
}

int query_instance_get_formatted(const query_instance_t *query) {
    return query->formatted;
}

size_t query_instance_get_number_in_file(const query_instance_t *query) {
    return query->number_in_file;
}

const void *query_instance_get_argument_data(const query_instance_t *query) {
    return query->argument_data;
}

void query_instance_free(query_instance_t *query, const query_type_list_t *query_type_list) {
    const query_type_t *type = query_type_list_get_by_index(query_type_list, query->type);
    if (!type) {
        return; /* Invalid query type */
    } else if (query->argument_data != NULL) {
        query_type_free_query_instance_argument_data_callback_t cb =
            query_type_get_free_query_instance_argument_data_callback(type);
        cb(query->argument_data);
    }
    free(query);
}
