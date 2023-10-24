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
 * @file test.c
 * @brief Contains the entry point to the program.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/dataset_parser.h"
#include "utils/fixed_n_delimiter_parser.h"

/** @brief Path to the file to be opened for this test. */
#define TEST_FILE "testfile.txt"

/** @brief Maximum number of people the test dataset can contain */
#define TEST_MAX_PEOPLE 32

typedef struct {
    char *name;
    int   age, height;
} person_t;

typedef struct {
    size_t   n;
    person_t current_person;
    person_t person_array[TEST_MAX_PEOPLE];
} person_dataset_t;

int parse_name(void *user_data, char *token, size_t ntoken) {
    (void) ntoken;
    person_dataset_t *dataset = (person_dataset_t *) user_data;

    /* Copy string to another buffer */
    char *name_copy              = strdup(token);
    dataset->current_person.name = name_copy;
    return 0;
}

int parse_int(void *user_data, char *token, size_t ntoken) {
    person_dataset_t *dataset = (person_dataset_t *) user_data;

    int value = atoi(token);
    if (value <= 0) {
        fputs("Integer parsing failure!\n", stderr);
        return 1;
    }

    if (ntoken == 1) {
        dataset->current_person.age = value;
    } else if (ntoken == 2) {
        dataset->current_person.height = value;
    }
    return 0;
}

int before_parse_token(void *user_data, char *token) {
    (void) user_data;
    printf("Parsing line: %s\n", token);
    return 0;
}

int add_to_reject_from_database(void *user_data, int retcode) {
    person_dataset_t *dataset = (person_dataset_t *) user_data;

    if (retcode) {
        free(dataset->current_person.name);
        fprintf(stderr, "Failed to parse person of name \"%s\"\n", dataset->current_person.name);
        return 0; /* Recover and continue parsing */
    } else {
        if (dataset->n >= TEST_MAX_PEOPLE) {
            free(dataset->current_person.name);
            fputs("Filled person array!\n", stderr);
            return 1; /* Can't continue parsing */
        } else {
            dataset->person_array[dataset->n] = dataset->current_person;
            dataset->n                        = dataset->n + 1;
            return 0;
        }
    }
}

int main(void) {
    int   retcode = 0;
    FILE *file    = fopen(TEST_FILE, "r");
    if (!file) {
        fputs("Failed to open file \"" TEST_FILE "\"!\n", stderr);
        return 1;
    }

    fixed_n_delimiter_parser_iter_callback_t token_grammar_callbacks[3] = {parse_name,
                                                                           parse_int,
                                                                           parse_int};
    fixed_n_delimiter_parser_grammar_t      *token_grammar =
        fixed_n_delimiter_parser_grammar_new(';', 3, token_grammar_callbacks);
    dataset_parser_grammar_t *grammar = dataset_parser_grammar_new('\n',
                                                                   token_grammar,
                                                                   before_parse_token,
                                                                   add_to_reject_from_database);

    person_dataset_t dataset      = {0};
    int              parse_result = dataset_parser_parse(file, grammar, &dataset);
    if (parse_result) {
        fputs("Parsing failure!\n", stderr);
        retcode = 1;
        goto CLEANUP;
    }

    for (size_t i = 0; i < dataset.n; ++i) {
        person_t *person = dataset.person_array + i;
        printf("Person %" PRIuPTR ": %s is %d years old and %dcm tall\n",
               i,
               person->name,
               person->age,
               person->height);
    }

CLEANUP:
    fclose(file);
    dataset_parser_grammar_free(grammar);

    for (size_t i = 0; i < dataset.n; ++i)
        free(dataset.person_array[i].name);
    return retcode;
}
