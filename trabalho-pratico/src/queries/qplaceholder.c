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
 * @file  qplaceholder.c
 * @brief Implementation of methods in include/queries/qplaceholder.h
 */

#include <stdio.h>
#include <stdlib.h>

#include "queries/qplaceholder.h"
#include "queries/query_instance.h"

void *__qplaceholder_parse_arguments(char **argv, size_t argc) {
    printf("QPLACEHOLDER arguments: [ ");
    for (size_t i = 0; i < argc; ++i) {
        if (i == argc - 1)
            printf("\"%s\"", argv[i]);
        else
            printf("\"%s\", ", argv[i]);
    }
    printf(" ]\n");

    return (void *) 1; /* Don't fail with NULL */
}

void __qplaceholder_free_query_instance_argument_data(void *argument_data) {
    (void) argument_data;
}

int __qplaceholder_execute(database_t       *database,
                           void             *statistics,
                           query_instance_t *instance,
                           FILE             *output) {
    (void) database;
    (void) statistics;

    fprintf(output, "Query on line %zu\n", query_instance_get_number_in_file(instance));
    return 0;
}

query_type_t *qplaceholder_create(void) {
    return query_type_create(__qplaceholder_parse_arguments,
                             __qplaceholder_free_query_instance_argument_data,
                             NULL,
                             NULL,
                             __qplaceholder_execute);
}
