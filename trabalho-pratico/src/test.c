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
 * @file main.c
 * @brief Contains the entry point to the program.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/dataset_parser.h"
#include "utils/fixed_n_delimiter_parser.h"
#include "utils/string_utils.h"

typedef struct {
    char *name;
    int   age, height;
} person_t;

typedef struct {
    int       n;
    person_t *person_array;
} person_parser_helper;

int parse_name(void *user_data, char *token, size_t ntoken) {
    (void) ntoken;
    person_parser_helper *helper = (person_parser_helper *) user_data;

    // Copy string to another buffer
    char *name_copy                      = string_duplicate(token);
    helper->person_array[helper->n].name = name_copy;
    return 0;
}

int parse_int(void *user_data, char *token, size_t ntoken) {
    person_parser_helper *helper = (person_parser_helper *) user_data;

    int value = atoi(token);
    if (value <= 0) {
        fputs("Integer parsing failure!\n", stderr);
        return 1;
    }

    if (ntoken == 1) {
        helper->person_array[helper->n].age = value;
    } else if (ntoken == 2) {
        helper->person_array[helper->n].height = value;
        helper->n++;
    }
    return 0;
}

int main(void) {
    FILE *file = fopen("testfiles/testfile.txt", "r");

    fixed_n_delimiter_parser_iter_callback_t grammar_callbacks[3] = {parse_name,
                                                                     parse_int,
                                                                     parse_int};
    fixed_n_delimiter_parser_grammar_t      *grammar =
        fixed_n_delimiter_parser_grammar_new(';', 3, grammar_callbacks);

    person_parser_helper *helper = malloc(sizeof(person_parser_helper));
    helper->person_array         = malloc(sizeof(person_t) * 2);
    helper->n                    = 0;

    parse_dataset(file, '\n', grammar, helper);

    printf("PERSON1: %s, %d, %d\n",
           helper->person_array[0].name,
           helper->person_array[0].age,
           helper->person_array[0].height);
    printf("PERSON1: %s, %d, %d\n",
           helper->person_array[1].name,
           helper->person_array[1].age,
           helper->person_array[1].height);

    free(helper);
    fclose(file);
    fixed_n_delimiter_parser_grammar_free(grammar);
    return 0;
}
