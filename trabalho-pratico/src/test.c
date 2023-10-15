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

#include "utils/fixed_n_delimiter_parser.h"

/**
 * Example data to be tokenized and parsed (name, age, height).
 */
#define PERSON_DATA "José Silva,60,176"

typedef struct {
    char *name;
    int   age, height;
} person_t;

/**
 * @brief A @ref tokenize_iter_callback_t to sum heights and count students.
 * @param user_data An array with two integers, the accumulated height and the number of students,
 *                  respectively.
 * @param token Token read by @ref string_const_tokenize
 */
int parse_name(void *user_data, char *token, size_t ntoken) {
    (void) ntoken;

    size_t len       = strlen(token);
    char  *name_copy = malloc(len + 1);
    memcpy(name_copy, token, len + 1);

    ((person_t *) user_data)->name = name_copy;
    return 0;
}

int parse_int(void *user_data, char *token, size_t ntoken) {
    int value = atoi(token);

    if (value <= 0) {
        fputs("Integer parsing failure!\n", stderr);
        return 1;
    }

    if (ntoken == 1) {
        ((person_t *) user_data)->age = value;
    } else if (ntoken == 2) {
        ((person_t *) user_data)->height = value;
    }
    return 0;
}

/**
 * @brief The entry point to the test program.
 * @details Parses information about a person.
 * @retval 0 Success
 * @retval 1 Insuccess
 */
int main(void) {
    fixed_n_delimiter_parser_iter_callback_t grammar_callbacks[3] = {parse_name,
                                                                     parse_int,
                                                                     parse_int};

    fixed_n_delimiter_parser_grammar_t *grammar =
        fixed_n_delimiter_parser_grammar_new(',', 3, grammar_callbacks);

    person_t person = {0};
    int status = fixed_n_delimiter_parser_parse_string_const(PERSON_DATA, grammar, &person);
    if (status) {
        fprintf(stderr, "Parsing failure! (%d)\n", status);
    } else {
        printf("%s is %d and %dcm tall\n", person.name, person.age, person.height);
    }

    if (person.name)
        free(person.name);

    fixed_n_delimiter_parser_grammar_free(grammar);
    return status;
}
