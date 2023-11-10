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
 * @file  query_type.c
 * @brief Implementation of methods in include/queries/query_type.h
 */

#include <stdlib.h>

#include "queries/query_type.h"

/**
 * @struct query_type
 * @brief  A query definition based on its behavior.
 *
 * @var query_type::parse_arguments
 *     @brief Method that parses query arguments and generates ::query_instance::argument_data.
 * @var query_type::free_query_instance_argument_data
 *     @brief Method that frees data in ::query_instance::argument_data.
 */
struct query_type {
    query_type_parse_arguments_callback                   parse_arguments;
    query_type_free_query_instance_argument_data_callback free_query_instance_argument_data;

    /* TODO - add other methods */
};

query_type_t *query_type_create(
    query_type_parse_arguments_callback                   parse_arguments,
    query_type_free_query_instance_argument_data_callback free_query_instance_argument_data) {

    query_type_t *query = malloc(sizeof(struct query_type));
    if (!query)
        return NULL;

    query->parse_arguments                   = parse_arguments;
    query->free_query_instance_argument_data = free_query_instance_argument_data;

    return query;
}

query_type_parse_arguments_callback query_type_get_parse_arguments_callback(query_type_t *type) {
    return type->parse_arguments;
}

query_type_free_query_instance_argument_data_callback
    query_type_get_free_query_instance_argument_data_callback(query_type_t *type) {

    return type->free_query_instance_argument_data;
}

void query_type_free(query_type_t *query) {
    free(query);
}
