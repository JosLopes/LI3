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
    return ret;
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

void query_instance_set_argument_data(query_instance_t *query, void *argument_data) {
    query->argument_data = argument_data;
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

void *query_instance_get_argument_data(const query_instance_t *query) {
    return query->argument_data;
}

void query_instance_free(query_instance_t *query) {
    /* TODO - free argument based on query type */
    free(query);
}
