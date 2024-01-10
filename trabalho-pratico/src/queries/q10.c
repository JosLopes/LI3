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
 * @file  q10.c
 * @brief Implementation of methods in include/queries/q10.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "queries/q10.h"
#include "queries/query_instance.h"

void *__q10_parse_arguments(char *const *argv, size_t argc) {
    (void) argv;
    (void) argc;

    return (void *) 1; /* Don't fail with NULL */
}

void *__q10_clone_arguments(const void *args_data) {
    (void) args_data;
    return (void *) 1;
}

void __q10_free_query_instance_argument_data(void *argument_data) {
    (void) argument_data;
}

void *__q10_generate_statistics(const database_t              *database,
                                const query_instance_t *const *instances,
                                size_t                         n) {
    (void) database;
    (void) instances;
    (void) n;

    return (void *) 1; /* Don't fail with NULL */
}

void __q10_free_statistics(void *statistics) {
    (void) statistics;
}

int __q10_execute(const database_t       *database,
                  const void             *statistics,
                  const query_instance_t *instance,
                  query_writer_t         *output) {
    (void) database;
    (void) statistics;
    (void) instance;
    (void) output;

    return 0;
}

query_type_t *q10_create(void) {
    return query_type_create(__q10_parse_arguments,
                             __q10_clone_arguments,
                             __q10_free_query_instance_argument_data,
                             __q10_generate_statistics,
                             __q10_free_statistics,
                             __q10_execute);
}
