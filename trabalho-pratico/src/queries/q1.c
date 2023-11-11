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
 * @file  q1.c
 * @brief Implementation of methods in include/queries/q1.h
 */

#include <stdio.h>

#include "queries/q1.h"

void *__q1_parse_arguments(char **argv, size_t argc) {
    (void) argv;
    (void) argc;

    printf("Parsing Q1 arguments!");

    return NULL; /* Fail */
}

void __q1_free_query_instance_argument_data(void *argument_data) {
    (void) argument_data;

    printf("Freeing Q1 argument data!");
}

query_type_t *q1_create(void) {
    return query_type_create(__q1_parse_arguments, __q1_free_query_instance_argument_data);
}
